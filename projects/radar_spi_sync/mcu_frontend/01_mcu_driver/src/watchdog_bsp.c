/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    watchdog_bsp.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file references the watchdog HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "watchdog_bsp.h"
#include "adt6101P_system.h"


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   watchdog init.
 * @param   reload: s.
 * @return  None.
 */
void watchdogInit(uint32 reload)
{
    setWatchdogLock(WATCHDOG_UNLOCK);

    sysHighClkTypedef sysHighClk = getSysHighClk();
    if(sysHighClk == SYS_HIGH_CLK_50M)
    {
        reload = SYS_HIGH_CLK_50M * 1000000 * reload - 1;
    }
    else
    {
        reload = SYS_HIGH_CLK_100M * 1000000 * reload - 1;
    }

    clearWatchDogClearInt();
    setWatchDogLoad(reload);
    setWatchDogControl(WATCHDOG_RESEN_AND_INTEN);

    setWatchdogLock(WATCHDOG_LOCK);
}


/**
 * @brief   watchdog feed.
 * @param   reload: s.
 * @return  None.
 */
void watchdogFeed(uint32 reload)
{
    setWatchdogLock(WATCHDOG_UNLOCK);

    sysHighClkTypedef sysHighClk = getSysHighClk();
    if(sysHighClk == SYS_HIGH_CLK_50M)
    {
        reload = SYS_HIGH_CLK_50M * 1000000 * reload - 1;
    }
    else
    {
        reload = SYS_HIGH_CLK_100M * 1000000 * reload - 1;
    }

    setWatchDogLoad(reload);

    setWatchdogLock(WATCHDOG_LOCK);
}

/**
 * @brief   watchdog close.
 * @param   None.
 * @return  None.
 */
void watchdogClose(void)
{
    setWatchdogLock(WATCHDOG_UNLOCK);

    setWatchDogControl(WATCHDOG_NONE);

    setWatchdogLock(WATCHDOG_LOCK);
}
