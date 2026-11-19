/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
/**
 * @file  app_state_machine.h
 * @brief 带每状态静态 Context 和可选固定优先级调度的多实例状态机。
 *
 * 库只管理流程，不申请动态内存。机器级共享 Context、每状态静态 Context
 * 都由用户提供；状态需要动态内存时，在自己的 ENTER/EXIT 中调用 MCU 或
 * RTOS 分配接口，并把动态指针保存在该状态的静态 Context 中。
 */

#ifndef APP_STATE_MACHINE_H
#define APP_STATE_MACHINE_H

#include <stdint.h>

#include "app_timestamp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_STATE_INVALID_ID   ((uint8_t)0xffu)
#define APP_STATE_EVENT_NONE   ((uint8_t)0xffu)

typedef uint8_t AppStateIdTypedef;
typedef uint8_t AppStateEventTypedef;

typedef enum
{
    APP_STATE_PHASE_ENTER = 0u,
    APP_STATE_PHASE_RUN,
    APP_STATE_PHASE_EXIT
} AppStatePhaseTypedef;

typedef enum
{
    APP_STATE_OK = 0u,
    APP_STATE_BAD_PARAM,
    APP_STATE_BAD_CONFIG,
    APP_STATE_NOT_STARTED,
    APP_STATE_BLOCKED,
    APP_STATE_NO_READY_MACHINE,
    APP_STATE_EVENT_IGNORED,
    APP_STATE_TRANSITION_DENIED,
    APP_STATE_WAITING_INTERVAL
} AppStateStatusTypedef;

typedef struct AppStateMachine AppStateMachineTypedef;

/**
 * @brief 一个状态的 ENTER/RUN/EXIT 任务函数。
 *
 * stateContext/stateContextBytes 来自当前实例的 Context 绑定表；没有绑定时
 * 分别为 NULL/0。跨状态共享数据通过 AppStateMachine_GetContext() 取得。
 * ENTER/EXIT 返回值被忽略，RUN 返回的事件会立即按转换表分发。
 *
 * 动态内存推荐模式：ENTER 通过 Platform_Alloc() 分配并把指针写进静态
 * stateContext，EXIT 通过 Platform_Free() 释放。若 ENTER 分配失败，将
 * 失败标志写入静态 Context，由下一次 RUN 返回业务失败事件。
 */
typedef AppStateEventTypedef (*AppStateTaskFunc)(
    AppStateMachineTypedef *machine,
    AppStatePhaseTypedef phase,
    void *stateContext,
    uint16_t stateContextBytes);

/**
 * @brief 转换 guard；返回非零允许切换，0 拒绝切换。
 *
 * guard 通过 machine 取得机器级 Context。guard 应快速且无副作用，因为
 * CanDispatch() 查询时也会调用它。
 */
typedef uint8_t (*AppStateGuardFunc)(AppStateMachineTypedef *machine);

/** 可由多个实例共享的状态 ID -> 任务函数定义。 */
typedef struct
{
    AppStateIdTypedef id;
    AppStateTaskFunc task;
} AppStateDefinitionTypedef;

/**
 * @brief 某个状态在某个实例中的静态 Context 地址和长度。
 *
 * 该表属于实例而不是共享流程定义，因此同一套状态任务可以绑定多套 RAM，
 * 实例化多次而互不干扰。无需 Context 的状态可以不出现在绑定表中。
 */
typedef struct
{
    AppStateIdTypedef stateId;
    void *address;
    uint16_t bytes;
} AppStateContextBindingTypedef;

/** 流程图的一条箭头：from --event--> to。 */
typedef struct
{
    AppStateIdTypedef from;
    AppStateEventTypedef event;
    AppStateIdTypedef to;
    AppStateGuardFunc guard;
} AppStateTransitionTypedef;

/** 可由多个实例共享、通常位于 Flash 的完整流程定义。 */
typedef struct
{
    AppStateIdTypedef initialState;
    AppStateIdTypedef fallbackState;
    const AppStateDefinitionTypedef *states;
    uint8_t stateCount;
    const AppStateTransitionTypedef *transitions;
    uint8_t transitionCount;
} AppStateConfigTypedef;

/**
 * @brief 独立状态机运行时。
 *
 * priority 数值越大优先级越高。ready=0 表示该实例主动释放 CPU；可选
 * RunHighestReady() 只会选择 started 且 ready 的最高优先级实例。
 */
struct AppStateMachine
{
    const AppStateConfigTypedef *config;
    void *machineContext;
    const AppStateContextBindingTypedef *stateContexts;
    uint8_t stateContextCount;
    AppStateIdTypedef currentState;
    uint8_t priority;
    uint8_t ready;
    uint8_t started;
    uint8_t initialized;

    /* 可选：多个状态机可以绑定同一个用户时间戳实例。 */
    const AppTimestampTypedef *timestamp;
    AppTimestampTickTypedef runStartTick;
    AppTimestampTickTypedef nextRunTick;
    AppTimestampTickTypedef lastRunTick;
    AppTimestampTickTypedef requestedInterval;
    AppTimestampTickTypedef lastInterval;
    uint16_t timingId;
    AppStateIdTypedef timedState;
    uint8_t intervalRequested;
    uint8_t intervalWaiting;
    uint8_t hasLastTimedRun;
    uint8_t inRun;
};

#define APP_STATE_COUNT_OF(array) \
    ((uint8_t)(sizeof(array) / sizeof((array)[0])))

#define APP_STATE(idValue, taskValue) \
    { (idValue), (taskValue) }

#define APP_STATE_CONTEXT(stateValue, addressValue, bytesValue) \
    { (stateValue), (addressValue), (bytesValue) }

#define APP_STATE_CONTEXT_OBJECT(stateValue, objectValue)          \
    {                                                               \
        (stateValue), &(objectValue), (uint16_t)sizeof(objectValue) \
    }

#define APP_STATE_TRANSITION(fromValue, eventValue, toValue) \
    { (fromValue), (eventValue), (toValue), (AppStateGuardFunc)0 }

#define APP_STATE_GUARDED_TRANSITION( \
    fromValue, eventValue, toValue, guardValue) \
    { (fromValue), (eventValue), (toValue), (guardValue) }

#define APP_STATE_CONFIG(initial, fallback, stateArray, transitionArray) \
    {                                                                  \
        (initial), (fallback),                                         \
        (stateArray), APP_STATE_COUNT_OF(stateArray),                  \
        (transitionArray), APP_STATE_COUNT_OF(transitionArray)         \
    }

/**
 * @brief 绑定共享流程、机器 Context、每状态 Context 和实例优先级。
 *
 * 初始化会验证状态/转换唯一性以及所有 Context 绑定。函数不执行 ENTER；
 * config 和 stateContexts 在实例生命周期内必须保持有效。
 */
AppStateStatusTypedef AppStateMachine_Init(
    AppStateMachineTypedef *machine,
    const AppStateConfigTypedef *config,
    void *machineContext,
    const AppStateContextBindingTypedef *stateContexts,
    uint8_t stateContextCount,
    uint8_t priority);


/**
 * @brief 为状态机绑定共享时间戳并设置日志对象 ID。
 *
 * timestamp 可以被多个状态机共用。绑定后，状态 RUN 内可调用
 * SetNextRunInterval()；未绑定时状态机行为与旧版本完全一致。
 */
AppStateStatusTypedef AppStateMachine_AttachTimestamp(
    AppStateMachineTypedef *machine,
    const AppTimestampTypedef *timestamp,
    uint16_t timingId);

/**
 * @brief 在当前 RUN 内请求“下一次同状态 RUN 入口”的最小对齐间隔。
 *
 * 只有本次 RUN 完成并处理返回事件后仍处于同一状态，请求才生效；若切换
 * 状态，请求自动作废。多次调用时最后一次调用覆盖前值。
 */
AppStateStatusTypedef AppStateMachine_SetNextRunInterval(
    AppStateMachineTypedef *machine,
    AppTimestampTickTypedef intervalTicks);

/** @brief 查询距离当前状态下一次允许 RUN 还剩多少 tick；已到期返回 0。 */
AppTimestampTickTypedef AppStateMachine_GetRemainingInterval(
    const AppStateMachineTypedef *machine);

/**
 * @brief 启动实例、默认置 READY，并执行初始状态 ENTER。
 *
 * ENTER 可以调用 Block()，因此 Start 返回后实例不一定仍然 READY。
 */
AppStateStatusTypedef AppStateMachine_Start(AppStateMachineTypedef *machine);

/**
 * @brief 停止实例并执行当前状态 EXIT，确保状态动态资源得到释放。
 *
 * Stop 后 started/ready 清零；再次 Start 会从当前状态重新执行 ENTER。
 * 重复 Stop 是幂等的。
 */
AppStateStatusTypedef AppStateMachine_Stop(AppStateMachineTypedef *machine);

/**
 * @brief 同步投递事件；匹配时按 EXIT -> 更新状态 -> ENTER 完成切换。
 */
AppStateStatusTypedef AppStateMachine_Dispatch(
    AppStateMachineTypedef *machine,
    AppStateEventTypedef event);

/**
 * @brief 不经过转换表，强制切换到一个已定义状态。
 */
AppStateStatusTypedef AppStateMachine_SetState(
    AppStateMachineTypedef *machine,
    AppStateIdTypedef stateId);

/**
 * @brief 若实例 READY 且触发间隔已到，执行当前状态一次 RUN 并分发事件。
 * @return WAITING_INTERVAL 表示仍在等待同状态下一次触发时刻。
 */
AppStateStatusTypedef AppStateMachine_RunOnce(AppStateMachineTypedef *machine);

/**
 * @brief 从用户提供的实例指针数组中运行最高优先级 READY 状态机一步。
 *
 * 同优先级时数组中靠前者先运行；若它始终 READY，后面的同级实例会饥饿，
 * 应主动 Block 或由用户轮换数组顺序。本函数没有独立调度器对象。
 *
 * @param machines 数组元素必须指向已初始化并 Start 的实例。
 * @param count    数组元素数。
 * @param executed 可选输出；成功时返回本轮实际运行的实例。
 */
AppStateStatusTypedef AppStateMachine_RunHighestReady(
    AppStateMachineTypedef *const machines[],
    uint8_t count,
    AppStateMachineTypedef **executed);

/** @brief 将实例标记为 READY，使其重新参与优先级选择。 */
void AppStateMachine_Wake(AppStateMachineTypedef *machine);

/** @brief 将实例标记为 BLOCKED，主动释放 CPU 给下一优先级。 */
void AppStateMachine_Block(AppStateMachineTypedef *machine);

/** @brief 查询实例当前是否具备实际调度资格（READY 且周期已到）。 */
uint8_t AppStateMachine_IsReady(const AppStateMachineTypedef *machine);

/** @brief 查询某事件当前是否存在且 guard 允许，不改变状态。 */
uint8_t AppStateMachine_CanDispatch(
    AppStateMachineTypedef *machine,
    AppStateEventTypedef event);

/** @brief 读取当前状态；实例无效返回 APP_STATE_INVALID_ID。 */
AppStateIdTypedef AppStateMachine_GetState(
    const AppStateMachineTypedef *machine);

/** @brief 取得该实例跨状态共享的机器级 Context。 */
void *AppStateMachine_GetContext(AppStateMachineTypedef *machine);

/**
 * @brief 查找指定状态在当前实例中绑定的静态 Context。
 * @param bytes 非 NULL 时输出 Context 长度；未绑定时输出 0。
 * @return 绑定地址；未绑定或参数非法返回 NULL。
 */
void *AppStateMachine_GetStateContext(
    AppStateMachineTypedef *machine,
    AppStateIdTypedef stateId,
    uint16_t *bytes);


#ifndef APP_STATE_TIMING_LOG_ENABLE
#define APP_STATE_TIMING_LOG_ENABLE 0
#endif

#if APP_STATE_TIMING_LOG_ENABLE
/**
 * @brief 时序日志钩子，由用户实现。
 *
 * ratioPermille=1000 表示实际相邻触发间隔等于设定间隔，3000 表示 3 倍。
 * 库仅在两次连续同状态受控 RUN 之间调用，不依赖 printf。
 */
void AppStateMachine_TimingLog(
    uint16_t timingId,
    AppStateIdTypedef stateId,
    AppTimestampTickTypedef setInterval,
    AppTimestampTickTypedef actualInterval,
    uint32_t ratioPermille);
#endif


#ifdef __cplusplus
}
#endif

#endif /* APP_STATE_MACHINE_H */
