/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    lowpower_bsp.h
 * @original_author  DingZhen(zding@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-29
 * @brief   This file provides a Lowpower universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __LOWPOWER_BSP_H
#define __LOWPOWER_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_system.h"
#include "adt6101P_rx.h"
#include "adt6101P_tx.h"
#include "pmu_bsp.h"
#include "timer_bsp.h"
#include "rtc_bsp.h"
#include "gpio_bsp.h"
#include "sys_bsp.h"
#include "adt6101P.h"

/** @addtogroup BSP
 *  @{
 */

/** @addtogroup LOWPOWER_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/** @addtogroup LOWERPOWER_Exported_Macros
  * @{
  */
/**
 * @brief  Select RAM on or off status
 */
typedef enum
{
    PD_RAM_OFF = 0x0,
    PD_RAM_ON
} ramStateTypedef;

typedef enum
{
    WAKEUP_MODE_RTC = 0x0,    /* Only support method: RTC. */
    WAKEUP_MODE_GPIO,         /* Only support method: GPIO. */
    WAKEUP_MODE_RTC_AND_GPIO  /* Supports two wake-up methods: RTC and GPIO. */
} wakeUpModeTypedef;

typedef enum
{
    LOWPOWER_FAST = 0x0,            /* Close Plln and some RF whitch
                                        Closing time that doesn't take that long  */
    LOWPOWER_SLOW,                  /* Based on LOWPOWER_FAST close more RF */
} lowpowerFrameModeTypedef;

typedef struct
{
    ramStateTypedef       pdRamSet;             /* 0: Select RAM off, 1: Select RAM on. */
    uint16                sleepTime;            /* mcu sleep time, unit: ms. */
    wakeUpModeTypedef     wakeUpMode;
    uint32                wakeupPin;            /* When supporting GPIO wake-up, one or more GPIOs can
                                                    be set as external wake-up sources. */
    gpioWakeUpTypedef     wakeUpLevelSet;       /* 0: Low level to wake up, 1: High level to wake up. */
} wakeUpInitTypedef;


/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup LOWERPOWER_Exported_Functions
  * @{
  */

void lowPowerActiveGotoRadio(uint8 lowpowerEn, lowpowerFrameModeTypedef lowpowerFrameMode,
                             const AnalogDataStrTypedef *pDataStr, const sysHighClkTypedef targetHighClock);
void lowPowerRadioGotoActive(uint8 lowpowerEn, lowpowerFrameModeTypedef lowpowerFrameMode,
                             const AnalogDataStrTypedef *pDataStr, const sysHighClkTypedef targetHighClock);
void lowPowerActiveGotoIdle(void);
void lowPowerActiveGotoSleep(void);

void mcuSleepConfig(ramStateTypedef ramOn);

void pmuGpioWakeup(uint32 pinNumber, uint32 pinPolarity);

void wakeUpInit(wakeUpInitTypedef *wakeuInit);

void nextChirpDisable(void);

void nextChirpEnable(const AnalogDataStrTypedef *pDataStr);


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

