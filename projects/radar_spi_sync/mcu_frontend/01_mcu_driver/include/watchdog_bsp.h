/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    watchdog_bsp.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file provides a watchdog universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __WATCHDOG_BSP_H
#define __WATCHDOG_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_watchdog.h"
#include "sys_bsp.h"


/** @addtogroup BSP
 *  @{
 */

/** @addtogroup WATCHDOG_BSP
 *  @{
 */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup WATCHDOG_Exported_Functions
  * @{
  */

void watchdogInit(uint32 reload);

void watchdogFeed(uint32 reload);

void watchdogClose(void);

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
