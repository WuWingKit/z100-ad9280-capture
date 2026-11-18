/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    rtc_bsp.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file provides a RTC universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __RTC_BSP_H
#define __RTC_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_rtc.h"
#include "adt6101P_pmu.h"


/** @addtogroup BSP
 *  @{
 */

/** @addtogroup RTC_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
#define RTC_CLOCK       32


/* Exported functions --------------------------------------------------------*/
/** @addtogroup RTC_Exported_Functions
  * @{
  */
/**
 * @brief   RTC init.
 * @param   rtcInitStrTypedef.
 * @note
 */
void rtcInit(stateTypedef enable, clk32kSrcTypedef clkSource);

/**
 * @brief   start rtc timer.
 * @param   cycle_number: RTC_CLOCK * reloadValue.
 * @return  None.
 * @note
 */
void rtcTimerStart(uint32 cycleNumber);

/**
  * @}
  */
/**
  * @}
  */
/**
  * @}
  */

#endif
