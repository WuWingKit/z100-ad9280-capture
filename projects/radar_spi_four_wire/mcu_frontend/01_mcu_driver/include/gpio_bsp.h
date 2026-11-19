/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    gpio_bsp.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file provides a GPIO universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __GPIO_BSP_H
#define __GPIO_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_gpio.h"


/** @addtogroup BSP
 *  @{
 */

/** @addtogroup GPIO_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/** @addtogroup GPIO_Exported_Macros
  * @{
  */
#define GPIO_PIN_0                 ((uint32)0x00000001)  /* Pin 0 selected    */
#define GPIO_PIN_1                 ((uint32)0x00000002)  /* Pin 1 selected    */
#define GPIO_PIN_2                 ((uint32)0x00000004)  /* Pin 2 selected    */
#define GPIO_PIN_3                 ((uint32)0x00000008)  /* Pin 3 selected    */
#define GPIO_PIN_4                 ((uint32)0x00000010)  /* Pin 4 selected    */
#define GPIO_PIN_5                 ((uint32)0x00000020)  /* Pin 5 selected    */
#define GPIO_PIN_6                 ((uint32)0x00000040)  /* Pin 6 selected    */
#define GPIO_PIN_7                 ((uint32)0x00000080)  /* Pin 7 selected    */
#define GPIO_PIN_8                 ((uint32)0x00000100)  /* Pin 8 selected    */
#define GPIO_PIN_9                 ((uint32)0x00000200)  /* Pin 9 selected    */
#define GPIO_PIN_10                ((uint32)0x00000400)  /* Pin 10 selected   */
#define GPIO_PIN_11                ((uint32)0x00000800)  /* Pin 11 selected   */
#define GPIO_PIN_12                ((uint32)0x00001000)  /* Pin 12 selected   */
#define GPIO_PIN_13                ((uint32)0x00002000)  /* Pin 13 selected   */
#define GPIO_PIN_14                ((uint32)0x00004000)  /* Pin 14 selected   */
#define GPIO_PIN_15                ((uint32)0x00008000)  /* Pin 15 selected   */
#define GPIO_PIN_16                ((uint32)0x00010000)  /* Pin 16 selected   */
#define GPIO_PIN_17                ((uint32)0x00020000)  /* Pin 17 selected   */
#define GPIO_PIN_18                ((uint32)0x00040000)  /* Pin 18 selected   */
#define GPIO_PIN_19                ((uint32)0x00080000)  /* Pin 19 selected   */
#define GPIO_PIN_20                ((uint32)0x00100000)  /* Pin 20 selected   */
#define GPIO_PIN_21                ((uint32)0x00200000)  /* Pin 21 selected   */
#define GPIO_PIN_22                ((uint32)0x00400000)  /* Pin 22 selected   */
#define GPIO_PIN_23                ((uint32)0x00800000)  /* Pin 23 selected   */
#define GPIO_PIN_24                ((uint32)0x01000000)  /* Pin 24 selected   */
#define GPIO_PIN_25                ((uint32)0x02000000)  /* Pin 25 selected   */
#define GPIO_PIN_26                ((uint32)0x04000000)  /* Pin 26 selected   */
#define GPIO_PIN_27                ((uint32)0x08000000)  /* Pin 27 selected   */
#define GPIO_PIN_28                ((uint32)0x10000000)  /* Pin 28 selected   */
#define GPIO_PIN_29                ((uint32)0x20000000)  /* Pin 29 selected   */
#define GPIO_PIN_30                ((uint32)0x40000000)  /* Pin 30 selected   */
#define GPIO_PIN_All               ((uint32)0x7FFFFFFF)  /* All pins selected */

#define GPIO_PIN_MASK              0x7FFFFFFFu /* PIN mask for assert test */

/**
  * @brief GPIO Init structure definition
  */
typedef struct
{
    uint32 Pin;                /*!< Specifies the GPIO pins to be configured.
                           This parameter can be any value of @ref GPIO_pins_define */

    gpioModeTypedef Mode;      /*!< Specifies the operating mode for the selected pins.
                           This parameter can be a value of @ref GPIO_mode_define */

    gpioPullTypedef Pull;      /*!< Specifies the Pull-up or Pull-Down activation for the selected pins.
                           This parameter can be a value of @ref GPIO_pull_define */
} gpioInitStrTypedef;
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup GPIO_Exported_Functions
  * @{
  */
void gpioInit(gpioInitStrTypedef *gpioInitStr);

gpioPinStateTypedef gpioReadPin(uint32 gpioPin);

void gpioWritePin(uint32 gpioPin, gpioPinStateTypedef pinState);

void gpioDeInit(uint32 gpioPin);

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
