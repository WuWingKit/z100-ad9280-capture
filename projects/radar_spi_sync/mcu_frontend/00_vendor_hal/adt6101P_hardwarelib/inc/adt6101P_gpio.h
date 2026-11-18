/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_gpio.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   Header file containing functions prototypes of GPIO HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_GPIO_H
#define __ADT6101P_GPIO_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup GPIO
 *  @{
 */


/* Private types -------------------------------------------------------------*/
/** @addtogroup GPIO_Private_Macros
  * @{
  */
#define GPIO_CONFIG_OFFSET         0x0000000C
#define GPIO_CONFIG_REGS           GPIO_REGS + GPIO_CONFIG_OFFSET

//#define gpio_config 0x0c
#define gpio_config_smt_shift     (11)
#define gpio_config_smt_bit       (1 << 11)
#define gpio_config_e_shift       (9)
#define gpio_config_e_mask        ((1 << 2) - 1)
#define gpio_config_sr_shift      (8)
#define gpio_config_sr_bit        (1 << 8)
#define gpio_config_pu_shift      (7)
#define gpio_config_pu_bit        (1 << 7)
#define gpio_config_pd_shift      (6)
#define gpio_config_pd_bit        (1 << 6)
#define gpio_config_ie_shift      (5)
#define gpio_config_ie_bit        (1 << 5)
#define gpio_config_oen_shift     (4)
#define gpio_config_oen_bit       (1 << 4)
#define gpio_config_pid_shift     (0)
#define gpio_config_pid_mask      ((1 << 4) - 1)

/**
  * @brief  Configuration Mode enumeration
  */
typedef enum
{
    GPIO_MODE_OUTPUT            = 0x0,
    GPIO_MODE_INPUT             = 0x1,
    GPIO_MODE_HIGH_INPENDECE    = 0x2,
    GPIO_MODE_OUTPUT_AND_INPUT  = 0x3,
} gpioModeTypedef;

/**
  * @brief  Configuration Pull enumeration
  */
typedef enum
{
    GPIO_NO_PULL     = 0x0, /*!< No Pull-up or Pull-down activation  */
    GPIO_PULL_UP     = 0x1, /*!< Pull-up activation                  */
    GPIO_PULL_DOWN   = 0x2  /*!< Pull-down activation                */
} gpioPullTypedef;

/**
  * @brief  Configuration Drive enumeration
  */
typedef enum
{
    GPIO_DRIVE_2mA     = 0x0,
    GPIO_DRIVE_4mA     = 0x1,
    GPIO_DRIVE_8mA     = 0x2,
    GPIO_DRIVE_12mA    = 0x3
} gpioDriveTypedef;

/**
  * @brief  Configuration trigger enumeration
  */
typedef enum
{
    LEVEL_MODE_HIGH_LEVEL = 0x0,
    LEVEL_MODE_LOW_LEVEL  = 0x1,
    EDGE_MODE_POSEDGE     = 0x2,
    EDGE_MODE_NEGEDGE     = 0x3
} triggerTypedef;

/**
  * @brief  Configuration sr enumeration
  */
typedef enum
{
    SLOW = 0x0,
    FAST = 0x1
} srTypedef;

/**
  * @brief  Configuration smt enumeration
  */
typedef enum
{
    NO_HYSTERESIS   = 0x0,
    SCHMITT_TRIGGER = 0x1
} smtTypedef;

/**
  * @brief  GPIO Bit SET and Bit RESET enumeration
  */
typedef enum
{
    GPIO_PIN_RESET = 0u,
    GPIO_PIN_SET
} gpioPinStateTypedef;

/**
  * @brief GPIO Wake Up definition
  */
typedef enum
{
    HIGH_LEVEL = 0,
    LOW_LEVEL  = 1
} gpioWakeUpTypedef;

/**
  * @brief GPIO_pins_config GPIO pins enumeration
  */
typedef enum
{
    GPIO_CFG_P0     = 0,
    GPIO_CFG_P1     = 1,
    GPIO_CFG_P2     = 2,
    GPIO_CFG_P3     = 3,
    GPIO_CFG_P4     = 4,
    GPIO_CFG_P5     = 5,
    GPIO_CFG_P6     = 6,
    GPIO_CFG_P7     = 7,
    GPIO_CFG_P8     = 8,
    GPIO_CFG_P9     = 9,
    GPIO_CFG_P10    = 10,
    GPIO_CFG_P11    = 11,
    GPIO_CFG_P12    = 12,
    GPIO_CFG_P13    = 13,
    GPIO_CFG_P14    = 14,
    GPIO_CFG_P15    = 15,
    GPIO_CFG_P16    = 16,
    GPIO_CFG_P17    = 17,
    GPIO_CFG_P18    = 18,
    GPIO_CFG_P19    = 19,
    GPIO_CFG_P20    = 20,
    GPIO_CFG_P21    = 21,
    GPIO_CFG_P22    = 22,
    GPIO_CFG_P23    = 23,
    GPIO_CFG_P24    = 24,
    GPIO_CFG_P25    = 25,
    GPIO_CFG_P26    = 26,
    GPIO_CFG_P27    = 27,
    GPIO_CFG_P28    = 28,
    GPIO_CFG_P29    = 29,
    GPIO_CFG_P30    = 30

} gpioConfigPinTypedef;

/**
  * @brief  Configuration Pull enumeration
  */
typedef enum
{
    PID_GPIO              = 0x0,

    PID_UART0_TX          = 0x1,    /*!< P00 */
    PID_UART0_RX          = 0x1,    /*!< P01 */

    PID_TIMER0_EXTIN      = 0x1,    /*!< P02 */

    PID_SPI0_CLK_MASTER   = 0x1,    /*!< P03 */
    PID_SPI0_CLK_SLAVE    = 0x2,

    PID_SPI0_CS_MASTER    = 0x1,    /*!< P04 */
    PID_SPI0_CS_SLAVE     = 0x2,

    PID_SPI0_MISO_MASTER  = 0x1,    /*!< P05 */
    PID_SPI0_MISO_SLAVE   = 0x2,

    PID_ADC_DCKIN_0       = 0x5,
    PID_ADC_DCKIN_1       = 0x6,

    PID_SPI0_MOSI_MASTER  = 0x1,    /*!< P06 */
    PID_SPI0_MOSI_SLAVE   = 0x2,

    PID_IIC0_SCL          = 0x1,    /*!< P07 */
    PID_DSM_DIVN0         = 0x2,

    PID_IIC0_SDA          = 0x1,    /*!< P08 */
    PID_DSM_DIVN1         = 0x2,

    PID_DSM_DIVN2         = 0x2,    /*!< P09 */

    PID_UART1_TX          = 0x1,    /*!< P10 */
    PID_DSM_DIVN3         = 0x2,

    PID_UART1_RX          = 0x1,    /*!< P11 */
    PID_DSM_DIVN4         = 0x2,

    PID_TIMER1_EXTIN      = 0x1,    /*!< P12 */

    PID_SPI1_CLK_MASTER   = 0x1,    /*!< P13 */
    PID_SPI1_CLK_SLAVE    = 0x2,

    PID_SPI1_CS_MASTER    = 0x1,    /*!< P14 */
    PID_SPI1_CS_SLAVE     = 0x2,

    PID_SPI1_MISO_MASTER  = 0x1,    /*!< P15 */
    PID_SPI1_MISO_SLAVE   = 0x2,

    PID_SPI1_MOSI_MASTER  = 0x1,    /*!< P16 */
    PID_SPI1_MOSI_SLAVE   = 0x2,

    PID_IIC1_SCL          = 0x1,    /*!< P17 */
    PID_PLLI_LD           = 0x2,

    PID_IIC1_SDA          = 0x1,    /*!< P18 */
    PID_PLLI_D1T          = 0x2,

    PID_LOAD_N            = 0x1,    /*!< P19 */
    PID_PLLI_D2T          = 0x2,

    PID_UART2_TX          = 0x1,    /*!< P20 */
    PID_PLLN_LD           = 0x2,

    PID_UART2_RX          = 0x1,    /*!< P21 */
    PID_EXT_LDO_LOW       = 0x2,

    PID_PLLN_D2T          = 0x2,    /*!< P22 */
    PID_EXT_LDO_HIGH      = 0x3,

    PID_TEST_CLK_DSM      = 0x2,    /*!< P23 */

    PID_TEST_CLK_FMCW     = 0x2,    /*!< P24 */

    PID_DSM_CH_TOG        = 0x2,    /*!< P25 */

    PID_DSM_STEP_TOG      = 0x2,    /*!< P26 */

    PID_XO50M_EN_HIGH     = 0x1,    /*!< P27 */
    PID_WIDEN_BW          = 0x2,

    PID_XO50M_ENB_LOW     = 0x1,    /*!< P28 */

    PID_EXT_LDO_ENB_HIGH  = 0x1,    /*!< P29 */

    PID_EXT_LDO_ENB_LOW   = 0x1,    /*!< P30 */
    PID_ADC_TEST_OUT      = 0x4,
    PID_CLK_32K           = 0x5,

    PID_CAN0_TX           = 0x3,    /*!< P00/P07/P10/P12/P14/P16/P18/P23 */
    PID_CAN0_RX           = 0x3    /*!< P01/P08/P11/P13/P15/P17/P19/P24 */
} gpioPidTypedef;
/**
  * @}
  */


/* Private functions --------------------------------------------------------*/
/** @addtogroup GPIO_Private_Functions
  * @{
  */
uint32 getGpioInterruptState(void);

void clearGpioInterrupt(uint32 gpioPin);

void setGpioInterruptEnable(uint32 gpioPin, stateTypedef enable);
uint32 getGpioInterruptEnable(void);

void setGpioInterruptTrigger(uint32 gpioPin, triggerTypedef trigger);
uint32 getGpioIntEdgeNlevel(void);

uint32 getGpioIntPol(void);

void setGpioWakePol(uint32 gpioPin, gpioWakeUpTypedef wakeUpPol);
uint32 getGpioWakePol(void);

void setGpioWakeEnable(uint32 gpioPin, stateTypedef enable);
uint32 getGpioWakeEnable(void);

void clearGpioOutput(uint32 gpioPin);

uint32 getGpioOutputState(void);

void setGpioConfigPid(gpioConfigPinTypedef gpioCfgPin, gpioPidTypedef pid);
uint32 getGpioConfigPid(gpioConfigPinTypedef gpioCfgPin);

void setGpioConfigMode(gpioConfigPinTypedef gpioCfgPin, gpioModeTypedef mode);
gpioModeTypedef getGpioConfigMode(gpioConfigPinTypedef gpioCfgPin);

void setGpioConfigPull(gpioConfigPinTypedef gpioCfgPin, gpioPullTypedef gpioPull);
gpioPullTypedef getGpioConfigPull(gpioConfigPinTypedef gpioCfgPin);

void setGpioConfigSr(gpioConfigPinTypedef gpioCfgPin, srTypedef sr);
srTypedef getGpioConfigSr(gpioConfigPinTypedef gpioCfgPin);

void setGpioConfigDrive(gpioConfigPinTypedef gpioCfgPin, gpioDriveTypedef drive);
gpioDriveTypedef getGpioConfigDrive(gpioConfigPinTypedef gpioCfgPin);

void setGpioConfigSmt(gpioConfigPinTypedef gpioCfgPin, smtTypedef smt);
smtTypedef getGpioConfigSmt(gpioConfigPinTypedef gpioCfgPin);

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


