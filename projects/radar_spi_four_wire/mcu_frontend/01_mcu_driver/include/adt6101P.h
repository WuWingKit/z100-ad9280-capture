/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P.h
 * @author
 * @version 1.0.0
 * @date    2024-01-28
 * @brief   This file is a millimeter wave radar message structure, used as a
 *          millimeter wave radar configuration box
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __ADT6101P__H
#define __ADT6101P__H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_adc.h"
#include "adt6101P_type_define.h"
#include <arm_math.h>


/** @addtogroup APP
 *  @{
 */

/** @addtogroup ADT6101P_MCB
 *  @{
 */

/* Exported types ------------------------------------------------------------*/
/** @addtogroup Debug_Output_Exported_Macros
  * @{
  */
#define ANALOG_IF_OUTPUT                    0               /*!< analog if output define e.g.,output result, analog IF sign enbuffer */
#define APP_DEBUG_ADC_SINGLE_ENABLE         0               /*!< 1: Enable ADC, 0: Disable ADC, default: second chirp */
#define APP_DEBUG_FFT_SINGLE_ENABLE         0               /*!< 1: Enable FFT, 0: Disable FFT, default: second chirp */
#define APP_DEBUG_RANGE_FFT_ENABLE          0               /*!< 1: Enable RANGE FFT, 0: Disable RANGE FFT, default: second chirp */
#define APP_DEBUG_VELOCITY_FFT_ENABLE       0               /*!< 1: Enable VELOCITY FFT, 0: Disable VELOCITY FFT, default: second chirp */
#define APP_DEBUG_ABS_FFT_ENABLE            0               /*!< 1: Enable ABS FFT, 0: Disable ABS FFT, default: second chirp */
#define APP_DEBUG_ABS_MERGE_ENABLE          0               /*!< 1: Enable ABS MERGE, 0: Disable ABS MERGE, default: second chirp */
#define APP_DEBUG_CFAR_ENABLE               0               /*!< 1: Enable CFAR, 0: Disable CFAR, default: second chirp */
/**
  * @}
  */

/** @addtogroup RF_Exported_Macros
  * @{
  */
#define TX0_EN                              DISABLE          /*!< Tx0 channel ENABLE or DISABLE, see also @ref stateTypedef */
#define TX1_EN                              DISABLE         /*!< Tx1 channel ENABLE or DISABLE, see also @ref stateTypedef */
#define TD_MIMO_EN                          DISABLE         /*!< Enable TD_MIMO, Disable TD_MIMO */
#define PA0_GAIN                            0              /*!< Tx0 PA gain, Range: 0~15, step: 1 */
#define PA1_GAIN                            0               /*!< Tx1 PA gain, Range: 0~15, step: 1 */

#define RX0_EN                              ENABLE          /*!< Rx0 channel ENABLE or DISABLE, see also @ref stateTypedef */
#define RX1_EN                              ENABLE          /*!< Rx1 channel ENABLE or DISABLE, see also @ref stateTypedef */
#define TIA0_GAIN                           TIA_GAIN_18DB    /*!< set RX0 TIA GAIN, Range: 0DB~18DB, step: 6DB, see also @ref tiaDiffGainTypedef */
#define PGA0_GAIN                           PGA_GAIN_38DB   /*!< set RX0 PGA GAIN, Range: 0DB~40DB, step: 2DB, see also @ref pgaDiffGainTypedef */
#define TIA1_GAIN                           TIA_GAIN_18DB    /*!< set RX1 TIA GAIN, Range: 0DB~18DB, step: 6DB, see also @ref tiaDiffGainTypedef */
#define PGA1_GAIN                           PGA_GAIN_38DB   /*!< set RX1 PGA GAIN, Range: 0DB~40DB, step: 2DB, see also @ref pgaDiffGainTypedef */

#define TIA_HPF_EN                          ENABLE          /*!< RX TIA HPF ENABLE or DISABLE, see also @ref stateTypedef */
#define RC_HPF_EN                           ENABLE          /*!< RX RC HPF ENABLE or DISABLE, see also @ref stateTypedef */
#define PGA_HPF_EN                          ENABLE          /*!< RX PGA HPF ENABLE or DISABLE, see also @ref stateTypedef */
#define TIA_HPF                             TIA_HPF_100KHZ  /*!< set RX TIA HPF, Range: 25KHz~300KHz, see also @ref tiaHpfTypedef */
#define RC_HPF                              RC_HPF_150KHZ   /*!< set RC HPF, Range: 150KHz~400KHz, see also @ref rcHpfTyepdef */
#define PGA_HPF                             PGA_HPF_100KHZ  /*!< set PGA HPF, Range: 100KHz~400KHz, see also @ref pgaHpfTypedef */
#define PGA_LPF                             PGA_LPF_4M      /*!< set PGA LPF, Range: 2M~8M, step: 2M, see also @ref pgaLpfTypedef */


#define ADT6101P_UART0_COMM       (1)
#define ADT6101P_UART1_COMM       (2)
#define ADT_UART_COMM_SELECT  ADT6101P_UART0_COMM
/**
  * @}
  */

/** @addtogroup FMCW_CALCULATE_Exported_Macros
  * @{
  */
#define ADC_SAMPLE_RATE                     ADC_CLOCK_7P14M  /*!< set the ADC clock frequency, Range: 8.33MHz, 16.667MHz, see also @ref adcClockTypedef */
#define FIR_EN                              FIR_ENABLE      /*!< FIR ENABLE or DISABLE */
#define DOWN_SAMPLE                         DOWNSAMPLE_NO    /*!< Downsampling coefficient, Range: 0~16, step: 1, see also @ref downSampleRateTypedef */
#define FSTART                              58.5              /*!< Starting frequency, Range: 76~82GHz */
#define FM                                  1400            /*!< Frequency Modulation, Range: 0~4000MHz */
#define SAMPLE_POINT                        512             /*!< The number of samping points for ADC */
#define RANGE_STEP                          0          /*!< Range step, unit: m */
#define CHIRP_T0                            (84)              /*!< Rise edge time, uints: us */
#define CHIRP_T1                            (14)           /*!< Falling edge time, uints: us */
#define CHIRP_T2                            (0)             /*!< Interval time between chirps */
#define CHIRP_PERIOD                        (CHIRP_T0 + CHIRP_T1 + CHIRP_T2) /*!< Chirp cycle, uints: us */
#define CHIRP_NUM                           2                  /*!< The number of chirps transmiited per frame in FMCW */
#define CHIRP_NUM2                          (CHIRP_NUM / 2)     /*!< The number of chirp cpllected by each ADC channel */
#define CHIRP_NUM2_LOG2                     1                   /*!< index */
#define SPEED_STEP                          (0)        /*!< Speed step, unit: cm/s */
#define RANGE_MIN                           3                   /*!< Minimum distance step, distance(min) = RANGE_STEP * RANGE_MIN. */
#define RANGE_MAX                           (86)     /*!< Maximum distance step, distance(max) = RANGE_STEP * RANGE_MAX. */
#define FRAME_CONCTRL                       (20000)  


/**
  * @}
  */

/** @addtogroup ADC_Exported_Macros
  * @{
  */
#define ADC_SAMPLE_SEL                      SAMPLE_ADC0    /*!< Enable ADC acquisition channel, see also @ref adcSampleSelTypedef */
#define ADC_SAMPLE_INTERVAL_NUM             getIntervalNum(ADC_SAMPLE_RATE, CHIRP_T0, CHIRP_T1, CHIRP_T2)     /*!< ADC sampling interval numbers */
#define ADC_SAMPLE_END_NUM                  getEndNum(ADC_SAMPLE_RATE, CHIRP_T0, CHIRP_T1, CHIRP_T2)          /*!< The total number of ADC sampling numbers CHIRP_T1 + CHIRP_T2 */
/**
  * @}
  */

/** @addtogroup Windows_Exported_Macros
  * @{
  */
#define ADC_WN_MODE                         WINDOW_HAMMING              /*!< The type of window added, see also @ref windowModeTypedef */
#define ADC_WN_LEN                          WINDOW_LEN_512              /*!< The length of window added, see also @ref windowLenTypedef */
#define ADC_WN_EN                           ADC_WINDOW_BYPASS           /*!< ADC window ENABLE or DISBALE, see also @ref adcWnTypedef */
#define FFT_WN_EN                           FFT_WINDOW_ENABLE           /*!< FFT window ENABLE or DISBALE, see also @ref fftWnTypedef */
/**
  * @}
  */


/** @addtogroup Windows_Exported_Macros
  * @{
  */
/**
  * @brief ADC DSP(digital signal processing) macro definition
  */
#define ADC_CH0_ADDR                        0x20008000
#define ADC_CH1_ADDR                        0x20008000

/**
  * @brief Memory scheduling DSP(digital signal processing) macro definition
  */
#define DSP_MEMSCHED_BUFFER_ADDR            0x20028000

/**
  * @brief Range-FFT DSP(digital signal processing) macro definition
  */
#define RNG_FFT_CH0_ODD_ADDR                ADC_CH0_ADDR
#define RNG_FFT_CH1_ODD_ADDR                ADC_CH1_ADDR

/**
  * @brief Velocity-FFT DSP(digital signal processing) macro definition
  */
#define DOP_FFT_CH0_ODD_ADDR                (ADC_CH0_ADDR + SAMPLE_POINT * CHIRP_NUM2 * 4)
#define DOP_FFT_CH1_ODD_ADDR                (ADC_CH1_ADDR + SAMPLE_POINT * CHIRP_NUM2 * 4)

/**
  * @brief ABS DSP(digital signal processing) macro definition
  */
#define ABS_CH0_ODD_ADDR                    ADC_CH0_ADDR
#define ABS_CH1_ODD_ADDR                   (ADC_CH1_ADDR)
/**
  * @brief ABS merge DSP(digital signal processing) macro definition
  */
#define ABS_MERGE_ADDR                      (ADC_CH1_ADDR)

/**
  * @}
  */
#define SAGC_EN                             1   /*!< 1: Enable automatic gain, 0: Disable automatic gain */
#define FRAME_LOWPOWER                      1   /*!< 1: Enable RF low power consumption, 0: Disable RF low power consumption */
#define SLEEP_LOWPOWER                      0   /*!< 1: Enable Sleep 0: Disable Sleep */
#define STATIC_REMOVE                       1

#if 0
    /*!< Dn = 2 */
    #define FIRREG0   0x1FFF
    #define FIRREG1   0x2001
    #define FIRREG2   0x3FFBFFF
    #define FIRREG3   0xA000
    #define FIRREG4   0x3FF4003
    #define FIRREG5   0x9FF7
    #define FIRREG6   0x6010
    #define FIRREG7   0x3FDFFEA
    #define FIRREG8   0x4A016
    #define FIRREG9   0x3F8DFF6
    #define FIRREG10  0x8DFEE
    #define FIRREG11  0x3F84040
    #define FIRREG12  0x23F83
    #define FIRREG13  0xB00C0
    #define FIRREG14  0x3D85F03
    #define FIRREG15  0x9DE129
    #define FIRREG16  0x6C8
#endif
#if 1
    #define FIRREG0   0x0
    #define FIRREG1   0x0
    #define FIRREG2   0x0
    #define FIRREG3   0x0
    #define FIRREG4   0x0
    #define FIRREG5   0x0
    #define FIRREG6   0x0
    #define FIRREG7   0x0
    #define FIRREG8   0x0
    #define FIRREG9   0x0
    #define FIRREG10  0x0
    #define FIRREG11  0x0
    #define FIRREG12  0x0
    #define FIRREG13  0x0
    #define FIRREG14  0x0
    #define FIRREG15  0x0
    #define FIRREG16  0xfff
#endif


/**
 * @brief 简化版工程不再使用通用状态调度配置。
 * @note 驱动公共配置头只保留硬件与业务参数，避免与应用状态机耦合。
 */
/**
  * @}
  */
/**
  * @}
  */

#endif
