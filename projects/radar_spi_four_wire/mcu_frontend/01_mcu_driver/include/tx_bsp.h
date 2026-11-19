/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    tx_bsp.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file provides a TX universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __TX_BSP_H
#define __TX_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup BSP
 *  @{
 */

/** @addtogroup TX_BSP
 *  @{
 */



/* Exported functions --------------------------------------------------------*/
/** @addtogroup TX_Exported_Functions
  * @{
  */
void pllLock(float fstart, uint32 fm, uint32 chirpNumber);
uint8 pllLockM(float *freqStart, uint32 fm, uint32 chirpNumber);

void fmcwConfig_dualSaw(float fstart, float fmSaw0, float t0Saw0, float t1Saw0, float fmSaw1, float t0Saw1,
                        float t1Saw1, float t2, uint32 chirpNumber);
void fmcwConfig(float fstart, float fm, float t0, float t1, float t2, uint32 chirpNumber);

float vtuneRead(void);

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
