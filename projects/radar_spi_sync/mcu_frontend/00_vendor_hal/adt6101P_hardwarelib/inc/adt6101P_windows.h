/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_windows.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-23
 * @brief   Header file containing functions prototypes of WINDOWS HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_WINDOWS_H
#define __ADT6101P_WINDOWS_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup WINODW
 *  @{
 */


/** @addtogroup WINODW_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
/**
  * @brief  Select ADC window enable or bypass
  */
typedef enum
{
    ADC_WINDOW_ENABLE   = 0,
    ADC_WINDOW_BYPASS   = 1
} adcWnTypedef;

/**
  * @brief  Select FFT window enable or bypass
  */
typedef enum
{
    FFT_WINDOW_ENABLE   = 0,
    FFT_WINDOW_BYPASS   = 1
} fftWnTypedef;

/**
  * @brief  Select FFT window enable or bypass
  */
typedef enum
{
    VELOCITY_FFT_WINDOW_ENABLE   = 0,
    VELOCITY_FFT_WINDOW_BYPASS   = 1
} VelFftWnTypedef;

/**
  * @brief  Select FFT window mode
  */
typedef enum
{
    WINDOW_HANNING  = 0,
    WINDOW_HAMMING  = 1
} windowModeTypedef;
/**
  * @}
  */


/* Private functions ---------------------------------------------------------*/
/** @addtogroup WINODW_Private_Functions
  * @{
  */
uint16 setAdcWnLen(const uint16 len);
uint16 getAdcWnLen(void);

uint8 isAdcWnBypass(const adcWnTypedef en);
uint8 adcWnStatus(void);

uint8 setAdcWnMode(const uint8 mode);
uint8 getAdcWnMode(void);

/**
  * @}
  */
/**
  * @}
  */
/**
  * @}
  */

#endif /* ADT6101P_WINDOWS_H */
