/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    sys_bsp.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file provides a SYSTEM universal function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef SYS_BSP_H
#define SYS_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_system.h"
#include "pmu_bsp.h"

/* Private variables ---------------------------------------------------------*/
extern volatile sysHighClkTypedef g_sysHighClk;
extern volatile int32 g_delayUsFlag;

/** @addtogroup BSP
 *  @{
 */

/** @addtogroup SYS_BSP
 *  @{
 */
/* Exported functions --------------------------------------------------------*/
/** @addtogroup SYSTEM_Exported_Functions
  * @{
  */
void adt6101PDigiInit(void);

void adt6101PRfInit(const AnalogDataStrTypedef *pDataStr, const uint32 chirpNumber);

void adt6101PAnalog(const AnalogDataStrTypedef *pDataStr);

void adt6101PSysClkInit(const sysHighClkTypedef *pClkStr);

void rc32kCalibration(void);

void switch50MHclk(void);

void switch100MHclk(void);

sysHighClkTypedef getSysHighClk(void);
sysHighClkTypedef getPclk(void);


void ledInit(void);

void ledAllon(void);
void ledAlloff(void);

void ledOn(unsigned int Pin_number);
void ledOff(unsigned int Pin_number);

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
