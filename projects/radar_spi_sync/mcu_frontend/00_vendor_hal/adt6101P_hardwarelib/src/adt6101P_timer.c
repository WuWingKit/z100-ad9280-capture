/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_timer.c
 * @original_author  TaoCaoJia(cjtao@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file contains all the functions implemented by the TIMER
 *          peripheral register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_timer.h"
#include "timer_ctype_map.h"


/*------------------------------ variable ------------------------------------*/
static TIMER_HANDLER s_timerHandler[2] = {NULL, NULL};
static DUALTIMER_HANDLER s_dualtimerHandler[2] = {NULL, NULL};

/* Private functions ---------------------------------------------------------*/
/**
 * @brief   Enable timerX Default mode
 * @param   Timerx: Timer0 / Timer1.

 */
static void setTimerDefaultMode(TimerTypeDef *Timerx)
{
    SET_BIT(Timerx->CTRL, Timer_enable);
    CLEAR_BIT(Timerx->CTRL, Sel_exinput_enable);
    CLEAR_BIT(Timerx->CTRL, Sel_exinput_clock);
}

/**
 * @brief   Enable TimerX Extern input mode
 * @param   Timerx: Timer0 / Timer1 .
 * @return  None.
 */
static void setTimerExInMode(TimerTypeDef *Timerx)
{
    SET_BIT(Timerx->CTRL, Timer_enable);
    SET_BIT(Timerx->CTRL, Sel_exinput_enable);
    CLEAR_BIT(Timerx->CTRL, Sel_exinput_clock);
}

/**
 * @brief   Enable TimerX Extern input clock mode
 * @param   Timerx: Timer0 / Timer1.
 * @return  None.
 */
static void setTimerExinClkEn(TimerTypeDef *Timerx)
{
    SET_BIT(Timerx->CTRL, Timer_enable);
    CLEAR_BIT(Timerx->CTRL, Sel_exinput_enable);
    SET_BIT(Timerx->CTRL, Sel_exinput_clock);
}

/**
 * @brief   Enable TimerX Interrupt
 * @param   Timerx: Timer0 / Timer1.
 * @param   enable: ENABLE / DISABLE, see also @ref stateTypedef.
 * @return  None.
 */
void setTimerIntEn(TimerTypeDef *Timerx, const stateTypedef enable)
{
    if(ENABLE == enable)
    {
        SET_BIT(Timerx->CTRL, Timer_int_enable);
    }
    else if(DISABLE == enable)
    {
        CLEAR_BIT(Timerx->CTRL, Timer_int_enable);
    }
}

/**
 * @brief   Clear TimerX ctrl
 * @param   Timerx: Timer0 / Timer1.
 * @return  None.
 */
void clearTimerCtrl(TimerTypeDef *Timerx)
{
    CLEAR_VALUE(Timerx->CTRL);
}

/**
 * @brief   get Timer Ctrl register value
 * @param   Timerx: Timer0 / Timer1.
 * @param   ctrl: see also @ref timerCtrlTypeDef.
 *                  ENABLE / INPUT_ENABLE /
 *                  INPUT_CLOCK/ ENABLE_INTERRUPT.
 * @return  result: Ctrl register value.
 */
uint8 getTimerCtrl(TimerTypeDef *Timerx, const timerCtrlTypeDef ctrl)
{
    uint8 result;

    switch(ctrl)
    {
        case TIMER_ENABLE:
        {
            result = GET_BIT_VALUE(Timerx->CTRL, TIMER_ENABLE);
            break;
        }
        case INPUT_ENABLE:
        {
            result = GET_BIT_VALUE(Timerx->CTRL, INPUT_ENABLE);
            break;
        }
        case INPUT_CLOCK:
        {
            result = GET_BIT_VALUE(Timerx->CTRL, INPUT_CLOCK);
            break;
        }
        case ENABLE_INTERRUPT:
        {
            result = GET_BIT_VALUE(Timerx->CTRL, ENABLE_INTERRUPT);
            break;
        }
    }

    return result;
}

/**
 * @brief   set Timer Ctrl Mode
 * @param   Timerx: Timer0 / Timer1.
 * @param   ctrlMode: see also @ref timerCtrlModeTypeDef.
 * @return  None.
 */
void setTimerCtrlMode(TimerTypeDef *Timerx, const timerCtrlModeTypeDef ctrlMode)
{
    switch(ctrlMode)
    {
        case DEFUALT_MODE:
        {
            setTimerDefaultMode(Timerx);
            break;
        }
        case INPUT_ENABLE_MODE:
        {
            setTimerExInMode(Timerx);
            break;
        }
        case INPUT_CLOCK_MODE:
        {
            setTimerExinClkEn(Timerx);
            break;
        }
    }
}

/**
 * @brief   get Timer Value register value
 * @param   Timerx: Timer0 / Timer1.
 * @return  value: timer value.
 */
uint32 getTimerValue(TimerTypeDef *Timerx)
{
    return Timerx->VALUE;
}

/**
 * @brief   set Timer Value
 * @param   Timerx: Timer0 / Timer1.
 * @param   value: timer value.
 * @return  None.
 */
void setTimerValue(TimerTypeDef *Timerx, const uint32 value)
{
    Timerx->VALUE = value;
}

/**
 * @brief   Set TimerX Reload
 * @param   Timerx: Timer0 / Timer1.
 * @param   reload: frequency of crystal oscillator
 * @return  None.
 */
void setTimerReload(TimerTypeDef *Timerx, const uint32 reload)
{
    Timerx->RELOAD = reload;
}

/**
 * @brief   Get TimerX Reload
 * @param   Timerx: Timer0 / Timer1.
 * @return  reload : timer load.
 */
uint32 getTimerReload(TimerTypeDef *Timerx)
{
    return Timerx->RELOAD;
}

/**
 * @brief   Set TimerX Interrupt flag
 * @param   Timerx: Timer0 / Timer1.
 * @param   enable: ENABLE / DISABLE, see also @ref stateTypedef.
 * @return  None.
 */
void setTimerIntFlag(TimerTypeDef *Timerx, const stateTypedef enable)
{
    Timerx->INTSTATUS = enable;
}

/**
 * @brief   Get TimerX Interrupt flag
 * @param   Timerx: Timer0 / Timer1.
 * @param   enable: ENABLE / DISABLE.
 * @return  flag : timer interrupt flag
 */
uint8 getTimerIntFlag(TimerTypeDef *Timerx)
{
    return Timerx->INTSTATUS;
}

/**
 * @brief   Set DualtimerX reload
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @param   reload: frequency of crystal oscillator
 * @return  None.
 */
void setDualtimerReload(const dualTimerTypedef Timerx, const uint32 reload)
{
    if(DUALTIMER1 == Timerx)
    {
        Dualtimer->TIMER1LOAD = reload;
    }
    else if(DUALTIMER2 == Timerx)
    {
        Dualtimer->TIMER2LOAD = reload;
    }
}

/**
 * @brief   Get DualtimerX reload
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @return  reload : reload.
 */
uint32 getDualtimerReload(const dualTimerTypedef Timerx)
{
    uint32 reload;

    if(DUALTIMER1 == Timerx)
    {
        reload = Dualtimer->TIMER1LOAD;
    }
    else if(DUALTIMER2 == Timerx)
    {
        reload = Dualtimer->TIMER2LOAD;
    }

    return reload;
}

/**
 * @brief   get DualtimerX Value
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @return  value : DualtimerX Value.
 */
uint32 getDualtimerValue(const dualTimerTypedef Timerx)
{
    uint32 value;

    if(DUALTIMER1 == Timerx)
    {
        value = Dualtimer->TIMER1VALUE;
    }
    else if(DUALTIMER2 == Timerx)
    {
        value = Dualtimer->TIMER2VALUE;
    }

    return value;
}

/**
 * @brief   Set DualtimerX Counter Mode
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @param   mode: WRAPPING / ONE_SHOT, see also @ref dualTimerCntModeTypedef.
 * @return  None.
 */
void setDualtimerCounterMode(const dualTimerTypedef Timerx, const dualTimerCntModeTypedef mode)
{
    if(DUALTIMER1 == Timerx)
    {
        if(WRAPPING == mode)
        {
            CLEAR_BIT(Dualtimer->TIMER1CONTROL, Oneshot);
        }
        else if(ONE_SHOT == mode)
        {
            SET_BIT(Dualtimer->TIMER1CONTROL, Oneshot);
        }
    }
    else if(DUALTIMER2 == Timerx)
    {
        if(WRAPPING == mode)
        {
            CLEAR_BIT(Dualtimer->TIMER2CONTROL, Oneshot);
        }
        else if(ONE_SHOT == mode)
        {
            SET_BIT(Dualtimer->TIMER2CONTROL, Oneshot);
        }
    }
}

/**
 * @brief   Set DualtimerX Size
 * @param   timerx: DUALTIMER1 / DUALTIMER2, see also @ref dualTimerTypedef.
 * @param   size: COUNTER16BIT / COUNTER32BIT, see also @ref dualTimerSizeTypedef.
 * @return  None.
 */
void setDualtimerSize(const dualTimerTypedef Timerx, const dualTimerSizeTypedef size)
{
    if(DUALTIMER1 == Timerx)
    {
        if(COUNTER16BIT == size)
        {
            CLEAR_BIT(Dualtimer->TIMER1CONTROL, Timer_size_32bit);
        }
        else if(COUNTER32BIT == size)
        {

            SET_BIT(Dualtimer->TIMER1CONTROL, Timer_size_32bit);
        }
    }
    else if(DUALTIMER2 == Timerx)
    {
        if(COUNTER16BIT == size)
        {
            CLEAR_BIT(Dualtimer->TIMER2CONTROL, Timer_size_32bit);
        }
        else if(COUNTER32BIT == size)
        {
            SET_BIT(Dualtimer->TIMER2CONTROL, Timer_size_32bit);
        }
    }
}

/**
 * @brief   Set DualtimerX Pre
 * @param   timerx: DUALTIMER1 / DUALTIMER2, see also @ref dualTimerTypedef.
 * @param   clkdiv: CLKDIV1 / CLKDIV16 / CLKDIV256 / CLKDIV0, see also @ref clkdivTypedef.
 * @return  None.
 */
void setDualtimerPre(const dualTimerTypedef Timerx, const clkdivTypedef clkdiv)
{
    if(DUALTIMER1 == Timerx)
    {
        SET_VALUE(Dualtimer->TIMER1CONTROL, Timer_prescale_shift, Timer_prescale_mask, clkdiv);
    }
    else if(DUALTIMER2 == Timerx)
    {
        SET_VALUE(Dualtimer->TIMER2CONTROL, Timer_prescale_shift, Timer_prescale_mask, clkdiv);
    }
}

/**
 * @brief   Set DualtimerX Interrupt
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @param   enable: ENABLE / DISABLE.
 * @return  None.
 */
void setDualtimerInt(const dualTimerTypedef Timerx, const stateTypedef enable)
{
    if(DUALTIMER1 == Timerx)
    {
        if(DISABLE == enable)
        {
            CLEAR_BIT(Dualtimer->TIMER1CONTROL, int_Enable);
        }
        else if(ENABLE == enable)
        {
            SET_BIT(Dualtimer->TIMER1CONTROL, int_Enable);
        }
    }
    else if(DUALTIMER2 == Timerx)
    {
        if(DISABLE == enable)
        {
            CLEAR_BIT(Dualtimer->TIMER2CONTROL, int_Enable);
        }
        else if(ENABLE == enable)
        {
            SET_BIT(Dualtimer->TIMER2CONTROL, int_Enable);
        }
    }
}

/**
 * @brief   Set DualtimerX Mode
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @param   mode: FREE_RUNING / PERIODIC, see also @ref dualTimerModeTypedef.
 * @return  None.
 */
void setDualtimerMode(const dualTimerTypedef Timerx, const dualTimerModeTypedef mode)
{
    if(DUALTIMER1 == Timerx)
    {
        if(FREE_RUNING == mode)
        {
            CLEAR_BIT(Dualtimer->TIMER1CONTROL, periodic);
        }
        else if(PERIODIC == mode)
        {
            SET_BIT(Dualtimer->TIMER1CONTROL, periodic);
        }
    }
    else if(DUALTIMER2 == Timerx)
    {
        if(FREE_RUNING == mode)
        {
            CLEAR_BIT(Dualtimer->TIMER2CONTROL, periodic);
        }
        else if(PERIODIC == mode)
        {
            SET_BIT(Dualtimer->TIMER2CONTROL, periodic);
        }
    }
}

/**
 * @brief   Set DualtimerX Enable
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @param   enable: ENABLE / DISABLE, see also @ref enable.
 * @return  None.
 */
void setDualtimerEnable(const dualTimerTypedef Timerx, const stateTypedef enable)
{
    if(DUALTIMER1 == Timerx)
    {
        if(DISABLE == enable)
        {
            CLEAR_BIT(Dualtimer->TIMER1CONTROL, Dultimer_enable);
        }
        else if(ENABLE == enable)
        {
            SET_BIT(Dualtimer->TIMER1CONTROL, Dultimer_enable);
        }
    }
    else if(DUALTIMER2 == Timerx)
    {
        if(DISABLE == enable)
        {
            CLEAR_BIT(Dualtimer->TIMER2CONTROL, Dultimer_enable);
        }
        else if(ENABLE == enable)
        {
            SET_BIT(Dualtimer->TIMER2CONTROL, Dultimer_enable);
        }
    }
}

/**
 * @brief   Get DualtimerX Ctrl
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @param   ctrl: COUNTER_MODE / TIMER_SIZE /
 *                TIMER_PRE / ENABLE_INTERRUPT /
 *                TIMER_MODE / ENABLE
 * @return  result : Dualtimerx CTRL.
 */
uint8 getDualtimerCtrl(const dualTimerTypedef Timerx, const dualTimerCtrlTypeDef ctrl)
{
    uint8 result;

    if(DUALTIMER1 == Timerx)
    {
        switch(ctrl)
        {
            case COUNTER_MODE:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER1CONTROL, COUNTER_MODE);
                break;
            }
            case TIMER_SIZE:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER1CONTROL, TIMER_SIZE);
                break;
            }
            case TIMER_PRE:
            {
                result = GET_BITS_VALUE(Dualtimer->TIMER1CONTROL,
                                        Timer_prescale_shift,
                                        Timer_prescale_mask);
                break;
            }
            case DUALTIMER_ENABLE_INT:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER1CONTROL, DUALTIMER_ENABLE_INT);
                break;
            }
            case TIMER_MODE:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER1CONTROL, TIMER_MODE);
                break;
            }
            case DUALTIMER_ENABLE:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER1CONTROL, DUALTIMER_ENABLE);
                break;
            }
        }
    }
    else if(DUALTIMER2 == Timerx)
    {
        switch(ctrl)
        {
            case COUNTER_MODE:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER2CONTROL, COUNTER_MODE);
                break;
            }
            case TIMER_SIZE:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER2CONTROL, TIMER_SIZE);
                break;
            }
            case TIMER_PRE:
            {
                result = GET_BITS_VALUE(Dualtimer->TIMER2CONTROL,
                                        Timer_prescale_shift,
                                        Timer_prescale_mask);
                break;
            }
            case DUALTIMER_ENABLE_INT:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER2CONTROL, ENABLE_INTERRUPT);
                break;
            }
            case TIMER_MODE:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER2CONTROL, TIMER_MODE);
                break;
            }
            case DUALTIMER_ENABLE:
            {
                result = GET_BIT_VALUE(Dualtimer->TIMER2CONTROL, DUALTIMER_ENABLE);
                break;
            }
        }

    }
    return result;
}

/**
 * @brief   clear Dualtimer Interrupt Register value
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @param   value: Interrupt Register value
 * @return  None.
 */
void clearDualtimerIntReg(const dualTimerTypedef Timerx, const uint32 value)
{
    if(DUALTIMER1 == Timerx)
    {
        Dualtimer->TIMER1INTCLR = value;
    }
    else if(DUALTIMER2 == Timerx)
    {
        Dualtimer->TIMER2INTCLR = value;
    }
}

/**
 * @brief   Get Raw DualtimerX Interrupt Register status
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @return  status : Raw DualtimerX Interrupt Register status.
 */
uint8 getRawDualtimerInt(const dualTimerTypedef Timerx)
{
    uint8 ris;

    if(DUALTIMER1 == Timerx)
    {
        ris = Dualtimer->TIMER1RIS;
    }
    else if(DUALTIMER2 == Timerx)
    {
        ris = Dualtimer->TIMER2RIS;
    }

    return ris;
}

/**
 * @brief   Get DualtimerX Interrupt Register status
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @return  status : DualtimerX Interrupt Register status.
 */
uint8 getDualtimerIntStatus(const dualTimerTypedef Timerx)
{
    uint8 mis;

    if(DUALTIMER1 == Timerx)
    {
        mis = Dualtimer->TMEER1MIS;
    }
    else if(DUALTIMER2 == Timerx)
    {
        mis = Dualtimer->TMEER2MIS;
    }
    return mis;
}

/**
 * @brief   Get DualtimerX Background Load register
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @return  BGLOAD : Background Load register
 */
uint32 getDualtimerBGload(const dualTimerTypedef Timerx)
{
    uint32 bgload;

    if(DUALTIMER1 == Timerx)
    {
        bgload = Dualtimer->TIMER1BGLOAD;
    }
    else if(DUALTIMER2 == Timerx)
    {
        bgload = Dualtimer->TIMER2BGLOAD;
    }

    return bgload;
}

/**
 * @brief   Clear Dual TimerX Ctrl
 * @param   Timerx: Timer0 / Timer1.
 * @return  None.
 */
void clearDualTimerCtrl(const dualTimerTypedef Timerx)
{
    if(DUALTIMER1 == Timerx)
    {
        CLEAR_VALUE(Dualtimer->TIMER1CONTROL);
    }
    else if(DUALTIMER2 == Timerx)
    {
        CLEAR_VALUE(Dualtimer->TIMER2CONTROL);
    }
}

/**
 * @brief   TIMER0 interrupt handler.
 * @return  None.
 */
void timer0Handler(void)
{
    setTimerIntFlag(Timer0, ENABLE);
    s_timerHandler[0]();
}

/**
 * @brief   TIMER1 interrupt handler.
 * @return  None.
 */
void timer1Handler(void)
{
    setTimerIntFlag(Timer1, ENABLE);
    s_timerHandler[1]();
}

/**
 * @brief   Set the TIMER interrupt handler function.
 * @param   timerx: Timer0 / Timer1
 * @param   handler: the address of the TIMER interrupt callback function.
 * @return  None.
 */
void setTimerIntHandler(const TimerTypeDef *Timerx, const TIMER_HANDLER handler)
{
    if(NULL != handler)
    {
        if(Timer0 == Timerx)
        {
            s_timerHandler[0] = handler;
        }
        else if(Timer1 == Timerx)
        {
            s_timerHandler[1] = handler;
        }
    }
}

/**
 * @brief   DUALTIMER interrupt handler.
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @return  None.
 */
void dualtimer0Handler(void)
{
    if(getDualtimerIntStatus(DUALTIMER1) == 1)
    {
        clearDualtimerIntReg(DUALTIMER1, 1);
        s_dualtimerHandler[0]();
    }
    if(getDualtimerIntStatus(DUALTIMER2) == 1)
    {
        clearDualtimerIntReg(DUALTIMER2, 1);
        s_dualtimerHandler[1]();
    }
}


/**
 * @brief   Set the DUALTIMER interrupt handler function.
 * @param   timerx: DUALTIMER1 / DUALTIMER2
 * @param   handler: the address of the TIMER interrupt callback function.
 * @return  None.
 */
void setDualTimerIntHandler(const dualTimerTypedef Timerx, const DUALTIMER_HANDLER handler)
{
    if(NULL != handler)
    {
        if(DUALTIMER1 == Timerx)
        {
            s_dualtimerHandler[0] = handler;
        }
        else if(DUALTIMER2 == Timerx)
        {
            s_dualtimerHandler[1] = handler;
        }
    }
}
