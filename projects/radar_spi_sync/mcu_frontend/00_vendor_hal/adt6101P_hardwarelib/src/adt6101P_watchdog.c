/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_watchdog.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file contains all the functions implemented by the watchdog HAL peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_watchdog.h"


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   set watch dog load.
 * @param   reload: clock beats, 50000000 = 1s.
 * @return  None.
 */
void setWatchDogLoad(uint32 reload)
{
    WATCHDOG->WDOGLOAD = reload;
}

/**
 * @brief   get watch dog load.
 * @param   None.
 * @return  watch dog load.
 */
uint32 getWatchDogLoad(void)
{
    return WATCHDOG->WDOGLOAD;
}

/**
 * @brief   get watch dog value.
 * @param   None.
 * @return  watch dog value.
 */
uint32 getWatchDogValue(void)
{
    return WATCHDOG->WDOGVALUE;
}

/**
 * @brief   set watch dog control .
 * @param   control: see also @ref watchdogControlTypedef.
 * @return  None.
 */
void setWatchDogControl(watchdogControlTypedef control)
{
    WATCHDOG->WDOGCONTROL = control;
}

/**
 * @brief   get watch dog control .
 * @param   None.
 * @return  watch dog control.
 */
uint8 getWatchDogControl(void)
{
    return WATCHDOG->WDOGCONTROL;
}

/**
 * @brief   clear watch Clear Interrupt.
 * @param   None.
 * @return  None.
 */
void clearWatchDogClearInt(void)
{
    WATCHDOG->WDOGINTCLR = WATCHDOG_INT_CLEAR;
}

/**
 * @brief   get watch dog raw interrupt status.
 * @param   control.
 * @return  watch dog raw interrupt status.
 */
uint8 getWatchDogRawIntStatus(void)
{
    return WATCHDOG->WDOGRIS;
}

/**
 * @brief   get watch dog raw interrupt status.
 * @param   control.
 * @return  watch dog raw interrupt status.
 */
uint8 getWatchDogIntStatus(void)
{
    return WATCHDOG->WDOGMIS;
}

/**
 * @brief   set watch dog lock.
 * @param   isLock: WATCHDOG_UNLOCK: Enable watchdog, unlcok.
 *                  WATCHDOG_LOCK: Disable watchdog, lock.
 * @return  None.
 */
void setWatchdogLock(uint32 isLock)
{
    WATCHDOG->WDOGLOCK = isLock;
}

/**
 * @brief   get watch dog lock.
 * @param   None.
 * @return  isLock: WATCHDOG_UNLOCK: Enable watchdog, unlcok.
 *                  WATCHDOG_LOCK: Disable watchdog, lock.
 */
uint32 getWatchdogLock(void)
{
    return WATCHDOG->WDOGLOCK;
}
