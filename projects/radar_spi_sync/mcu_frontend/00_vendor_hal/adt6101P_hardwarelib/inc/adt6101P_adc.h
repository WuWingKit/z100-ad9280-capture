/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_adc.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-19
 * @brief   Header file containing functions prototypes of ADC HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_ADC_H
#define __ADT6101P_ADC_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup ADC
 *  @{
 */


/** @addtogroup ADC_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
#define ADC0_FIFO       0x40023000   /*!< ADC0_FIFO : adc0 sample source data fifo address */
#define ADC1_FIFO       0x40023004   /*!< ADC1_FIFO : adc0 sample source data fifo address */


#define ADC0SEL         0    /*!< ADC0SEL : select adc0 channel sample */
#define ADC1SEL         1    /*!< ADC1SEL : select adc1 channel sample */
#define ADC01SEL        2    /*!< ADC01SEL : select adc0 and adc1 channel sample */


/* Private typedef -----------------------------------------------------------*/
typedef enum
{
    ADC_CLOCK_7P14M  = 0,    /*!< ADC_7P14M : config adc sample rate 7.14M */
} adcClockTypedef;

/**
  * @brief  Set to 1 to remove offset automatically,
  *         otherwise use register offsets
  */
typedef enum
{
    REGISTER_OFFSET = 0,
    AUTO_OFFSET     = 1
} adcAutoOffsetTypedef;

typedef enum
{
    CHANNEL01_IQ_NO_SWAP    = 0x0,  /*!< channel0 i/q no swap */
    CHANNEL0_IQ_SWAP        = 0x1,  /*!< channel0 i/q swap */
    CHANNEL1_IQ_SWAP        = 0x2,  /*!< channel1 i/q no swap */
    CHANNEL01_IQ_SWAP       = 0x3   /*!< channel1 i/q swap */
} adcChIQSwapTypedef;

/**
  * @brief  Set to 1 to invert adc data
  */
typedef enum
{
    CHANNEL0_I  = 0x0,
    CHANNEL0_Q  = 0x1,
    CHANNEL1_I  = 0x2,
    CHANNEL1_Q  = 0x3
} adcInvertTypedef;

typedef enum
{
    DOWNSAMPLE_NO       = 0x0,  /*!< no down sample */
    DOWNSAMPLE_1DIV2    = 0x1,  /*!< 1/2 down sample */
    DOWNSAMPLE_1DIV3    = 0x2,  /*!< 1/3 down sample */
    DOWNSAMPLE_1DIV4    = 0x3,  /*!< 1/4 down sample */
    DOWNSAMPLE_1DIV5    = 0x4,  /*!< 1/5 down sample */
    DOWNSAMPLE_1DIV6    = 0x5,  /*!< 1/6 down sample */
    DOWNSAMPLE_1DIV7    = 0x6,  /*!< 1/7 down sample */
    DOWNSAMPLE_1DIV8    = 0x7,  /*!< 1/8 down sample */
    DOWNSAMPLE_1DIV9    = 0x8,  /*!< 1/9 down sample */
    DOWNSAMPLE_1DIV10   = 0x9,  /*!< 1/10 down sample */
    DOWNSAMPLE_1DIV11   = 0xA,  /*!< 1/11 down sample */
    DOWNSAMPLE_1DIV12   = 0xB,  /*!< 1/12 down sample */
    DOWNSAMPLE_1DIV13   = 0xC,  /*!< 1/13 down sample */
    DOWNSAMPLE_1DIV14   = 0xD,  /*!< 1/14 down sample */
    DOWNSAMPLE_1DIV15   = 0xE,  /*!< 1/15 down sample */
    DOWNSAMPLE_1DIV16   = 0xF   /*!< 1/16 down sample */
} downSampleRateTypedef;

typedef enum
{
    FIR_ENABLE   = 0,
    FIR_BYPASS   = 1
} downsampleBypassTypedef;

/**
  * @brief  ADC DMA src selec.
  */
typedef enum
{
    FIFO0_CH0QN_CH0IN_FIFO1_CH1QN_CH1IN   = 0,  /*!< means fifo0[31:0] = {ch0_q[N][15:0], ch0_i[N][15:0]} \
                                                    and fifo1[31:0] = {ch1_q[N][15:0], ch1_i[N][15:0]} ; */
    FIFO0_CH0IN_CH1IN       = 1,    /*!< means fifo0[31:0] = {ch0_i[N][15:0], ch1_i[N][15:0]} and fifo1[31:0] = 0. */
    FIFO0_CH0QN_CH1QN       = 2,    /*!< means fifo0[31:0] = {ch0_q[N][15:0], ch1_q[N][15:0]} and fifo1[31:0] = 0. */
    FIFO0_CH0IN_1_CH0IN     = 3,    /*!< means fifo0[31:0] = {ch0_i[N-1][15:0], ch0_i[N][15:0]} and fifo1[31:0] = 0. */
    FIFO0_CH0QN_1_CH0QN     = 4,    /*!< means fifo0[31:0] = {ch0_q[N-1][15:0], ch0_q[N][15:0]} and fifo1[31:0] = 0. */
    FIFO0_CH1IN_1_CH1IN     = 5,    /*!< means fifo0[31:0] = {ch1_i[N-1][15:0], ch1_i[N][15:0]} and fifo1[31:0] = 0. */
    FIFO0_CH1QN_1_CH1QN     = 6     /*!< means fifo0[31:0] = {ch1_q[N-1][15:0], ch1_q[N][15:0]} and fifo1[31:0] = 0. */
} adcDmaSelTypedef;

/**
  * @brief  sync out for cascade application
  */
typedef enum
{
    SYNC_OUT_0   = 0,   /*!< sync_out is 0 */
    SYNC_OUT_1   = 1    /*!< sync_out is 1 */
} syncOutTypedef;


/**
  * @brief  select sync_out's clock edge
  */
typedef enum
{
    PLLI50M_NEGATIVE_EDGE   = 0,    /*!< using plli_50m's negative edge */
    PLLI50M_POSITIVE_EDGE   = 1     /*!< using plli_50m's positive edge */
} syncOutEdgeTypedef;

/**
  * @brief  start tig source select
  */
typedef enum
{
    ADC_START_NO_TRIGGER    = 0,    /*!< select soc register */
    ADC_START_TRIGGER       = 1     /*!< select pll_fmcw_en */
} adcStartTypedef;

typedef enum
{
    NORMAL_OPERTION_MODE    = 0,
    INPUT_DC_COUPLED_MODE   = 1
} acCoupledModeTypedef;

typedef enum
{
    SW_PLLINT   = 0,
    SW_FMCWIN   = 1
} specModeTypedef;

typedef enum
{
    NORMAL_APP   = 0,
    CASCADE_APP   = 1
} cascadeModeTypedef;

typedef struct
{
    uint16                  intervalNum;    /*!< pll adc interval num, matching chirp cycle period. */
    uint16                  endNum;         /*!< the interval_num = start_num + chirp_sample*2 + end_num,
                                            which means head, valid and tail separately. */
    uint16                  adcSampleNum;   /*!< adc sample number in a chirp cycle. */
    uint16                  scanNum;        /*!< data sample scan number. */
} dataSampleEndConfigStrTypedef;

typedef struct
{
    uint16                  intervalNum;    /*!< pll adc interval num, matching chirp cycle period. */
    uint16                  startNum;       /*!< adc sample start number. */
    uint16                  adcSampleNum;   /*!< adc sample number in a chirp cycle. */
    uint16                  scanNum;        /*!< data sample scan number. */
} dataSampleStartConfigStrTypedef;


/**
  * @brief  config fir filter coefficients
  * @note   please refer to cof32's comments for more information about this FIR
  */
typedef struct
{
    uint32 firReg0;     /*!< [12:0]: The 0th fir filter coefficient; [25:13]: The 1th fir filter coefficient */
    uint32 firReg1;     /*!< [12:0]: The 2th fir filter coefficient; [25:13]: The 3th fir filter coefficient */
    uint32 firReg2;     /*!< [12:0]: The 4th fir filter coefficient; [25:13]: The 5th fir filter coefficient */
    uint32 firReg3;     /*!< [12:0]: The 6th fir filter coefficient; [25:13]: The 7th fir filter coefficient */
    uint32 firReg4;     /*!< [12:0]: The 8th fir filter coefficient; [25:13]: The 9th fir filter coefficient */
    uint32 firReg5;     /*!< [12:0]: The 10th fir filter coefficient; [25:13]: The 11th fir filter coefficient */
    uint32 firReg6;     /*!< [12:0]: The 12th fir filter coefficient; [25:13]: The 13th fir filter coefficient */
    uint32 firReg7;     /*!< [12:0]: The 14th fir filter coefficient; [25:13]: The 15th fir filter coefficient */
    uint32 firReg8;     /*!< [12:0]: The 16th fir filter coefficient; [25:13]: The 17th fir filter coefficient */
    uint32 firReg9;     /*!< [12:0]: The 18th fir filter coefficient; [25:13]: The 19th fir filter coefficient */
    uint32 firReg10;    /*!< [12:0]: The 20th fir filter coefficient; [25:13]: The 21th fir filter coefficient */
    uint32 firReg11;    /*!< [12:0]: The 22th fir filter coefficient; [25:13]: The 23th fir filter coefficient */
    uint32 firReg12;    /*!< [12:0]: The 24th fir filter coefficient; [25:13]: The 25th fir filter coefficient */
    uint32 firReg13;    /*!< [12:0]: The 26th fir filter coefficient; [25:13]: The 27th fir filter coefficient */
    uint32 firReg14;    /*!< [12:0]: The 28th fir filter coefficient; [25:13]: The 29th fir filter coefficient */
    uint32 firReg15;    /*!< [12:0]: The 30th fir filter coefficient; [25:13]: The 31th fir filter coefficient */
    uint32 filter_coefficient;  /*!< The 32th fir filter coefficient. This is the middle coef of FIR.
                                        FIR for down sample purpose.
                                        -3db : 0.235*fs/2
                                        -50db: 0.306*fs/2
                                        ripple in bandwidth: 0.02db.
                                        format is (1,0,12). 2's complement, and 1 sign bit, 0 integer bit and 12 fraction bits. */
} firDownsampleStrTypedef;
/**
  * @}
  */


/* Private functions ---------------------------------------------------------*/
/** @addtogroup ADC_Private_Functions
  * @{
  */
void adcEnable(const stateTypedef adc0Enable, const stateTypedef adc1Enable);
void adcBiasSetting(uint8 adc0Bias0, uint8 adc0Bias1, uint8 adc0Bias2, uint8 adc0Bias3,
                    uint8 adc0Bias4, uint8 adc0Bias5, uint8 adc1Bias0, uint8 adc1Bias1,
                    uint8 adc1Bias2, uint8 adc1Bias3, uint8 adc1Bias4, uint8 adc1Bias5);
void setAdcSample(const stateTypedef ch0En, const stateTypedef ch1En,
                  const adcAutoOffsetTypedef offset, const adcChIQSwapTypedef swap);
void adcDataCorrection(uint32 addr, uint16 adcSamples, uint16 chirpNum);
void setDataSampleConfigIntervalAndEnd(const dataSampleEndConfigStrTypedef dsModStr,   \
                                       const downSampleRateTypedef downSample);
void setDataSampleConfigIntervalAndStart(const dataSampleStartConfigStrTypedef dsModStr,   \
        const downSampleRateTypedef downSample);
void adcFmcwStart(void);
void siggen(uint16 step, uint16 chgStep, uint16 comp, uint16 siggenEn, uint16 ang);
void gpadcEnable(const uint16 enableSigInput);
uint8 getGpadcData(void);

void gpadcCalibrationInit(void);
void setGpadcVTest(const stateTypedef en);

uint8 baseBandBiasSetting(const stateTypedef bbBiasEn);

uint8 setRefBuffForADC1(const uint8 vrefVal);
uint8 getRefBuffForADC1(void);

uint8 setRefBuffForADC0(const uint8 vrefVal);
uint8 getRefBuffForADC0(void);

uint8 setInputCoupledModeForADC1(const acCoupledModeTypedef mode);
uint8 getInputCoupledModeForADC1(void);

uint8 setInputCoupledModeForADC0(const acCoupledModeTypedef mode);
uint8 getInputCoupledModeForADC0(void);

uint8 setSpecModeTo20Buff(const specModeTypedef mode);
uint8 getSpecModeTo20Buff(void);

uint8 adcClkSwitch(const adcClockTypedef adcClk);

void setAdcCh0AndCh1En(const stateTypedef ch0En, const stateTypedef ch1En);
uint8 getAdcCh0AndCh1En(void);

uint8 isAdcAutoOffset(const adcAutoOffsetTypedef mode);

uint8 setAdcIQSwap(const adcChIQSwapTypedef mode);
uint8 getAdcIQSwap(void);

uint8 setAdcInvert(const adcInvertTypedef mode);
uint8 getAdcInvert(void);

uint8 setDownSampleRate(const downSampleRateTypedef div);
uint8 getDownSampleRate(void);

uint8 isDownSampleBypass(const downsampleBypassTypedef div);

void setFirCoef(const firDownsampleStrTypedef firStr);

void setFirMidCoef(const uint16 coef);
uint16 getFirMidCoef(void);

uint16 getIntervalNum(adcClockTypedef adcClk, uint32 t0, uint32 t1, uint32 t2);

uint16 getEndNum(adcClockTypedef adcClk, uint32 t0, uint32 t1, uint32 t2);

void setAllFirCoef(const firDownsampleStrTypedef *firStr);
firDownsampleStrTypedef *getAllFirCoef(void);

// uint32 getIntervalNum(uint32 t0, uint32 t1, uint32 t2);

void setAdcStartNum(const uint16 startNum);
uint16 getAdcStartNum(void);

void setAdcVaildtNum(const uint16 vaildNum);
uint16 getAdcVaildNum(void);

void setAdcIntervalNum(const uint16 intervalNum);
uint16 getAdcIntervalNum(void);

void setDataSampleScanChirpNum(const uint8 scanNum);
uint8 getDataSampleScanChirpNum(void);

uint8 setAdcDmaSelect(const adcDmaSelTypedef select);
uint8 getAdcDmaSelect(void);

uint8 setFmcwSyncOut(const syncOutTypedef out);
uint8 getFmcwSyncOut(void);

uint8 setSyncOutClockEdge(const syncOutEdgeTypedef edge);
uint8 getSyncOutClockEdge(void);

uint8 setCascadeApp(const cascadeModeTypedef mode);
uint8 getCascadeApp(void);

uint8 setCkadcFromSyncin(const cascadeModeTypedef mode);
uint8 getCkadcFromSyncin(void);

uint8 setAdcStartFromSyncin(const cascadeModeTypedef mode);
uint8 getAdcStartFromSyncin(void);

uint8 setAdcStart(const adcStartTypedef mode);
uint8 getAdcStart(void);

void setAdcTestMode(const stateTypedef en);
uint8 getAdcTestMode(void);

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
