/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    rtc_bsp.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file references the RTC HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "rtc_bsp.h"


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   RTC init.
 * @param   enable: RTC enable or disable, see also @ref stateTypedef.
 * @param   clkSource: Select 32k_high source, see also @ref clk32kSrcTypedef.
 */
void rtcInit(stateTypedef enable, clk32kSrcTypedef clkSource)
{
    /* RTC enable */
    setRtcStart(enable);
    /* control the masking of the interrupt that the RTC generates */
    setRtcMask(SET);

    set32kClkSrc(clkSource);
}

/**
 * @brief   start rtc timer.
 * @param   cycle_number: RTC_CLOCK * reloadValue.
 * @return  None.
 */
void rtcTimerStart(uint32 cycleNumber)
{
    /* Option 1: Write RTCMR twice before going to SLEEP.
        Because there may be a conflict between writing RTCMR and edge of clk_32k. */
    //  int32 regVel;
    //  regVel = RTC->RTCDR;
    //  RTC->RTCMR = regVel + cycle_number;
    //  delayUs(1) ;
    //  RTC->RTCMR = regVel + cycle_number;

    /* Option 2: Write RTCMR and then Write RTCLR.
    Writing RTCLR has highest priority to writing RTCMR and edge of clk_32k,
    And it will active all register. */
    setRtcMatch(cycleNumber * RTC_CLOCK);//match value
    /* After changing RTCMR, there MUST is a RTCLR command, to active MR register. */
    setRtcLoad(0);
}
