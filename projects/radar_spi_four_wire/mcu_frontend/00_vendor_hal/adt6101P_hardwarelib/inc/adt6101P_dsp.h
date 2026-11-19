/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_dsp.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of DSP HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_DSP_H
#define __ADT6101P_DSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_windows.h"

/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup DSP
 *  @{
 */

/** @addtogroup DSP_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
/**
  * @brief  Select hareware FFT enable or disable
  */
typedef enum
{
    DSP_DISABLE = 0x0,
    DSP_ENABLE
} dspCtrlTypedef;

/**
  * @brief  Select hareware FFT exponent enable or disable
  *
  * disable exponent, that is, use fixed point
  */
typedef enum
{
    EXPONENT_DISABLE = 0x0,
    EXPONENT_ENABLE
} dspExponentTypedef;

/**
  * @brief  Select the hareware FFT to trigger the start signal
  */
typedef enum
{
    SINGLE_STOP = 0x0,
    SINGLE_START,
} dspStartSingleTypedef;
/**
  * @}
  */


/* Private fcuntions ---------------------------------------------------------*/
/** @addtogroup DSP_Private_Functions
  * @{
  */
uint8 setFftEnable(dspCtrlTypedef en);
uint8 getFftStatus(void);

uint8 setFftExponentEnable(dspExponentTypedef en);
uint8 getFftExponentEnable(void);

uint8 setFftWindowtEnable(fftWnTypedef en);
uint8 getFftWindowtEnable(void);

void setFftLen(uint16 len);
uint8 getFftLen(void);

uint8 setFftStartSingle(dspStartSingleTypedef mode);

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
