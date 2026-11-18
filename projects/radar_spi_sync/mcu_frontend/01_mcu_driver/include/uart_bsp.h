/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    uart_bsp.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file provides a UART  HAL universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __UART_BSP_H
#define __UART_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_uart.h"
#include "adt6101P_dma.h"
#include "sys_bsp.h"


/** @addtogroup BSP
 *  @{
 */

/** @addtogroup UART_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/** @addtogroup UART_Exported_Macros
  * @{
  */
/**
  * @brief UART Init Structure definition
  */
typedef struct
{
    UartTypeDef *uartX;             /*!< UART registers base address        */

    uint32      baudRate;           /*!< This member configures the UART communication baud rate. */

    uint32      mode;               /*!< Specifies whether the Receive or Transmit mode is enabled or disabled. */

    uint32      intEn;              /*!< UART interrupt enable. */
    UART_HANDLER    intHandler;     /*!< UART interrupt handler. */
} uartInitStrTypedef;

/**
  * @brief  uart status structures definition
  */
typedef enum
{
    UART_OK       = 0x00U,
    UART_ERROR    = 0x01U
} uartStatusTypedef;
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup UART_Exported_Functions
  * @{
  */
uartStatusTypedef uartInit(uartInitStrTypedef *huart);

void uartDeInit(UartTypeDef *uartX);

int32 uartWrite(UartTypeDef *uartX, const uint8 *sendBuff, int32 sendLen);

/* DMA 发送成功返回 sendLen，参数错误返回 0，传输超时返回 -1。 */
int32 uartWriteDma(UartTypeDef *uartX,
                   const uint8 *sendBuff,
                   int32 sendLen,
                   dmaSelTypedef dmaId,
                   uint32 timeoutLoops);

int32 uartRead(UartTypeDef *UartX, uint8 *recvBuff, int32 recvLen);

int16 printfDebug(const char *format, ...);

void printfUart(UartTypeDef *uartX, const char *format, ...);

uint32 getUartRxReadyFlag(void);

void  setUartRxReadyFlag(uint32 value);

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
