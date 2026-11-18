/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    timer_bsp.c
 * @original_author  TaoCaoJia(cjtao@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file references the TIMER HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "timer_bsp.h"
#include "timer_ctype_map.h"
#include "adt6101P_system.h"


/* Exported variables ---------------------------------------------------------*/
volatile int32 g_delayUsFlag;

static void delayUsInterruptHandler(void)
{
    g_delayUsFlag = 1;
}


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   timerX default mode Init
 *
 * @code
 * uint32 reload = getReload(CHIRP_PERIOD);
 * timerInitStrTypedef timer0;
 * timer0.Timerx = Timer0;
 * timer0.mode = DEFUALT_MODE;
 * timer0.reload = reload;
 * timer0.intHandler = timer0InterruptHandler;
 * timerInit(&timer0);
 * @endcode
 * @param   timerInitStrTypedef: Timer initialization configuration structure, see also @ref timerInitStrTypedef.
 * @return  None.
 * @note    timer include TIMER0 TIMER1
 */
void timerInit(timerInitStrTypedef *Timerx)
{
    setTimerCtrlMode(Timerx->Timerx, Timerx->mode);
    setTimerReload(Timerx->Timerx, Timerx->reload);
    setTimerIntEn(Timerx->Timerx, ENABLE);
    setTimerIntHandler(Timerx->Timerx, Timerx->intHandler);
    
    if(Timerx->Timerx == Timer0)
    {
        setIntHandlerEn(INT_TIMER0, ENABLEINT);
    }
    else if(Timerx->Timerx == Timer1)
    {
        setIntHandlerEn(INT_TIMER1, ENABLEINT);
    }
    else if(Timerx->Timerx == Timer2)
    {
        setIntHandlerEn(INT_TIMER2, ENABLEINT);
    }
}

/**
 * @brief   dualtimer Init
 *
 * @code
 * dualtimerInitStrTypedef timer2;
 * timer2.Timerx = DUALTIMER2;
 * timer2.reload = delayus * SYS_HIGH_CLK_50M - 1;
 * timer2.clkdiv = CLKDIV1;
 * timer2.cntmode = ONE_SHOT;
 * timer2.mode = PERIODIC;
 * timer2.intHandler = dualtimer2InterruptHandler;
 * dualtimerInit(&timer2);
 * @endcode
 * @param   dualtimerInitStrTypedef:  Dual-Timer initialization configuration structure, see also @ref dualtimerInitStrTypedef.
 */
void dualtimerInit(dualtimerInitStrTypedef *Timerx)
{
    setDualtimerReload(Timerx->Timerx, Timerx->reload);
    setDualtimerCounterMode(Timerx->Timerx, Timerx->cntmode);
    setDualtimerSize(Timerx->Timerx, COUNTER32BIT);
    setDualtimerPre(Timerx->Timerx, Timerx->clkdiv);
    setDualtimerInt(Timerx->Timerx, ENABLE);
    setDualtimerMode(Timerx->Timerx, Timerx->mode);
    setDualtimerEnable(Timerx->Timerx, ENABLE);
    setDualTimerIntHandler(Timerx->Timerx, Timerx->intHandler);
}

/**
 * @brief   timerX off
 * @param   Timerx: Timer0 / Timer1.
 * @return  None.
 */
void timerOff(TimerTypeDef *Timerx)
{
    clearTimerCtrl(Timerx);
    setTimerReload(Timerx, CLEAR_TIMER_VALUE);
    setTimerValue(Timerx, CLEAR_TIMER_VALUE);
}

/**
 * @brief   dualtimer Off
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 */
void dualtimerOff(dualTimerTypedef Timerx)
{
    setDualtimerReload(Timerx, CLEAR_TIMER_VALUE);
    clearDualTimerCtrl(Timerx);
}

/**
 * @brief   get Reload
 * @param   value: time value
 */
uint32 getReload(float value)
{
    uint32 reload;

    sysHighClkTypedef sysPclk = getPclk();
    
    reload = value * sysPclk - 1;

    return reload;
}

/**
 * @brief   The delay realized by dualtimer, the accuracy is in "US".
 * @param   delayUs: the time you want to delay,in us.
 * @return  None.
 */
void delayUs(uint32 delayus)
{
    g_delayUsFlag = 0;
    
    sysHighClkTypedef sysPclk = getPclk();
    
    dualtimerInitStrTypedef timer2;
    timer2.Timerx = DUALTIMER2;
    timer2.reload = delayus * sysPclk - 1;
    timer2.clkdiv = CLKDIV1;
    timer2.cntmode = ONE_SHOT;
    timer2.mode = PERIODIC;
    timer2.intHandler = delayUsInterruptHandler;
    dualtimerInit(&timer2);
    
    while(g_delayUsFlag != 1);
}
