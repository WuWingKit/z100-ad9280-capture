/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_systick.h
 * @original_author  WangFan(fwang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-1-03
 * @brief   This file describes the CM3-core system_Tick registers.
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_SYSTICK_H
#define __ADT6101P_SYSTICK_H

/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include <CMSDK_CM3.h>

/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup SYSTICK
 *  @{
 */

/** @addtogroup SYSTICK_Private_Macros
  * @{
  */

/* Private types ------------------------------------------------------------*/
typedef enum
{
    EXTERNAL_CLK = 0,
    PROCESSOR_CLK = 1
} clkSourceTypedef;
/**
  * @}
  */

/* Private functions --------------------------------------------------------*/
/** @addtogroup SYSTICK_private_Functions
  * @{
  */

uint8 getSysTickFlag(void);
void setSysTickClkShource(const clkSourceTypedef clk);
uint8 getSysTickClkShource(void);
void setSysTickIntEn(const stateTypedef select);
uint8 getSysTickIntEn(void);
void setSysTickStart(const stateTypedef select);
uint8 getSysTickStart(void);
void setSysTickReload(const uint32 val);
uint32 getSysTickReload(void);
void setSysTickValue(const uint32 val);
uint32 getSysTickValue(void);
uint8 getSysTickNoref(void);
uint8 getSysTickSkew(void);
uint32 getSysTickTenms(void);


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
