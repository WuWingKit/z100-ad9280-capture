/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_rx.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of RX HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_RX_H
#define __ADT6101P_RX_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup RX
 *  @{
 */


/** @addtogroup RX_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
typedef enum
{
    CH0_LNA_BIAS1 = 8,
    CH0_LNA_BIAS2 = 12,
    CH0_LNA_BIAS3 = 16,
    CH1_LNA_BIAS1 = 20,
    CH1_LNA_BIAS2 = 24,
    CH1_LNA_BIAS3 = 28
} lnaBiasChTypedef;

/**
  * @brief  TIA diff gain setting, 0~18dB, 6dB/bit
  */
typedef enum
{
    TIA_GAIN_0DB = 0x0,
    TIA_GAIN_6DB,
    TIA_GAIN_12DB,
    TIA_GAIN_18DB
} tiaDiffGainTypedef;

/**
  * @brief  PGA diff gain, 0~40dB,2dB/bit
  */
typedef enum
{
    PGA_GAIN_0DB = 0,
    PGA_GAIN_2DB,
    PGA_GAIN_4DB,
    PGA_GAIN_6DB,
    PGA_GAIN_8DB,
    PGA_GAIN_10DB,
    PGA_GAIN_12DB,
    PGA_GAIN_14DB,
    PGA_GAIN_16DB,
    PGA_GAIN_18DB,
    PGA_GAIN_20DB,
    PGA_GAIN_22DB,
    PGA_GAIN_24DB,
    PGA_GAIN_26DB,
    PGA_GAIN_28DB,
    PGA_GAIN_30DB,
    PGA_GAIN_32DB,
    PGA_GAIN_34DB,
    PGA_GAIN_36DB,
    PGA_GAIN_38DB,
    PGA_GAIN_40DB
} pgaDiffGainTypedef;

/**
  * @brief  I/Q calibration gain range: -1~1dB, 0.5dB/bit
  */
typedef enum
{
    PGA_CAL_N1 = 0x0,   /*!< -1DB */
    PGA_CAL_N5,         /*!< -0.5DB */
    PGA_CAL_P1,         /*!< 0.5DB */
    PGA_CAL_P5          /*!< 1DB */
} pgaCalTypedef;

/**
  * @brief  Select tia hpf parameter
  */
typedef enum
{
    TIA_HPF_100KHZ = 0x0,
    TIA_HPF_200KHZ
} tiaHpfTypedef;

/**
  * @brief  Select rc hpf
  */
typedef enum
{
    RC_HPF_150KHZ = 150,
    RC_HPF_400KHZ = 400
} rcHpfTyepdef;

/**
  * @brief  Select pga lpf parameter
  */
typedef enum
{
    PGA_LPF_2M = 0x0,
    PGA_LPF_4M,
    PGA_LPF_6M,
    PGA_LPF_8M
} pgaLpfTypedef;

/**
  * @brief  Select pga hpf parameter
  */
typedef enum
{
    PGA_HPF_100KHZ = 0x0,
    PGA_HPF_500KHZ,
} pgaHpfTypedef;
/**
  * @}
  */


/* Private functions ---------------------------------------------------------*/
/** @addtogroup RX_Private_Functions
  * @{
  */
void rxLnaEnable(const stateTypedef lna0Enable, const stateTypedef lna1Enable);

void mixerEnable(const stateTypedef ch0MixerEnable, const stateTypedef ch1MixerEnable);

void lnaDacBias(const lnaBiasChTypedef chxBias, const uint8 val);

void rfTiaEnable(const stateTypedef tia0Enable, const stateTypedef tia1Enable);

void rfTiaGainSetting(const tiaDiffGainTypedef tia0IGain, const tiaDiffGainTypedef tia0QGain, \
                      const tiaDiffGainTypedef tia1IGain, const tiaDiffGainTypedef tia1QGain);

void rfTiaDacBias(const uint8 tiaBiasI, const uint8 tiaBiasQ);

/*                        VGA                             */
void rfVgaEnable(const stateTypedef pga0Enable, const stateTypedef pga1Enable);

void rfVgaGainSetting(const pgaDiffGainTypedef vga0IGain, const pgaDiffGainTypedef vga0QGain, \
                      const pgaDiffGainTypedef vga1IGain, const pgaDiffGainTypedef vga1QGain);

void rfPgaGainCalEnable(const stateTypedef pga0ICalEnable, const stateTypedef pga0QCalEnable,
                        const stateTypedef pga1ICalEnable, const stateTypedef pga1QCalEnable);
void rfPgaGainCalSetting(const pgaCalTypedef pga0ICal, const pgaCalTypedef pga0QCal, \
                         const pgaCalTypedef pga1ICal, const pgaCalTypedef pga1QCal);

void rfVgaDacBias(const uint8 pgaBiasI, const uint8 pgaBiasQ);

void tiaHpfConfig(const stateTypedef tia0HpfEnable, const stateTypedef tia1HpfEnable, \
                  const tiaHpfTypedef tia0HpfParameter, const tiaHpfTypedef tia1HpfParameter);

void rcHpfConfig(const stateTypedef  rcHpf0Enable, const stateTypedef  rcHpf1Enable, \
                 const rcHpfTyepdef rc0HpfParameter, const rcHpfTyepdef rc1HpfParameter);

void pgaLpfConfig(const pgaLpfTypedef pga0LpfParameter, const pgaLpfTypedef pga1LpfParameter);

void pgaLpfCal(int32 lpf0CalEn, int32 lpf1CalEn, int32 lpf0CalConf, int32 lpf1CalConf);

void pgaHpfConfig(const stateTypedef pga0HpfEnable, const stateTypedef pga1HpfEnable, \
                  const pgaHpfTypedef pga0HpfParameter, const pgaHpfTypedef pga1HpfParameter);

void setPgaOutputBuff(const stateTypedef pga0BuffEn, const stateTypedef pga1BuffEn);

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
