/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_cfar.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   Header file containing functions prototypes of CFAR HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_CFAR_H
#define __ADT6101P_CFAR_H
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"

/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup CFAR
 *  @{
 */

/* Private functions --------------------------------------------------------*/
/** @addtogroup CFAR_Private_Functions
  * @{
  */
void setCfarStr(uint32 value);

void setCfarDatNum(uint32 value);

void setCfarDatAccSumFrz(bitStateTypedef set);

uint32 getCfarDatNumber(void);

void setCfarMask0(uint32 value);

uint32 getCfarMask0(void);

void setCfarMask1(uint32 value);

uint32 getCfarMask1(void);

uint32 getCfarDoneAcc(void);
uint32 getCfarDoneOrd(void);

uint32 getCfarAccSumH(void);

uint32 getCfarAccSumL(void);

void setCfarOrdIdx(uint32 value);

uint32 getCfarOrdIdx(void);

uint32 getCfarOrdValue(void);

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
