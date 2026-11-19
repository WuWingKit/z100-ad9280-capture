/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_pmu.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of PMU HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_PMU__H
#define __ADT6101P_PMU__H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "pmu_ctype_map.h"
#include "rfc_ctype_map.h"

/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup PMU
 *  @{
 */


/** @addtogroup PMU_Private_Macros
  * @{
  */
/* Private typedef -----------------------------------------------------------*/
/**
  * @brief  Select 32k_high source
  */
typedef enum
{
    CLK_32k_RCSOR = 0x0,
    CLK_32k_XOSOR,
    CLK_32k_DIVHIGHSOR
} clk32kSrcTypedef;

/**
  * @brief  Select rc32k cali clock, based on hclk
  */
typedef enum
{
    CLK_HIGH = 0x0,
    CLK_HIGH_DIV_2,
    CLK_HIGH_DIV_4,
    CLK_HIGH_DIV_8,
    CLK_HIGH_DIV_16,
    CLK_HIGH_DIV_32,
    CLK_HIGH_DIV_64
} AHBClockTypedef;

/**
  * @brief  Select can cali clock, based on hclk
  */
typedef enum
{
    CAN_CLK_HIGH_DIV_1 = 0x0,
    CAN_CLK_HIGH_DIV_2,
    CAN_CLK_HIGH_DIV_4,
    CAN_CLK_HIGH_DIV_8
} canClockTypedef;

typedef enum
{
    POWER_ON = 0x0,     /*!< power on 0 valid */
    POWER_DOWN          /*!< power down 1 valid */
} powerStateTypedef;

typedef enum
{
    PMU_IDLE = 0x0,
    PMU_POWERING_DOWN,
    PMU_POWERED_DOWN,
    PMU_POWERING_UP,
    PMU_POWERED_UP
} pmuStateTypedef;

/**
  * @brief  External clock gate en for each apb/ahb reg intf
  */
typedef enum
{
    CLK_GATING_PMU_APB = 0x0,
    CLK_GATING_MCU_AHB,
    CLK_GATING_I2C1_APB,
    CLK_GATING_I2C0_APB,
    CLK_GATING_GPIO_AHB,
    CLK_GATING_DMA_AHB,
    CLK_GATING_RFC_APB,
    CLK_GATING_DSP_AHB,
    CLK_GATING_FMCW_DSM,
    CLK_GATING_CNR_HIGH,
    CLK_GATING_FIR_FIFO,
    CLK_GATING_SRAM,
    CLK_GATING_IRAM,
    CLK_GATING_ROM
} extClkTypedef;

typedef enum
{
    CTRL_CLK_I2C1_EN = 0x0,
    CTRL_CLK_I2C0_EN,
    CTRL_CLK_SSP0_EN,
    CTRL_CLK_SSP1_EN,
    CTRL_CLK_TRNG_EN,
    CTRL_CLK_CALI_EN,
    CTRL_CLK_IFADC_EN,
    CTRL_CLK_FIR_EN,
    CTRL_CLK_FFT_EN,
    CTRL_CLK_DSAMPLE_EN,
    CTRL_CLK_ADC_TEST_12B_EN,
    CTRL_CLK_ADC_TEST_EN,
    CTRL_CLK_FMCW_TEST_EN,
    CTRL_CLK_DSM_TEST_EN,
    CAN0_HOST_CLK_EN,
    CAN0_CAN_CLK_EN,
    CAN0_TIMER_CLK_EN
} ctrlClkTypedef;

typedef enum
{
    POR_INPUT = 0x0,
    POR_OUTPUT
} selPorTypedef;

/**
  * @brief  Voltage control for LDO12 low power mode when in sleep mode
  */
typedef enum
{
    LDO12_VOL_1_DEC_2 = 0x0,
    LDO12_VOL_0_DEC_7,
    LDO12_VOL_0_DEC_75,
    LDO12_VOL_0_DEC_8,
    LDO12_VOL_0_DEC_85,
    LDO12_VOL_1_DEC_0,
    LDO12_VOL_1_DEC_1,
    LDO12_VOL_1_DEC_3
} ldo12LPVolTypdef;

typedef enum
{
    CTRL_RST_I2C1 = 0x0,
    CTRL_RST_I2C0,
    CTRL_RST_SSP0,
    CTRL_RST_SSP1,
    CTRL_RST_TRNG,
    CTRL_RST_CALI,
    CTRL_RST_IFADC,
    CTRL_RST_FIR,
    CTRL_RST_FFT,
    CTRL_RST_DSAMPLE,
    CTRL_RST_LOCK_DET,
    CAN0_HOST_RST,
    CAN0_CAN_RST,
    CAN0_TIMER_RST
} pmuSoftRstTypedef;
/**
  * @}
  */


/* Private functions ---------------------------------------------------------*/
/** @addtogroup PMU_Private_Functions
  * @{
  */
void clearAllWakeUpEn(void);
void setIoWakeUpEn(const stateTypedef en);
uint8 getIoWakeUpEn(void);

void setRtcTimerWakeUpEn(const stateTypedef en);
uint8 getRtcTimerWakeUpEn(void);

void setDebugWakeUpEn(const stateTypedef en);
uint8 getDebugWakeUpEn(void);

void setPoweringDownTime(uint16 val);
uint16 getPoweringDownTime(void);

void setPoweringUpTime(uint16 val);
uint16 getPoweringUpTime(void);

void set32kClkSrc(const clk32kSrcTypedef sel);
uint8 get32kClkSrc(void);

void setCanClkDiv(const canClockTypedef sel);
uint8 getCanClkDiv(void);

void setXo50MBias(const uint8 val);
uint8 getXo50MBias(void);

void setXo50MBiasAndCap(const uint8 bias, const uint8 cap);
uint8 getXo50MCap(void);

void setSlp(const uint32 xo50M, const uint32 plliCtrl, const uint32 ldoBg);
void setSlpPlliCtrl(const uint32 val);
uint32 getSlpXo50M(void);
uint32 getSlpPlliCtrl(void);
uint32 getSlpLdoBg(void);
void setSlpPowerCtrl(const uint8 val);
uint8 getSlpPowerCtrl(void);

void setLdoLpVol(const ldo12LPVolTypdef voltage);
uint8 getLdoLpVol(void);

void setPlliDaca(const uint32 val);
uint32 getPlliDaca(void);
void setPlliDacb(const uint32 val);
uint32 getPlliDacb(void);

void setAHBClock(const AHBClockTypedef clock);
uint8 getAHBClock(void);

void setManualCtrlToLdoAndClk(const stateTypedef en);
uint8 getManualCtrlToLdoAndClk(void);

void setLdo12PowerDown(const powerStateTypedef state);
uint8 getLdo12PowerDown(void);

void setPmuStatus(const pmuStateTypedef state);
uint8 getPmuStatus(void);

void setExtClkEn(const stateTypedef en, const extClkTypedef clkGating);
uint8 getExtClkEn(const extClkTypedef clkGating);

void setClkGateEn(const stateTypedef en, const ctrlClkTypedef ctrl);
uint8 getClkGateEn(const ctrlClkTypedef ctrl);

void setSoftwareReset(const stateTypedef en, const pmuSoftRstTypedef ctrl);
uint8 getSoftwareReset(const pmuSoftRstTypedef ctrl);

void setRc32kCailFilter(const uint8 val);
uint8 getRc32kCailFilter(void);

void setRc32kCail(const uint8 val);
uint8 getRc32kCail(void);

void setPorTest(const selPorTypedef sel);
uint8 getPorTest(void);

void rc32kOutputP30(void);

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
