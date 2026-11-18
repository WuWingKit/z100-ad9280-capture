/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_system.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of system HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_SYSTEM__H
#define __ADT6101P_SYSTEM__H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup SYSTEM
 *  @{
 */


/** @addtogroup SYSTEM_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
/**
  * @brief  Interrupt number
  */
typedef enum
{
    INT_UART0 = 0,
    INT_UART1 = 1,
    INT_UART2 = 2,
    INT_TIMER0 = 3,
    INT_TIMER1 = 4,
    INT_DUALTIMER = 5,
    INT_UARTOVERFLOW = 6,
    INT_PMU = 7,
    INT_SPI0 = 8,
    INT_SPI1 = 9,
    INT_RTC = 10,
    INT_I2C0 = 11,
    INT_I2C1 = 12,
    INT_FFT = 13,
    INT_GPIO = 15,
    INT_DMAA = 16,
    INT_DMAB = 17,
    INT_TRNG = 18,
    INT_CAN = 19,
    INT_TIMER2 = 20,
    INT_FMCW = 21
} interruptNumTypedef;

/**
  * @brief  Interrupt state
  */
typedef enum
{
    DISABLEINT = 0, /*!< enable interrupt */
    ENABLEINT = 1   /*!< disable interrupt */
} interruptStateTypedef;


/**
  * @brief  Select clk_high source.
  */
typedef enum
{
    CLK_PLLI_50M = 0x0,
    CLK_PLLI_CKDIG,
    CLK_IPAD_ADC_DCKIN  /*!< (iopad_p05, only for test) */
} clkHighSrcTypedef;

/**
  * @brief  Select clk_high source.
  */
typedef enum
{
    HCLK_DIV1 = 0x0, /*!< 4'h0 : pclk = hclk */
    HCLK_DIV2,     /*!< 4'h1 : pclk = hclk/2 */
    HCLK_DIV4,     /*!< 4'h2 : pclk = hclk/4 */
    HCLK_DIV8,     /*!< 4'h3 : pclk = hclk/8 */
    HCLK_DIV16     /*!< 4'h4 : pclk = hclk/16 */  
} pclkDivSrcTypedef;

/**
  * @brief  System High Clock Frequency.
  */
typedef enum
{
    SYS_HIGH_CLK_50M   = 50U,
    SYS_HIGH_CLK_100M  = 100U
} sysHighClkTypedef;
/**
  * @}
  */




/* Private functions ---------------------------------------------------------*/
/** @addtogroup SYSTEM_Private_Functions
  * @{
  */
void setIntHandlerEn(interruptNumTypedef interruptNum, interruptStateTypedef enableOrDisenable);

void setNvicPriority(int32 interruptNum, uint32 PreemptPriority, uint32 SubPriority);

void setNvicPriorityGroup(uint32 PriorityGroup);

void MY_NVIC_SetVectorTable(uint32 NVIC_VectTab, uint32 Offset);

void system_reset(void);

void setHighClkSrc(const clkHighSrcTypedef sel);
uint8 getHighClkSrc(void);

void setPclkDiv(const pclkDivSrcTypedef sel);

float getRangeRes(uint32 fm, uint32 t0, uint16 downSample, uint16 fftPoint);

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
