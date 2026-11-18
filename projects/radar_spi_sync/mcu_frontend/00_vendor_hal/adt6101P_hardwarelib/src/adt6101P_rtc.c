/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_gpio.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file contains all the functions implemented by the GPIO peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_rtc.h"


/* Private functions --------------------------------------------------------*/
/**
 * @brief   get rtc data.
 * @param   None.
 * @return  rtc data.
 */
uint32 getRtcData(void)
{
    return RTC->RTCDR;
}

/**
 * @brief   get rtc match.
 * @param   None.
 * @return  None.
 */
void setRtcMatch(uint32 value)
{
    RTC->RTCMR = value;
}


/**
 * @brief   get rtc match.
 * @param   None.
 * @return  rtc match.
 */
uint32 getRtcMatch(void)
{
    return RTC->RTCMR;
}

/**
 * @brief   get rtc load.
 * @param   None.
 * @return  None.
 */
void setRtcLoad(uint32 value)
{
    RTC->RTCLR = value;
}


/**
 * @brief   get rtc load.
 * @param   None.
 * @return  rtc match.
 */
uint32 getRtcLoad(void)
{
    return RTC->RTCLR;
}


/**
 * @brief   get rtc start.
 * @param   None.
 * @return  None.
 */
void setRtcStart(stateTypedef enable)
{
    if(enable == ENABLE)
    {
        SET_BIT(RTC->RTCCR, RTC_START);
    }
    else
    {
        CLEAR_BIT(RTC->RTCCR, RTC_START);
    }
}

/**
 * @brief   get rtc start.
 * @param   None.
 * @return  rtc start.
 */
uint8 getRtcStart(void)
{
    return RTC->RTCCR;
}


/**
 * @brief   get rtc mask.
 * @param   state: see also @ref bitStateTypedef.
 * @return  None.
 */
void setRtcMask(bitStateTypedef state)
{
    if(state == SET)
    {
        SET_BIT(RTC->RTCIMSC, RTC_MASK);
    }
    else
    {
        CLEAR_BIT(RTC->RTCIMSC, RTC_MASK);
    }
}

/**
 * @brief   get rtc mask.
 * @param   None.
 * @return  rtc mask.
 */
uint8 getRtcMask(void)
{
    return RTC->RTCIMSC;
}


/**
 * @brief   get rtc raw interrupt.
 * @param   None.
 * @return  rtc raw interrupt.
 */
uint8 getRtcRawInterrupt(void)
{
    return RTC->RTCRIS;
}


/**
 * @brief   get rtc mask interrupt.
 * @param   None.
 * @return  rtc mask interrupt.
 */
uint8 getRtcMaskInterrupt(void)
{
    return RTC->RTCMIS;
}


/**
 * @brief   clear rtc interrupt.
 * @param   None.
 * @return  None.
 */
void clearRtcIntrInterrupt(void)
{
    SET_BIT(RTC->RTCICR, RTC_CLEAR_INT);
}
