/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_systick.c
 * @original_author  WangFan(fwang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-1-02
 * @brief   This file describes the CM3-core system_Tick registers.
 *
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "adt6101P_systick.h"


/* Functions -----------------------------------------------------------------*/
/**
 * @brief   get SysTick flag.
 * @param   None.
 * @return  SysTick flag.
 * @note    Returns 1 if timer counted to 0 since last time this was read.
 */
uint8 getSysTickFlag(void)
{
    return GET_BIT_VALUE(SysTick->CTRL, SysTick_CTRL_COUNTFLAG_Pos);
}

/**
 * @brief   set SysTick Clock source.
 * @param   clk: EXTERNAL_CLK:external clock; PROCESSOR_CLK: processor clock, see also @ref clkSourceTypedef.
 * @return  None.
 */
void setSysTickClkShource(const clkSourceTypedef clk)
{
    switch(clk)
    {
        case EXTERNAL_CLK:
            SET_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk);
            break;
        case PROCESSOR_CLK:
            CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Msk);
            break;
        default:
            break;
    }
}

/**
 * @brief   get SysTick Clock source.
 * @param   None.
 * @return  0: EXTERNAL_CLK;  1:PROCESSOR_CLK.
 */
uint8 getSysTickClkShource(void)
{
    return GET_BIT_VALUE(SysTick->CTRL, SysTick_CTRL_CLKSOURCE_Pos);
}

/**
 * @brief   set SysTick interrupt.
 * @param   select: ENABLE: enable interrupt; DISABLE: disable interrupt, see also @ref stateTypedef.
 * @return  None.
 */
void setSysTickIntEn(const stateTypedef select)
{
    switch(select)
    {
        case ENABLE:
            SET_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
            break;
        case DISABLE:
            CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_TICKINT_Msk);
            break;
        default:
            break;
    }
}

/**
 * @brief   set SysTick interrupt.
 * @param   None.
 * @return  1: enable interrupt; 0: disable interrupt.
 */
uint8 getSysTickIntEn(void)
{
    return GET_BIT_VALUE(SysTick->CTRL, SysTick_CTRL_TICKINT_Pos);
}

/**
 * @brief   set enable/disable counter.
 * @param   select: ENABLE: counter enabled; DISABLE: counter disabled, see also @ref stateTypedef.
 * @return  None.
 */
void setSysTickStart(const stateTypedef select)
{
    switch(select)
    {
        case ENABLE:
            SET_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
            break;
        case DISABLE:
            CLEAR_BIT(SysTick->CTRL, SysTick_CTRL_ENABLE_Msk);
            break;
        default:
            break;
    }
}

/**
 * @brief   get enable/disable counter.
 * @param   None.
 * @return  1: counter enabled; 0: counter disabled.
 */
uint8 getSysTickStart(void)
{
    return GET_BIT_VALUE(SysTick->CTRL, SysTick_CTRL_ENABLE_Pos);
}

/**
 * @brief   set SysTick Reload value.
 * @param   val: Value to load into the SYST_CTRL register when the counter is enabled and when it reaches 0.
 * @return  None.
 */
void setSysTickReload(const uint32 val)
{
    SysTick->LOAD = val;
}

/**
 * @brief   get SysTick Reload value.
 * @param   None.
 * @return  Value to load into the SYST_CTRL register when the counter is enabled and when it reaches 0.
 */
uint32 getSysTickReload(void)
{
    return SysTick->LOAD;
}

/**
 * @brief   set SysTick value.
 * @param   val: A write of any value clears the field to 0, and also clears SysTickFlag.
 * @return  None.
 */
void setSysTickValue(const uint32 val)
{
    SysTick->VAL = val;
}

/**
 * @brief   get SysTick value.
 * @param   None.
 * @return  SysTick value.
 */
uint32 getSysTickValue(void)
{
    return SysTick->VAL;
}


/**
 * @brief   get SysTick noref: Indicates whether the device provides a reference clock to the processor.
 * @param   None.
 * @return  SysTick noref:0 = reference clock provided; 1 = no reference clock provided.
 * @note    If your device does not provide a reference clock, the SYST_CSR.CLKSOURCE bit reads-as-one
 *          and ignores writes
 */
uint8 getSysTickNoref(void)
{
    return GET_BIT_VALUE(SysTick->CALIB, SysTick_CALIB_NOREF_Pos);
}

/**
 * @brief   get SysTick skew: Indicates whether the TENMS value is exact.
 * @param   None.
 * @return  0 = TENMS value is exact; 1 = TENMS value is inexact, or not given.
 * @note    An inexact TENMS value can affect the suitability of SysTick as a software real time clock.
 */
uint8 getSysTickSkew(void)
{
    return GET_BIT_VALUE(SysTick->CALIB, SysTick_CALIB_SKEW_Pos);
}

/**
 * @brief   get SysTick tenms: Reload value for 10ms (100Hz) timing, subject to system clock skew errors. If the value reads as
 *          zero, the calibration value is not known.
 * @param   None.
 * @return  SysTick tenms.
 */
uint32 getSysTickTenms(void)
{
    return GET_BITS_VALUE(SysTick->CALIB, SysTick_CALIB_TENMS_Pos, ((1 << 24) - 1));
}
