/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adc_bsp.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file references the ADC HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adc_bsp.h"
#include "adt6101P.h"
#include "dsp_ctype_map.h"
#include "adt6101P_dma.h"
#include "adt6101P_system.h"
#include "adt6101P_pmu.h"
#include "adt6101P_rx.h"
#include "adt6101P_tx.h"
#include <string.h>
#include "timer_bsp.h"
#include "gpio_bsp.h"


/* Exported variables ---------------------------------------------------------*/
static firDownsampleStrTypedef s_firds =
{
    FIRREG0,
    FIRREG1,
    FIRREG2,
    FIRREG3,
    FIRREG4,
    FIRREG5,
    FIRREG6,
    FIRREG7,
    FIRREG8,
    FIRREG9,
    FIRREG10,
    FIRREG11,
    FIRREG12,
    FIRREG13,
    FIRREG14,
    FIRREG15,
    FIRREG16
};


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   adt6101P adc setting.
 * @param   adcStr: ADC setting structure, see also @ref adcStrTypedef.
 * @return  None.
 */
void adt6101PAdcSetting(const adcStrTypedef *adcStr)
{   
    int32 biasRx00 = (uint8)adcStr->rx0En * 8;
    int32 biasRx01 = (uint8)adcStr->rx0En * 7;
    int32 biasRx10 = (uint8)adcStr->rx1En * 8;
    int32 biasRx11 = (uint8)adcStr->rx1En * 7;
    
    int32 bias_dac15 = 0x8;
    int32 bias_dac16 = 0xC;
    
    adcEnable(adcStr->rx0En, adcStr->rx1En);
    adcBiasSetting(biasRx01, biasRx01, biasRx01, biasRx00, biasRx00, bias_dac15,    \
                    biasRx11, biasRx11, biasRx11, biasRx10, biasRx10, bias_dac16);
    adcClkSwitch(adcStr->adcClk);
    setAdcDmaSelect(adcStr->adcDmaSel);
    
    //AUTO_OFFSET->REGISTER_OFFSET:Avoid the effects of the FFT results caused by the restart fir clock
    setAdcSample(adcStr->rx0En, adcStr->rx1En, \
                 REGISTER_OFFSET, CHANNEL1_IQ_SWAP);

    isAdcWnBypass(adcStr->adcWnEn);
    setAdcWnLen(adcStr->wnLen);
    setAdcWnMode(adcStr->wnMode);
    adcFirDownsample(adcStr->firBypass, adcStr->downSample, \
                     s_firds.filter_coefficient, s_firds);

    setDataSampleConfigIntervalAndEnd(adcStr->dsModStr, adcStr->downSample);

    /* Assign interrupt service functions for DMA */
    setDmaIntHandler(DMA0, INT_HANLDER_0);
    setDmaIntHandler(DMA1, INT_HANLDER_0);
    setDmaIntHandler(DMA2, INT_HANLDER_0);
    setDmaIntHandler(DMA3, INT_HANLDER_1);
    setDmaIntHandler(DMA4, INT_HANLDER_1);
}

/**
 * @brief   the function is used to adc fir bypass and dowmsample.
 * @param   en : FIR filter enable or bypass, see also @ref downsampleBypassTypedef.
 * @param   div : Downsampling multiple, see also @ref downSampleRateTypedef.
 * @param   coef : The coefficient in the middle of FIR.
 * @param   firStr : Set the coefficient of FIR structure, see also @ref firDownsampleStrTypedef.
 * @return  None.
 */
void adcFirDownsample(const downsampleBypassTypedef en, \
                      const downSampleRateTypedef div, \
                      const uint16 coef, \
                      const firDownsampleStrTypedef firStr)
{
    setDownSampleRate(div);
    isDownSampleBypass(en);
    setFirMidCoef(coef);
    setFirCoef(firStr);
}


/**
 * @brief   adc bypass testing sw enable.
 * @param   None.
 * @return  None.
 */
void adt6101PAdcTest(void)
{
    rfTiaEnable(DISABLE, DISABLE);
    rfVgaEnable(DISABLE, DISABLE);
    setPgaOutputBuff(DISABLE, DISABLE);
    setAdcTestMode(ENABLE);
}

/**
 * @brief   ADC raw data output,  via P2(clk), P18~P7(bit[11:0]), P19(head).
               : 12bit format, please refer to pin mux excel document for more details. .
 * @param   enable: 1: enable.  0: disable.
 * @return  None.
 */
void adcRawOutput(int enable)
{

    /* comment this setting, so for all HIGH_CLOCK(50 or 125mhz) and ADC_SAMPLE_RATE(8.33 or 16.67) cases, the raw data has the same sequence.
        The sequence is:
        Head: 1       0        0         0        X           ...    X              1       0        0         0        X             ...      X
        data: rx0.i   rx0.q    rx1.i     rx1.q    don't care  ...    don't care     rx0.i   rx0.q    rx1.i     rx1.q    don't care    ...      don't care */
    if(enable)
    {
        /* adc raw data output via P2(clk), P18~P7(bit[11:0]), P19(head). */
        for(int i = 7; i < 20; i++)
        {
            /* set as function 5 (12bit adc output) with maximum drive strength: 12mA. */

            setGpioConfigMode((gpioConfigPinTypedef)i, GPIO_MODE_OUTPUT);
        }

        /* hclkOutputP02 */
        setGpioConfigMode(GPIO_CFG_P2, GPIO_MODE_OUTPUT);
        setClkGateEn(ENABLE, CTRL_CLK_ADC_TEST_12B_EN);
    }
    else
    {
        /* all high-z. */
        for(int i = 7; i < 20; i++)
        {
            setGpioConfigMode((gpioConfigPinTypedef)i, GPIO_MODE_HIGH_INPENDECE);
        }

        /* hclkOutputP02 */
        setGpioConfigMode(GPIO_CFG_P2, GPIO_MODE_HIGH_INPENDECE);
        setClkGateEn(DISABLE, CTRL_CLK_ADC_TEST_12B_EN);
    }
}

/**
 * @brief   read AIO value.
 * @return  uint8 : GPADC value for AIO, reference voltage is 1.35V.
 */
uint8 gpadcReadAIO(void)
{
    uint8 gpadcValue;
    gpadcEnable(GPADC_AIO1);

    /* After testing, we need to read gpadc data many times. */
    delayUs(60);
    getGpadcData();
    delayUs(60);
    getGpadcData();
    delayUs(60);
    gpadcValue = getGpadcData();

    return gpadcValue;
}

/**
 * @brief   Read the internal temperature of the chip.
 * @param   None.
 * @return  float:internal temperature of the chip.
 * @note
 */
float ReadChipTem(void)
{
    volatile float value;
    volatile uint8 gpadcValue;
    float temperature;
    gpioInitStrTypedef  gpio22InitStruct;
    
    //To correct the problem of temperature error in low-power mode
    //Open some power supply for gpadc in 50mhz
    gpio22InitStruct.Mode  = GPIO_MODE_OUTPUT;
    gpio22InitStruct.Pull  = GPIO_PULL_DOWN;
    gpio22InitStruct.Pin   = GPIO_PIN_22;
    gpioInit(&gpio22InitStruct);
    gpioWritePin(GPIO_PIN_22,GPIO_PIN_SET); 
    delayUs(1000);
    dacEnBias1Pd(1, 0); //all bias except for PA&LNA
    baseband18LDO(1, BB_LDO_1P8, 0, BB_LDO_1P8);
    delayUs(50);
    baseband18LDO(1, BB_LDO_1P8, 1, BB_LDO_1P8);
    

    gpadcEnable(GPADC_IN2);
    delayUs(120);
    gpadcValue = RFC->rf_gpadc_data_reg & 0xFF;
    
    value = (float)(((float)(gpadcValue / 256.0f)) * VDD_GPADC);
    temperature = 6.7f * (77.0f - gpadcValue);
    
    dacEnBias1Pd(0, 1); //all bias except for PA&LNA
    baseband18LDO(0, BB_LDO_1P8, 0, BB_LDO_1P8);
    gpioWritePin(GPIO_PIN_22,GPIO_PIN_RESET); 

    return temperature;
}


/**
 * @brief   gpadc calibration.
 * @param   None.
 * @return  None.
 * @note
 */
int8 gpadcCalibration(void)
{
    volatile uint8 gpadcValue;
    volatile int8 gpadcOffset;

    gpadcCalibrationInit();

    setGpadcVTest(ENABLE);

    gpadcValue = getGpadcData();
    delayUs(60);
    gpadcValue = getGpadcData();
    delayUs(60);
    gpadcValue = getGpadcData();

    gpadcOffset = gpadcValue - 128;

    setGpadcVTest(DISABLE);

    return gpadcOffset;
}
