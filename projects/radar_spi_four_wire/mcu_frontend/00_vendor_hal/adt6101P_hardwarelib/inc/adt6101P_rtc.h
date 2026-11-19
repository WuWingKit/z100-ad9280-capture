/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_rtc.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of RTC HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_RTC__H
#define __ADT6101P_RTC__H

/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "rtc_ctype_map.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup RTC
 *  @{
 */


/* Private types ------------------------------------------------------------*/
/** @addtogroup RTC_Private_Macros
  * @{
  */
/* RTCCR */
#define RTC_START       ((uint8)0x1)

/* RTCIMSC */
#define RTC_MASK        ((uint8)0x1)

/* RTCRIS */
#define RTC_RAW_INT     ((uint8)0x1)

/* RTCMIS */
#define RTC_MASK_INT    ((uint8)0x1)

/* RTCICR */
#define RTC_CLEAR_INT   ((uint8)0x1)

/**
  * @}
  */


/* Private functions --------------------------------------------------------*/
/** @addtogroup RTC_Private_Functions
  * @{
  */
uint32 getRtcData(void);

void setRtcMatch(uint32 value);
uint32 getRtcMatch(void);

void setRtcLoad(uint32 value);
uint32 getRtcLoad(void);

void setRtcStart(stateTypedef enable);
uint8 getRtcStart(void);

void setRtcMask(bitStateTypedef state);
uint8 getRtcMask(void);

uint8 getRtcRawInterrupt(void);

uint8 getRtcMaskInterrupt(void);

void clearRtcIntrInterrupt(void);

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
