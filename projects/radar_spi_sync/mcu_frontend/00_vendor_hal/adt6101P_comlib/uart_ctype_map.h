/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************
 * @file    uart_ctype_map.h
 * @original_author  WuHao(hwu@andartechs.com.cn)
 * @version 1.0.0
 * @date    2020-11-24
 * @brief   uart configuration registers address definition
 *
 *                 Copyright (c) 2020, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __UART_CTYPE_MAP_H
#define __UART_CTYPE_MAP_H
#include "adt6101P_type_define.h"
#include "stdio.h"

/* State */
#define uart_tx_buff_full_shift     (0)
#define uart_tx_buff_full_bit       (1 << 0) /* TX buffer full, read-only. */
#define uart_rx_buff_full_shift     (1)
#define uart_rx_buff_full_bit       (1 << 1) /* RX buffer full, read-only. */
#define uart_tx_buff_overrun_shift  (2)
#define uart_tx_buff_overrun_bit    (1 << 2) /* TX buffer overrun, write 1 to clear. */
#define uart_rx_buff_overrun_shift  (3)
#define uart_rx_buff_overrun_bit    (1 << 3) /* RX buffer overrun, write 1 to clear. */

/* CTRL */
#define uart_tx_enable_shift        (0)
#define uart_tx_enable_bit          (1 << 0) /* TX enable. */
#define uart_rx_enable_shift        (1)
#define uart_rx_enable_bit          (1 << 1) /* RX enable. */
#define uart_tx_int_en_shift        (2)
#define uart_tx_int_en_bit          (1 << 2) /* TX interrupt enable. */
#define uart_rx_int_en_shift        (3)
#define uart_rx_int_en_bit          (1 << 3) /* RX interrupt enable. */
#define uart_tx_or_in_en_shift      (4)
#define uart_tx_or_in_en_bit        (1 << 4) /* TX overrun interrupt enable. */
#define uart_rx_or_in_en_shift      (5)
#define uart_rx_or_in_en_bit        (1 << 5) /* RX overrun interrupt enable. */
#define uart_high_speed_shift       (6)
#define uart_high_speed_bit         (1 << 6) /* High-speed test mode for TX only. */

/* INTSTATUS INTCLEAR */
#define uart_clear_tx_int_shift     (0)
#define uart_clear_tx_int_bit       (1 << 0) /* TX interrupt. Write 1 to clear. */
#define uart_clear_rx_int_shift     (1)
#define uart_clear_rx_int_bit       (1 << 1) /* RX interrupt. Write 1 to clear. */
#define uart_clear_tx_or_int_shift  (2)
#define uart_clear_tx_or_int_bit    (1 << 2) /* TX overrun interrupt. Write 1 to clear */
#define uart_clear_rx_or_int_shift  (3)
#define uart_clear_rx_or_int_bit    (1 << 3) /* RX overrun interrupt. Write 1 to clear. */


#define IS_UART_ALL_PERIPH(PERIPH) (((PERIPH) == UART0) || \
                                    ((PERIPH) == UART1) || \
                                    ((PERIPH) == UART2))

#ifdef  USE_FULL_ASSERT

    /**
    * @brief  The assert_param macro is used for function's parameters check.
    * @param  expr: If expr is false, it calls assert_failed function which reports
    *         the name of the source file and the source line number of the call
    *         that failed. If expr is true, it returns no value.
    * @retval None
    */
    #define assert_param(expr) ((expr) ? (void)0 : assert_failed((uint8 *)__FILE__, __LINE__))

    /* Exported functions ------------------------------------------------------- */
    /**
    * @brief   assert_failed
    * @return  None.
    * @note    this is aoutput function when a parameter check
    fines an error.
    */
    void assert_failed(uint8 *file, uint32 line);

#else

    #define assert_param(expr) ((void)0)

#endif /* USE_FULL_ASSERT */

typedef struct
{
    __IO uint32	DATA;
    __IO uint32	STATE;
    __IO uint32	CTRL;
    __IO uint32 INTSTATUS;
    __IO uint32	BAUDDIV;

} UartTypeDef;

#endif
