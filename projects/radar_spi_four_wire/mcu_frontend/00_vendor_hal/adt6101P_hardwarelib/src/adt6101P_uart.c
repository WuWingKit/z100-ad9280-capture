/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_uart.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file contains all the functions implemented by the UART peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_uart.h"


/* Private variables ---------------------------------------------------------*/
static UART_HANDLER s_uartHandler[3] = {NULL, NULL, NULL};


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   UART0 interrupt handler.
 */
void uart0Handler(void)
{
    uint8 state;

    for(uint8 i = 0; i < 4; ++i)
    {
        state = (1 << i);
        if(state == (getUartIntStatus(UART0) & state))
        {
            setUartIntStatus(UART0, state);
            s_uartHandler[0](state);
        }
    }
}


/**
 * @brief   UART1 interrupt handler.
 */
void uart1Handler(void)
{
    uint8 state;

    for(uint8 i = 0; i < 4; ++i)
    {
        state = (1 << i);
        if(state == (getUartIntStatus(UART1) & state))
        {
            setUartIntStatus(UART1, state);
            s_uartHandler[1](state);
        }
    }
}


/**
 * @brief   UART2 interrupt handler.
 */
void uart2Handler(void)
{
    uint8 state;

    for(uint8 i = 0; i < 4; ++i)
    {
        state = (1 << i);
        if(state == (getUartIntStatus(UART2) & state))
        {
            setUartIntStatus(UART2, state);
            s_uartHandler[2](state);
        }
    }
}


/**
 * @brief   Set the uart interrupt handler function.
 * @param   uartX: Uart number:UART0,UART1,UART2.
 * @param   handler: the address of the uart interrupt callback function.
 * @return  None.
 */
void setUartIntHandler(UartTypeDef *uartX, const UART_HANDLER handler)
{
    if(NULL != handler)
    {
        if(UART0 == uartX)
        {
            s_uartHandler[0] = handler;
        }
        else if(UART1 == uartX)
        {
            s_uartHandler[1] = handler;
        }
        else if(UART2 == uartX)
        {
            s_uartHandler[2] = handler;
        }
    }
}


/**
 * @brief   set Uart Data.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @param   tmpreg: value of the register.
 * @return  None.
 */
void setUartData(UartTypeDef *uartX, uint8 data)
{
    WRITE_REG(uartX->DATA, data);
}

/**
 * @brief   get Uart Data.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @return  uart data.
 */
uint8 getUartData(UartTypeDef *uartX)
{
    return uartX->DATA;
}

/**
 * @brief   set Uart Data.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @param   tmpreg: value of the register.
 * @return  None.
 */
void setUartState(UartTypeDef *uartX, uint8 tmpreg)
{
    MODIFY_REG(uartX->STATE,                                                      \
               (uint32)(UART_STATE_TX_BUFF_OR_FULL | UART_STATE_RX_BUFF_OR_FULL), \
               tmpreg);
}

/**
 * @brief   get Uart State.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @return  uart state.
 */
uint8 getUartState(UartTypeDef *uartX)
{
    return uartX->STATE;
}

/**
 * @brief   set Uart Ctrl.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @param   tmpreg: value of the register.
 * @return  None.
 */
void setUartCtrl(UartTypeDef *uartX, uint8 tmpreg)
{
    MODIFY_REG(uartX->CTRL,                                                       \
               (uint32)(UART_MODE_TX | UART_MODE_RX | UART_INT_TX | UART_INT_RX | \
                        UART_OR_INT_TX | UART_OR_INT_RX | UART_HIGH_SPEED_TX),    \
               tmpreg);
}

/**
 * @brief   clear Uart Ctrl.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @return  None.
 */
void clearUartCtrl(UartTypeDef *uartX)
{
    uartX->CTRL = UART_MODE_NONE;
}

/**
 * @brief   get Uart Ctrl.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @return  uart ctrl.
 */
uint8 getUartCtrl(UartTypeDef *uartX)
{
    return uartX->CTRL;
}

/**
 * @brief   set Uart Int Status.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @param   tmpreg: value of the register.
 * @return  None.
 */
void setUartIntStatus(UartTypeDef *uartX, uint8 tmpreg)
{
    MODIFY_REG(uartX->INTSTATUS,                                      \
               (uint32)(UART_INTSTATUS_TX | UART_INTSTATUS_RX |       \
                        UART_INTSTATUS_TX_OR | UART_INTSTATUS_RX_OR), \
               tmpreg);
}

/**
 * @brief   get Uart Int Status.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @return  uart int status.
 */
uint8 getUartIntStatus(UartTypeDef *uartX)
{
    return uartX->INTSTATUS;
}

/**
 * @brief   set Uart baud rate divider.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @param   tmpreg: value of the register.
 * @return  None.
 * @note    bit[19:0].The minimum number is 16.
 */
void setUartBaudDiv(UartTypeDef *uartX, uint32 tmpreg)
{
    WRITE_REG(uartX->BAUDDIV, tmpreg);
}

/**
 * @brief   get Uart baud rate divider.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @return  uart baud rate divider.
 */
uint32 getUartBaudDiv(UartTypeDef *uartX)
{
    return uartX->BAUDDIV;
}

#ifdef  USE_FULL_ASSERT

/**
 * @brief   assert_failed
 * @return  None.
 * @note    this is aoutput function when a parameter check
            fines an error.
 */
void assert_failed(uint8 *file, uint32 line)
{
    printf("Wrong parameters value: file%s oneline%d \r\n", file, line);
}

#endif

/**
 * @brief   redefinition fputc function(UART)
 * @return  int32.
 */
int32 fputc(int32 ch, FILE *f)
{
    (void)f;

    /* 当前协议版本：UART0为115200指令/调试口，UART1为921600纯数据上报口。
     * printf只允许走UART0，避免ASCII调试信息混入UART1数据帧。
     */
    if((UART0->CTRL & UART_MODE_TX) == UART_MODE_TX)
    {
        while((UART0->STATE & UART_STATE_TX_BUFF_FULL) == UART_STATE_TX_BUFF_FULL);
        UART0->DATA = (uint8) ch;
    }

    return ch;
}

