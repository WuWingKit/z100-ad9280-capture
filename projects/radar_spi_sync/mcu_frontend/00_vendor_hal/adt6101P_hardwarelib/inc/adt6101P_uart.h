/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_uart.h
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of UART HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_UART_H
#define __ADT6101P_UART_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "uart_ctype_map.h"



/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup UART
 *  @{
 */


/** @addtogroup UART_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
#define UART_MODE_NONE               ((uint16)0U)

#define UART_MODE_TX                 ((uint16)uart_tx_enable_bit)       /*!< TX enable. */
#define UART_MODE_RX                 ((uint16)uart_rx_enable_bit)       /*!< RX enable. */


#define UART_INT_NONE                ((uint16)0U)
#define UART_INT_TX                  ((uint16)uart_tx_int_en_bit)       /*!< TX interrupt enable. */
#define UART_INT_RX                  ((uint16)uart_rx_int_en_bit)       /*!< RX interrupt enable. */
#define UART_OR_INT_TX               ((uint16)uart_tx_or_in_en_bit)     /*!< TX overrun interrupt enable. */
#define UART_OR_INT_RX               ((uint16)uart_rx_or_in_en_bit)     /*!< RX overrun interrupt enable. */


#define UART_HIGH_SPEED_TX           ((uint32)uart_high_speed_bit)      /*!< High-speed test mode for TX only. */

#define UART_STATE_CLEAR             ((uint16)(uart_tx_buff_overrun_bit | uart_rx_buff_overrun_bit))
#define UART_STATE_TX_BUFF_FULL      ((uint16)uart_tx_buff_full_bit)    /*!< TX buffer full, read-only. */
#define UART_STATE_RX_BUFF_FULL      ((uint16)uart_rx_buff_full_bit)    /*!< RX buffer full, read-only. */
#define UART_STATE_TX_BUFF_OR_FULL   ((uint16)uart_tx_buff_overrun_bit) /*!< TX buffer overrun, write 1 to clear. */
#define UART_STATE_RX_BUFF_OR_FULL   ((uint16)uart_rx_buff_overrun_bit) /*!< RX buffer overrun, write 1 to clear. */


#define UART_INTSTATUS_CLEAR         ((uint16)(uart_clear_tx_int_bit | uart_clear_rx_int_bit | \
                                        uart_clear_tx_or_int_bit | uart_clear_rx_or_int_bit))
#define UART_INTSTATUS_TX            ((uint16)uart_clear_tx_int_bit)    /*!< TX interrupt. Write 1 to clear. */
#define UART_INTSTATUS_RX            ((uint16)uart_clear_rx_int_bit)    /*!< RX interrupt. Write 1 to clear. */
#define UART_INTSTATUS_TX_OR         ((uint16)uart_clear_tx_or_int_bit) /*!< TX overrun interrupt. Write 1 to clear */
#define UART_INTSTATUS_RX_OR         ((uint16)uart_clear_rx_or_int_bit) /*!< RX overrun interrupt. Write 1 to clear. */


typedef void (*UART_HANDLER)(const uint16 mode);

/**
  * @}
  */


/* Private functions ---------------------------------------------------------*/
/** @addtogroup UART_Private_Functions
  * @{
  */
void setUartIntHandler(UartTypeDef *uartX, const UART_HANDLER handler);

void setUartData(UartTypeDef *uartX, uint8 data);
uint8 getUartData(UartTypeDef *uartX);

void setUartState(UartTypeDef *uartX, uint8 tmpreg);
uint8 getUartState(UartTypeDef *uartX);

void setUartCtrl(UartTypeDef *uartX, uint8 tmpreg);
void clearUartCtrl(UartTypeDef *uartX);
uint8 getUartCtrl(UartTypeDef *uartX);

void setUartIntStatus(UartTypeDef *uartX, uint8 tmpreg);
uint8 getUartIntStatus(UartTypeDef *uartX);

void setUartBaudDiv(UartTypeDef *uartX, uint32 tmpreg);
uint32 getUartBaudDiv(UartTypeDef *uartX);

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
