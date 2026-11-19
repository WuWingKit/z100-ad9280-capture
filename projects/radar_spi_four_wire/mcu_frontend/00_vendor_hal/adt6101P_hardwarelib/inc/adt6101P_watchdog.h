/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_watchdog.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of watchdog HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_WATCHDOG__H
#define __ADT6101P_WATCHDOG__H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "watchdog_ctype_map.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup WATCHDOG
 *  @{
 */


/* Private types -------------------------------------------------------------*/
/** @addtogroup WATCHDOG_Private_Macros
  * @{
  */
#define WATCHDOG_UNLOCK       (uint32)0x1ACCE551
#define WATCHDOG_LOCK         (uint32)0xFFFFFFFF

#define WATCHDOG_INT_CLEAR    0x01

/**
  * @brief  Watchdog Control enumeration
  */
typedef enum
{
    WATCHDOG_NONE               = 0x0,
    WATCHDOG_RESEN              = 0x1,
    WATCHDOG_INTEN              = 0x2,
    WATCHDOG_RESEN_AND_INTEN    = 0x3
} watchdogControlTypedef;
/**
  * @}
  */


/* Private functions --------------------------------------------------------*/
/** @addtogroup WATCHDOG_Private_Functions
  * @{
  */
void setWatchDogLoad(uint32 reload);
uint32 getWatchDogLoad(void);

uint32 getWatchDogValue(void);

void setWatchDogControl(watchdogControlTypedef control);
uint8 getWatchDogControl(void);

void clearWatchDogClearInt(void);

uint8 getWatchDogRawIntStatus(void);

uint8 getWatchDogIntStatus(void);

void setWatchdogLock(uint32 isLock);
uint32 getWatchdogLock(void);

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
