/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#include "app_timebase.h"

#include "adt6101P_inthandler.h"
#include "timer_bsp.h"

static volatile uint32_t s_timer1TickMs;

void AppTimebase_InitTimer1(void)
{
    timerInitStrTypedef timer1;

    timerOff(Timer1);
    s_timer1TickMs = 0u;
    timer1.Timerx = Timer1;
    timer1.mode = DEFUALT_MODE;
    timer1.reload = getReload((float)APP_TIMEBASE_TIMER1_TICK_US);
    timer1.intHandler = timer1InterruptHandler;
    timerInit(&timer1);
}

void AppTimebase_OnTimer1Interrupt(void)
{
    s_timer1TickMs++;
}

uint32_t AppTimebase_NowMs(void *context)
{
    (void)context;
    return s_timer1TickMs;
}
