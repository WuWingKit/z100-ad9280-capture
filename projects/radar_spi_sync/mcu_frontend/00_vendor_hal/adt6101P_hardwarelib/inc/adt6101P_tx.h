/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_tx.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-19
 * @brief   Header file containing functions prototypes of TX library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_TX_H
#define __ADT6101P_TX_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup TX
 *  @{
 */


/** @addtogroup TX_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
/**
  * @brief  Select FMCW mode
  */
#define MODE_INTER      00      /*!< 20G to internal doubler */
#define MODE_EXTERN     01      /*!< 20G from external source */
#define MODE_CASCADED   10      /*!< 20G to internal doubler and output other chip */
#define MODE_TEST       11      /*!< 20G output test port */

/**
  * @brief  Set baseband ldo voltage x.yv
  */
#define BB_LDO_1P8      0
#define BB_LDO_2P0      1
#define BB_LDO_2P2      2
#define BB_LDO_2P5      3

/**
  * @brief  Set VDD ldo voltage x.yv
  */
#define DVDD1P2_HP      0
#define DVDD1P1_HP      1
#define DVDD1P3_HP      2
#define DVDD1P4_HP      3

/**
  * @brief  Set VDD ldo voltage
  */
#define DVDD1P2_LP      0
#define DVDD0P7_LP      1
#define DVDD0P75_LP     2
#define DVDD0P8_LP      3
#define DVDD0P85_LP     4
#define DVDD1P0_LP      5
#define DVDD1P1_LP      6
#define DVDD1P3_LP      7

/**
  * @brief  set PLLN VCO LDO voltage x.yv
  */
#define PLLN_VCO_LDO_1P8    0
#define PLLN_VCO_LDO_2P0    1
#define PLLN_VCO_LDO_2P2    2
#define PLLN_VCO_LDO_2P5    3

/**
  * @brief  set PLLN PLL LDO voltage x.yv
  */
#define PLLN_PLL_LDO_1P8    0
#define PLLN_PLL_LDO_2P0    1
#define PLLN_PLL_LDO_2P2    2
#define PLLN_PLL_LDO_2P5    3

/**
  * @brief  GPADC select
  */
#define GPADC_AIO1      1     /*!< temperature Compensation */
#define GPADC_IN2       2     /*!< VBE */
#define GPADC_IN3       3
#define GPADC_IN4       4
#define GPADC_IN5       5     /*!< powerdector */
#define GPADC_IN6       6     /*!< VBG */
#define BG_ATB_OUT      7
#define VGA_ATB_OUT     8
#define GPADC_IN18_1    9
#define GPADC_IN18_2    10    /*!< vtune */
#define GPADC_IN18_3    11
#define ADC_ATB_OUT     12

#define VDD_GPADC       (2.5)  //VDD GPADC is 2.5V

/**
  * @brief  Select the MIMO mode
  */
typedef enum
{
    NO_MIMO = 0,
    TD_MIMO,
    BP_MIMO
} mimoModeTypedef;

/**
  * @brief  Select FMCW chirp source
  */
typedef enum
{
    REGISTER = 0x0,         /*!< ready for start FMCW chirp from DATA_SAMPLE_MODULE */
    DATA_SAMPLE_MODULE      /*!< ready for start FMCW chirp from REGISTER */
} fmcwEnSelectTypedef;

/**
  * @brief  Select the FMCW chirp mode
  */
typedef enum
{
    CONTINUE_MODE = 0x0,    /*!< FMCW will be in a continuously emitting chirp mode. This mode is
                                generally used for testing phase noise and SNR of chirps.
                                It is actually useful in basic engineering! */
    SINGLE_MODE             /*!< FMCW will emit an effective number of chirps. This mode is generally
                                used in application engineering. The number of chirp emissions comes from
                                void setChirpCycleNum(const uint16 num) (in the adt6101P_tx.c file) */
} fmcwModeTypedef;

/**
  * @brief  Select the FMCW chirp channel
  */
typedef enum
{
    CHIRP_CHANNEL_1 = 0x0,
    CHIRP_CHANNEL_2,
    CHIRP_CHANNEL_3,
    CHIRP_CHANNEL_4,
    CHIRP_CHANNEL_5,
    CHIRP_CHANNEL_6,
    CHIRP_CHANNEL_7,
    CHIRP_CHANNEL_8,
    CHIRP_CHANNEL_9,
    CHIRP_CHANNEL_10,
    CHIRP_CHANNEL_11
} chirpChTypedef;

/**
  * @brief  Select the mode of FMCW chirp channel
  */
typedef enum
{
    CHIRP_CH_UP_MODE = 0x0,
    CHIRP_CH_DOWN_MODE,
    CHIRP_CH_HOLD_MODE,
    CHIRP_CH_RESET_MODE
} chirpChModeTypedef;

typedef enum
{
    CHIRP_CH_NOT_CLEAR = 0x0,
    CHIRP_CH_CLEAR
} chirpChClearTypedef;
/**
  * @}
  */


/* Private functions ---------------------------------------------------------*/
/** @addtogroup PLLI_Private_Functions
  * @{
  */
void pllLDOEnable(int32 plliVcoLdoEnable, int32 plliPllLdoEnable);

void plliLock(void);
/**
  * @}
  */


/** @addtogroup PLLN_Private_Functions
  * @{
  */
void pllVcoConfig(int32 pllnVcoLdoEnable, int32 pllnVcoLdo);

void pllPllConfig(int32 pllnPllLdoEnable, int32 pllnPllLdo, int32 pllnEnBias);

void plliDacSetting(void);

void pllnConfig(int32 vcoEn);

void pllVcoEnable(int32 vcoEn);
void pllBufEnable(int32 bufEn);
void pllMMDEnable(int32 MMDEn);

void setPllVcoCap(uint8 val);
uint8 getPllVcoCap(void);

uint8 getPllnLockStatus(void);
/**
  * @}
  */


/** @addtogroup LDO_Private_Functions
  * @{
  */
void baseband18LDO(int32 bb1Enable, int32 bb1Adjust, int32 bb2Enable, int32 bb2Adjust);
void dvdd12HpLDO(int32 powerOnDown, int32 dvdd12Adjust);
void dvdd12LpLDO(int32 powerOnDown, int32 dvdd12Adjust);
/**
  * @}
  */


/** @addtogroup Mixer_Private_Functions
  * @{
  */
void rfbuff20G(const stateTypedef buffer20gEn, const stateTypedef triplerEn);

void bias20GTo60G(int32 biasdb60G, int32 biasBuff38, int32 biasDb38, int32 bias2Buff20G,
                  int32 biasbuff20G, int32 txBuff1To4, int32 loBuff1To4, int32 rxBuff1To4);

void txMIMO(int32 mimoEn);

void dacEnBias1Pd(int32 dacEnable, int32 Bias1PowerDown);

void biasPowerdector(int32 biasPowerDector);

void biasGpadc(int32 gpadcBias1test, int32 gpadcBias2test, int32 gpadcBias);
void bistOn(uint16 bistSorSel, uint16 bistDivSel);
void bistOff(uint16 bistSorSel, uint16 bistDivSel);

void rfBuff60GEnable(const stateTypedef stage1Buff60G, \
                     const stateTypedef tx0BufferEn60G, const stateTypedef tx1BufferEn60G,
                     const stateTypedef rx0BufferEn60G, const stateTypedef rx1BufferEn60G);

void set60GBuffTx0En(const stateTypedef tx0En);
void set60GBuffTx1En(const stateTypedef tx1En);
uint8 get60GBuffTxEn(void);
/**
  * @}
  */


/** @addtogroup PA_Private_Functions
  * @{
  */
void txPaEnable(const stateTypedef pa0Enable, const stateTypedef pa1Enable);

void txPaBias(const uint8 pa0Bias, const uint8 pa1Bias);

void rfPhaseShift(int32 ps_ip, int32 ps_in, int32 ps_qp, int32 ps_qn);
void rfPhaseShiftLUT(uint8 ch0_idx, uint8 ch1_idx);
/**
  * @}
  */


/** @addtogroup FMCW_Private_Functions
  * @{
  */
void fmcwModeSel(int32 fmcwMode);

void temperatureCompensation(uint32 miniBais, uint32 kc1Reg, uint32 kc2Reg);

void fmcwEnmode(const fmcwEnSelectTypedef startMode, const fmcwModeTypedef chirpMode);
void waitFmcwcycEnd(void);

void tdMimo(float t0, float t1, float t2);

uint8 setEnableDitherForDSM(const stateTypedef en);
uint8 getEnableDitherForDSM(void);

void setFmcwStartFreq(const uint32 fstart);
uint32 getFmcwStartFreq(void);

void setChirpChannelNum(const chirpChTypedef num);
uint8 getChirpChannelNum(void);

void setChirpCycleNum(const uint16 num);
uint16 getChirpCycleNum(void);

void setChirpChannelStepNum(const chirpChTypedef chIndex, const uint32 numStep);
uint32 getChirpChannelStepNum(const chirpChTypedef chIndex);

void setChirpChannelDtStep(const chirpChTypedef chIndex, const uint8 dtStep);
uint8 getChirpChannelDtStep(const chirpChTypedef chIndex);

void setChirpChannelDfStep(const chirpChTypedef chIndex, const uint16 dfStep);
uint16 getChirpChannelDfStep(const chirpChTypedef chIndex);

void setChirpChannelMode(const chirpChTypedef chIndex, const chirpChModeTypedef chMode);
uint8 getChirpChannelMode(const chirpChTypedef chIndex);

void chirpChannelClear(const chirpChTypedef chIndex, const chirpChClearTypedef clear);

uint8 setMimoType(const mimoModeTypedef mode, const uint32 chirpChBits, const uint32 chirpChNum);

void fmcwChirpSingleFrequency(void);
void widenBwP27(float chirpT0);

uint32 getFmcwIntState(void);
void clearFmcwInt(uint32 state);


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
