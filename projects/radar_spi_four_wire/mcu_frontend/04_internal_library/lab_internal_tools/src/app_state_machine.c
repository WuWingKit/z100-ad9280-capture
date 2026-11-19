/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
/**
 * @file  app_state_machine.c
 * @brief 状态转换、Context 绑定和固定优先级协作调度实现。
 */

#include "app_state_machine.h"


/* 清除当前状态的间隔请求/等待；保留已绑定的共享时间戳和 timingId。 */
static void AppStateMachine_ClearTimingState(AppStateMachineTypedef *machine)
{
    machine->runStartTick = 0u;
    machine->nextRunTick = 0u;
    machine->lastRunTick = 0u;
    machine->requestedInterval = 0u;
    machine->lastInterval = 0u;
    machine->timedState = APP_STATE_INVALID_ID;
    machine->intervalRequested = 0u;
    machine->intervalWaiting = 0u;
    machine->hasLastTimedRun = 0u;
    machine->inRun = 0u;
}

/* 判断 READY 状态机是否已经到达其同状态下一次 RUN 入口。 */
static uint8_t AppStateMachine_IsIntervalDue(
    const AppStateMachineTypedef *machine)
{
    AppTimestampTickTypedef now;

    if((machine->intervalWaiting == 0u) ||
       (machine->timestamp == (const AppTimestampTypedef *)0))
    {
        return 1u;
    }
    now = AppTimestamp_Now(machine->timestamp);
    return AppTimestamp_HasReached(now, machine->nextRunTick);
}

#if APP_STATE_TIMING_LOG_ENABLE
static uint32_t AppStateMachine_RatioPermille(
    AppTimestampTickTypedef actual,
    AppTimestampTickTypedef configured)
{
    uint64_t scaled;

    if(configured == 0u)
    {
        return 0u;
    }
    scaled = ((uint64_t)actual * 1000u) / configured;
    return (scaled > UINT32_MAX) ? UINT32_MAX : (uint32_t)scaled;
}
#endif

/* 按 ID 在线性只读表中找任务定义；小表省 RAM，也允许状态 ID 不连续。 */
static const AppStateDefinitionTypedef *AppStateMachine_FindState(
    const AppStateMachineTypedef *machine,
    AppStateIdTypedef stateId)
{
    uint8_t i;

    if((machine == (const AppStateMachineTypedef *)0) ||
       (machine->config == (const AppStateConfigTypedef *)0))
    {
        return (const AppStateDefinitionTypedef *)0;
    }
    for(i = 0u; i < machine->config->stateCount; ++i)
    {
        if(machine->config->states[i].id == stateId)
        {
            return &machine->config->states[i];
        }
    }
    return (const AppStateDefinitionTypedef *)0;
}

/* 查找实例自己的状态 Context 绑定；没有绑定是合法情况。 */
static const AppStateContextBindingTypedef *AppStateMachine_FindContext(
    const AppStateMachineTypedef *machine,
    AppStateIdTypedef stateId)
{
    uint8_t i;

    if(machine == (const AppStateMachineTypedef *)0)
    {
        return (const AppStateContextBindingTypedef *)0;
    }
    for(i = 0u; i < machine->stateContextCount; ++i)
    {
        if(machine->stateContexts[i].stateId == stateId)
        {
            return &machine->stateContexts[i];
        }
    }
    return (const AppStateContextBindingTypedef *)0;
}

/* 调用指定状态的某个 phase，并自动注入该实例对应的静态 Context。 */
static AppStateEventTypedef AppStateMachine_Call(
    AppStateMachineTypedef *machine,
    const AppStateDefinitionTypedef *state,
    AppStatePhaseTypedef phase)
{
    const AppStateContextBindingTypedef *binding =
        AppStateMachine_FindContext(machine, state->id);
    void *address = (binding == (const AppStateContextBindingTypedef *)0) ?
                    (void *)0 : binding->address;
    uint16_t bytes = (binding == (const AppStateContextBindingTypedef *)0) ?
                     0u : binding->bytes;

    return state->task(machine, phase, address, bytes);
}

/* 完整验证共享流程定义以及本实例 Context 绑定，避免运行期出现歧义。 */
static uint8_t AppStateMachine_IsConfigValid(AppStateMachineTypedef *machine)
{
    const AppStateConfigTypedef *config = machine->config;
    uint8_t i;
    uint8_t j;

    if((config == (const AppStateConfigTypedef *)0) ||
       (config->states == (const AppStateDefinitionTypedef *)0) ||
       (config->stateCount == 0u) ||
       ((config->transitions == (const AppStateTransitionTypedef *)0) &&
        (config->transitionCount > 0u)) ||
       ((machine->stateContexts == (const AppStateContextBindingTypedef *)0) &&
        (machine->stateContextCount > 0u)) ||
       (AppStateMachine_FindState(machine, config->initialState) ==
        (const AppStateDefinitionTypedef *)0) ||
       (AppStateMachine_FindState(machine, config->fallbackState) ==
        (const AppStateDefinitionTypedef *)0))
    {
        return 0u;
    }

    for(i = 0u; i < config->stateCount; ++i)
    {
        if((config->states[i].id == APP_STATE_INVALID_ID) ||
           (config->states[i].task == (AppStateTaskFunc)0))
        {
            return 0u;
        }
        for(j = (uint8_t)(i + 1u); j < config->stateCount; ++j)
        {
            if(config->states[i].id == config->states[j].id)
            {
                return 0u;
            }
        }
    }

    for(i = 0u; i < config->transitionCount; ++i)
    {
        const AppStateTransitionTypedef *transition = &config->transitions[i];
        if((transition->event == APP_STATE_EVENT_NONE) ||
           (AppStateMachine_FindState(machine, transition->from) ==
            (const AppStateDefinitionTypedef *)0) ||
           (AppStateMachine_FindState(machine, transition->to) ==
            (const AppStateDefinitionTypedef *)0))
        {
            return 0u;
        }
        for(j = (uint8_t)(i + 1u); j < config->transitionCount; ++j)
        {
            if((transition->from == config->transitions[j].from) &&
               (transition->event == config->transitions[j].event))
            {
                return 0u;
            }
        }
    }

    for(i = 0u; i < machine->stateContextCount; ++i)
    {
        const AppStateContextBindingTypedef *binding = &machine->stateContexts[i];
        if((AppStateMachine_FindState(machine, binding->stateId) ==
            (const AppStateDefinitionTypedef *)0) ||
           (binding->address == (void *)0) ||
           (binding->bytes == 0u))
        {
            return 0u;
        }
        for(j = (uint8_t)(i + 1u); j < machine->stateContextCount; ++j)
        {
            if(binding->stateId == machine->stateContexts[j].stateId)
            {
                return 0u;
            }
        }
    }
    return 1u;
}

/* 绑定所有实例资源；不调用任何状态任务。 */
AppStateStatusTypedef AppStateMachine_Init(
    AppStateMachineTypedef *machine,
    const AppStateConfigTypedef *config,
    void *machineContext,
    const AppStateContextBindingTypedef *stateContexts,
    uint8_t stateContextCount,
    uint8_t priority)
{
    if((machine == (AppStateMachineTypedef *)0) ||
       (config == (const AppStateConfigTypedef *)0))
    {
        return APP_STATE_BAD_PARAM;
    }

    machine->config = config;
    machine->machineContext = machineContext;
    machine->stateContexts = stateContexts;
    machine->stateContextCount = stateContextCount;
    machine->currentState = config->initialState;
    machine->priority = priority;
    machine->ready = 0u;
    machine->started = 0u;
    machine->initialized = 0u;
    machine->timestamp = (const AppTimestampTypedef *)0;
    machine->timingId = 0u;
    AppStateMachine_ClearTimingState(machine);

    if(AppStateMachine_IsConfigValid(machine) == 0u)
    {
        return APP_STATE_BAD_CONFIG;
    }
    machine->initialized = 1u;
    return APP_STATE_OK;
}

/* 为实例绑定可共享时间戳；状态机不拥有该对象的生命周期。 */
AppStateStatusTypedef AppStateMachine_AttachTimestamp(
    AppStateMachineTypedef *machine,
    const AppTimestampTypedef *timestamp,
    uint16_t timingId)
{
    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u) ||
       (timestamp == (const AppTimestampTypedef *)0) ||
       (timestamp->initialized == 0u) ||
       (timestamp->now == (AppTimestampNowFunc)0))
    {
        return APP_STATE_BAD_PARAM;
    }

    machine->timestamp = timestamp;
    machine->timingId = timingId;
    AppStateMachine_ClearTimingState(machine);
    return APP_STATE_OK;
}

/* 只接受 RUN 内请求；最后一次调用覆盖本轮先前请求。 */
AppStateStatusTypedef AppStateMachine_SetNextRunInterval(
    AppStateMachineTypedef *machine,
    AppTimestampTickTypedef intervalTicks)
{
    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u))
    {
        return APP_STATE_BAD_PARAM;
    }
    if((machine->timestamp == (const AppTimestampTypedef *)0) ||
       (machine->inRun == 0u) ||
       (intervalTicks == 0u) ||
       (intervalTicks > (AppTimestampTickTypedef)INT32_MAX))
    {
        return APP_STATE_BAD_CONFIG;
    }

    machine->requestedInterval = intervalTicks;
    machine->intervalRequested = 1u;
    return APP_STATE_OK;
}

/* 返回回绕安全的剩余等待 tick。 */
AppTimestampTickTypedef AppStateMachine_GetRemainingInterval(
    const AppStateMachineTypedef *machine)
{
    AppTimestampTickTypedef now;

    if((machine == (const AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u) ||
       (machine->intervalWaiting == 0u) ||
       (machine->timestamp == (const AppTimestampTypedef *)0))
    {
        return 0u;
    }
    now = AppTimestamp_Now(machine->timestamp);
    return (AppTimestamp_HasReached(now, machine->nextRunTick) != 0u) ?
           0u : (machine->nextRunTick - now);
}

/* 默认 READY 后执行初始 ENTER；ENTER 可立即 Block 释放 CPU。 */
AppStateStatusTypedef AppStateMachine_Start(AppStateMachineTypedef *machine)
{
    const AppStateDefinitionTypedef *state;

    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u))
    {
        return APP_STATE_BAD_PARAM;
    }
    if(machine->started != 0u)
    {
        return APP_STATE_OK;
    }

    state = AppStateMachine_FindState(machine, machine->currentState);
    if(state == (const AppStateDefinitionTypedef *)0)
    {
        return APP_STATE_BAD_CONFIG;
    }
    machine->started = 1u;
    machine->ready = 1u;
    AppStateMachine_ClearTimingState(machine);
    (void)AppStateMachine_Call(machine, state, APP_STATE_PHASE_ENTER);
    return APP_STATE_OK;
}

/* 执行当前状态 EXIT 后撤销调度资格，供应用关闭时释放状态动态资源。 */
AppStateStatusTypedef AppStateMachine_Stop(AppStateMachineTypedef *machine)
{
    const AppStateDefinitionTypedef *state;

    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u))
    {
        return APP_STATE_BAD_PARAM;
    }
    if(machine->started == 0u)
    {
        return APP_STATE_OK;
    }

    state = AppStateMachine_FindState(machine, machine->currentState);
    if(state == (const AppStateDefinitionTypedef *)0)
    {
        return APP_STATE_BAD_CONFIG;
    }
    (void)AppStateMachine_Call(machine, state, APP_STATE_PHASE_EXIT);
    machine->ready = 0u;
    machine->started = 0u;
    AppStateMachine_ClearTimingState(machine);
    return APP_STATE_OK;
}

/* 统一执行 EXIT -> 更新 currentState -> ENTER；不检查转换表。 */
AppStateStatusTypedef AppStateMachine_SetState(
    AppStateMachineTypedef *machine,
    AppStateIdTypedef stateId)
{
    const AppStateDefinitionTypedef *current;
    const AppStateDefinitionTypedef *next;

    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u))
    {
        return APP_STATE_BAD_PARAM;
    }
    next = AppStateMachine_FindState(machine, stateId);
    if(next == (const AppStateDefinitionTypedef *)0)
    {
        return APP_STATE_BAD_CONFIG;
    }
    if(machine->started == 0u)
    {
        machine->currentState = stateId;
        AppStateMachine_ClearTimingState(machine);
        return APP_STATE_OK;
    }
    if(machine->currentState == stateId)
    {
        return APP_STATE_OK;
    }

    current = AppStateMachine_FindState(machine, machine->currentState);
    if(current != (const AppStateDefinitionTypedef *)0)
    {
        (void)AppStateMachine_Call(machine, current, APP_STATE_PHASE_EXIT);
    }
    machine->currentState = stateId;
    AppStateMachine_ClearTimingState(machine);
    (void)AppStateMachine_Call(machine, next, APP_STATE_PHASE_ENTER);
    return APP_STATE_OK;
}

/* 查找当前状态的事件箭头，guard 允许后委托 SetState 切换。 */
AppStateStatusTypedef AppStateMachine_Dispatch(
    AppStateMachineTypedef *machine,
    AppStateEventTypedef event)
{
    uint8_t i;

    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u))
    {
        return APP_STATE_BAD_PARAM;
    }
    if(machine->started == 0u)
    {
        return APP_STATE_NOT_STARTED;
    }
    if(event == APP_STATE_EVENT_NONE)
    {
        return APP_STATE_OK;
    }

    for(i = 0u; i < machine->config->transitionCount; ++i)
    {
        const AppStateTransitionTypedef *transition =
            &machine->config->transitions[i];
        if((transition->from == machine->currentState) &&
           (transition->event == event))
        {
            if((transition->guard != (AppStateGuardFunc)0) &&
               (transition->guard(machine) == 0u))
            {
                return APP_STATE_TRANSITION_DENIED;
            }
            return AppStateMachine_SetState(machine, transition->to);
        }
    }
    return APP_STATE_EVENT_IGNORED;
}

/* READY 且到达时间的实例执行一次 RUN；同状态请求在返回后生效。 */
AppStateStatusTypedef AppStateMachine_RunOnce(AppStateMachineTypedef *machine)
{
    const AppStateDefinitionTypedef *state;
    AppStateEventTypedef event;
    AppStateStatusTypedef dispatchStatus = APP_STATE_OK;
    AppStateIdTypedef runState;
    AppTimestampTickTypedef now = 0u;
    AppTimestampTickTypedef endTick;
    AppTimestampTickTypedef elapsed;
    AppTimestampTickTypedef interval;
    AppTimestampTickTypedef multiple;
#if APP_STATE_TIMING_LOG_ENABLE
    AppStateIdTypedef logState = APP_STATE_INVALID_ID;
    AppTimestampTickTypedef logSetInterval = 0u;
    AppTimestampTickTypedef logActualInterval = 0u;
    uint32_t logRatioPermille = 0u;
    uint8_t logPending = 0u;
#endif

    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u))
    {
        return APP_STATE_BAD_PARAM;
    }
    if(machine->started == 0u)
    {
        return APP_STATE_NOT_STARTED;
    }
    if(machine->ready == 0u)
    {
        return APP_STATE_BLOCKED;
    }
    if(AppStateMachine_IsIntervalDue(machine) == 0u)
    {
        return APP_STATE_WAITING_INTERVAL;
    }

    state = AppStateMachine_FindState(machine, machine->currentState);
    if(state == (const AppStateDefinitionTypedef *)0)
    {
        (void)AppStateMachine_SetState(machine, machine->config->fallbackState);
        return APP_STATE_BAD_CONFIG;
    }

    if(machine->timestamp != (const AppTimestampTypedef *)0)
    {
        now = AppTimestamp_Now(machine->timestamp);
#if APP_STATE_TIMING_LOG_ENABLE
        if((machine->intervalWaiting != 0u) &&
           (machine->hasLastTimedRun != 0u) &&
           (machine->timedState == machine->currentState) &&
           (machine->lastInterval != 0u))
        {
            logState = machine->currentState;
            logSetInterval = machine->lastInterval;
            logActualInterval = now - machine->lastRunTick;
            logRatioPermille = AppStateMachine_RatioPermille(
                logActualInterval, logSetInterval);
            logPending = 1u;
        }
#endif
        machine->intervalWaiting = 0u;
        machine->runStartTick = now;
    }

    runState = machine->currentState;
    machine->intervalRequested = 0u;
    machine->requestedInterval = 0u;
    machine->inRun = 1u;
    event = AppStateMachine_Call(machine, state, APP_STATE_PHASE_RUN);
    machine->inRun = 0u;

    if(event != APP_STATE_EVENT_NONE)
    {
        dispatchStatus = AppStateMachine_Dispatch(machine, event);
    }

    if((machine->currentState == runState) &&
       (machine->intervalRequested != 0u) &&
       (machine->timestamp != (const AppTimestampTypedef *)0))
    {
        endTick = AppTimestamp_Now(machine->timestamp);
        interval = machine->requestedInterval;
        elapsed = endTick - machine->runStartTick;
        multiple = elapsed / interval;
        if(((elapsed % interval) != 0u) || (multiple == 0u))
        {
            multiple++;
        }

        machine->nextRunTick = machine->runStartTick + multiple * interval;
        machine->lastRunTick = machine->runStartTick;
        machine->lastInterval = interval;
        machine->timedState = runState;
        machine->hasLastTimedRun = 1u;
        machine->intervalWaiting = 1u;
    }
    else
    {
        machine->intervalRequested = 0u;
        machine->requestedInterval = 0u;
        if(machine->currentState == runState)
        {
            machine->intervalWaiting = 0u;
            machine->hasLastTimedRun = 0u;
            machine->timedState = APP_STATE_INVALID_ID;
        }
    }

#if APP_STATE_TIMING_LOG_ENABLE
    if(logPending != 0u)
    {
        AppStateMachine_TimingLog(
            machine->timingId,
            logState,
            logSetInterval,
            logActualInterval,
            logRatioPermille);
    }
#endif

    return dispatchStatus;
}

/* 无额外调度器对象：线性扫描用户数组，数值最大的 READY 优先。 */
AppStateStatusTypedef AppStateMachine_RunHighestReady(
    AppStateMachineTypedef *const machines[],
    uint8_t count,
    AppStateMachineTypedef **executed)
{
    AppStateMachineTypedef *selected = (AppStateMachineTypedef *)0;
    uint8_t i;

    if(executed != (AppStateMachineTypedef **)0)
    {
        *executed = (AppStateMachineTypedef *)0;
    }
    if((machines == (AppStateMachineTypedef *const *)0) || (count == 0u))
    {
        return APP_STATE_BAD_PARAM;
    }

    for(i = 0u; i < count; ++i)
    {
        AppStateMachineTypedef *machine = machines[i];
        if((machine == (AppStateMachineTypedef *)0) ||
           (machine->initialized == 0u) ||
           (machine->started == 0u))
        {
            return APP_STATE_BAD_PARAM;
        }
        if((AppStateMachine_IsReady(machine) != 0u) &&
           ((selected == (AppStateMachineTypedef *)0) ||
            (machine->priority > selected->priority)))
        {
            selected = machine;
        }
    }

    if(selected == (AppStateMachineTypedef *)0)
    {
        return APP_STATE_NO_READY_MACHINE;
    }
    if(executed != (AppStateMachineTypedef **)0)
    {
        *executed = selected;
    }
    return AppStateMachine_RunOnce(selected);
}

/* 外部事件到来时重新占用自己的优先级。 */
void AppStateMachine_Wake(AppStateMachineTypedef *machine)
{
    if((machine != (AppStateMachineTypedef *)0) &&
       (machine->initialized != 0u) &&
       (machine->started != 0u))
    {
        machine->ready = 1u;
    }
}

/* 无工作可做时释放 CPU，使低优先级 READY 实例有机会运行。 */
void AppStateMachine_Block(AppStateMachineTypedef *machine)
{
    if((machine != (AppStateMachineTypedef *)0) &&
       (machine->initialized != 0u))
    {
        machine->ready = 0u;
    }
}

/* 只读查询 ready 标志，不触发任务。 */
uint8_t AppStateMachine_IsReady(const AppStateMachineTypedef *machine)
{
    if((machine == (const AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u) ||
       (machine->started == 0u) ||
       (machine->ready == 0u))
    {
        return 0u;
    }
    return AppStateMachine_IsIntervalDue(machine);
}

/* 查询当前事件箭头及 guard，不改变状态。 */
uint8_t AppStateMachine_CanDispatch(
    AppStateMachineTypedef *machine,
    AppStateEventTypedef event)
{
    uint8_t i;

    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u) ||
       (event == APP_STATE_EVENT_NONE))
    {
        return 0u;
    }
    for(i = 0u; i < machine->config->transitionCount; ++i)
    {
        const AppStateTransitionTypedef *transition =
            &machine->config->transitions[i];
        if((transition->from == machine->currentState) &&
           (transition->event == event))
        {
            return (transition->guard == (AppStateGuardFunc)0) ? 1u :
                   (uint8_t)(transition->guard(machine) != 0u);
        }
    }
    return 0u;
}

/* 返回当前状态 ID；无效实例用保留值 0xff 表示。 */
AppStateIdTypedef AppStateMachine_GetState(
    const AppStateMachineTypedef *machine)
{
    return ((machine == (const AppStateMachineTypedef *)0) ||
            (machine->initialized == 0u)) ?
           APP_STATE_INVALID_ID : machine->currentState;
}

/* 取得跨状态共享 Context。 */
void *AppStateMachine_GetContext(AppStateMachineTypedef *machine)
{
    return ((machine == (AppStateMachineTypedef *)0) ||
            (machine->initialized == 0u)) ?
           (void *)0 : machine->machineContext;
}

/* 对外提供任意状态的静态 Context 地址/长度查询。 */
void *AppStateMachine_GetStateContext(
    AppStateMachineTypedef *machine,
    AppStateIdTypedef stateId,
    uint16_t *bytes)
{
    const AppStateContextBindingTypedef *binding;

    if(bytes != (uint16_t *)0)
    {
        *bytes = 0u;
    }
    if((machine == (AppStateMachineTypedef *)0) ||
       (machine->initialized == 0u))
    {
        return (void *)0;
    }
    binding = AppStateMachine_FindContext(machine, stateId);
    if(binding == (const AppStateContextBindingTypedef *)0)
    {
        return (void *)0;
    }
    if(bytes != (uint16_t *)0)
    {
        *bytes = binding->bytes;
    }
    return binding->address;
}
