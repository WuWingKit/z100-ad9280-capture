/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adc_bsp.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file provides a ADC universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADC_BSP_H
#define __ADC_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_adc.h"
#include "adt6101P_windows.h"


/** @addtogroup BSP
 *  @{
 */

/** @addtogroup ADC_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/** @addtogroup ADC_Exported_Macros
  * @{
  */
typedef struct
{
    stateTypedef                    rx0En;          /*!< RX channel0 enable */
    stateTypedef                    rx1En;          /*!< RX channel1 enable */
    dataSampleEndConfigStrTypedef   dsModStr;       /*!< When sampling with ADC, cut the ends and select the
                                                            middle part with better signal quality */
    downsampleBypassTypedef         firBypass;      /*!< FIR eanble or bypass */
    downSampleRateTypedef           downSample;     /*!< Downsampling multiple */

    adcClockTypedef                 adcClk;         /*!< adc clock */
    adcDmaSelTypedef                adcDmaSel;      /*!< Set the mode of dma transporting ADC data */
    adcWnTypedef                    adcWnEn;        /*!< Is ADC windowing bypassed */
    windowModeTypedef               wnMode;         /*!< ADC window mode. */
    uint16                           wnLen;          /*!< Window length. */
    
    fftWnTypedef                    fftWnEn;        /*!< Is ADC windowing bypassed */
} adcStrTypedef;
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup ADC_Exported_Functions
  * @{
  */
void adt6101PAdcSetting(const adcStrTypedef *adcStr);

void adcDataCorrection(uint32 addr, uint16 adcSamples, uint16 chirpNum);

void adcFirDownsample(const downsampleBypassTypedef en, \
                      const downSampleRateTypedef div, \
                      const uint16 coef, \
                      const firDownsampleStrTypedef firStr);


void adt6101PAdcTest(void);

void adcRawOutput(int enable);

uint8 gpadcReadAIO(void);
                      
float ReadChipTem(void);

int8 gpadcCalibration(void);

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
