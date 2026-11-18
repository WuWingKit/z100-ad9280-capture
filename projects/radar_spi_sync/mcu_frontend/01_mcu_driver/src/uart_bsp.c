/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    uart_bsp.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file references the UART HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "uart_bsp.h"
#include "gpio_bsp.h"
#include "adt6101P_system.h"
#include "dma_bsp.h"

#include <stdarg.h>
#include <arm_math.h>


/* Exported variables ---------------------------------------------------------*/
static volatile uint32 s_uartRecvFlag = 0;


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   uart Init
 * @param   uartInitStrTypedef: UART configuration initialization structure, see also @ref uartInitStrTypedef.
 * @return  uart status structures definition, see also @ref uartStatusTypedef.
 */
uartStatusTypedef uartInit(uartInitStrTypedef *huart)
{
    /* Check the UART handle allocation */
    if(huart == NULL)
    {
        return UART_ERROR;
    }

    /* deInitializes */
    uartDeInit(huart->uartX);

    /* set uart io pid */
    gpioInitStrTypedef  uartTxInitStruct;
    uartTxInitStruct.Mode = GPIO_MODE_HIGH_INPENDECE;
    uartTxInitStruct.Pull = GPIO_NO_PULL;

    gpioInitStrTypedef  uartRxInitStruct;
    uartRxInitStruct.Mode = GPIO_MODE_HIGH_INPENDECE;
    uartRxInitStruct.Pull = GPIO_PULL_UP;

    if(huart->uartX == UART0)
    {
        uartTxInitStruct.Pin = GPIO_PIN_0;
        gpioInit(&uartTxInitStruct);
        setGpioConfigPid(GPIO_CFG_P0, PID_UART0_TX);

        uartRxInitStruct.Pin = GPIO_PIN_1;
        gpioInit(&uartRxInitStruct);
        setGpioConfigPid(GPIO_CFG_P1, PID_UART0_RX);
    }
    else if(huart->uartX == UART1)
    {
        uartTxInitStruct.Pin = GPIO_PIN_10;
        gpioInit(&uartTxInitStruct);
        setGpioConfigPid(GPIO_CFG_P10, PID_UART1_TX);

        uartRxInitStruct.Pin = GPIO_PIN_11;
        gpioInit(&uartRxInitStruct);
        setGpioConfigPid(GPIO_CFG_P11, PID_UART1_RX);
    }
    else if(huart->uartX == UART2)
    {
        uartTxInitStruct.Pin = GPIO_PIN_20;
        gpioInit(&uartTxInitStruct);
        setGpioConfigPid(GPIO_CFG_P20, PID_UART2_TX);

        uartRxInitStruct.Pin = GPIO_PIN_21;
        gpioInit(&uartRxInitStruct);
        setGpioConfigPid(GPIO_CFG_P21, PID_UART2_RX);
    }

    /* set baud rate */
    sysHighClkTypedef sysPclk = getPclk();

    uint32 hclkDivision;

    hclkDivision = sysPclk * 1000000 / huart->baudRate;

    if(hclkDivision < 16)
    {
        return UART_ERROR;
    }
    setUartBaudDiv(huart->uartX, hclkDivision);

    /* set uart ctrl */
    uint32 modeTmp = huart->mode | huart->intEn;
    setUartCtrl(huart->uartX, modeTmp);
    setUartIntHandler(huart->uartX, huart->intHandler);
    
    if(huart->uartX == UART0)
    {
        setIntHandlerEn(INT_UART0, ENABLEINT);
    }
    else if(huart->uartX == UART1)
    {
        setIntHandlerEn(INT_UART1, ENABLEINT);
    }
    else if(huart->uartX == UART2)
    {
        setIntHandlerEn(INT_UART2, ENABLEINT);
    }

    return UART_OK;
}

/**
 * @brief   DeInitializes the UART peripheral.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @return  None.
 */
void uartDeInit(UartTypeDef *uartX)
{
   if(uartX == UART0)
   {
       setIntHandlerEn(INT_UART0, DISABLEINT);
   }
   else if(uartX == UART1)
   {
       setIntHandlerEn(INT_UART1, DISABLEINT);
   }
   else if(uartX == UART2)
   {
       setIntHandlerEn(INT_UART2, DISABLEINT);
   }
    
    clearUartCtrl(uartX);
    setUartState(uartX, UART_STATE_CLEAR);
    setUartIntStatus(uartX, UART_INTSTATUS_CLEAR);
}

/**
 * @brief   uart send specified byte data.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @param   sendBuff: uart buffered data ready to send.
 * @param   sendLen: The length of the buffer data uart is ready to send.
 * @return  len uart has sent the length of the data .
 */
int32 uartWrite(UartTypeDef *uartX, const uint8 *sendBuff, int32 sendLen)
{
    int32 sendNum;

    if((uartX == NULL) || (sendBuff == NULL) || (sendLen <= 0))
    {
        return 0;
    }

    for(sendNum = 0; sendNum < sendLen; sendNum++)
    {
        while((getUartState(uartX) & UART_STATE_TX_BUFF_FULL) == UART_STATE_TX_BUFF_FULL);
        setUartData(uartX, sendBuff[sendNum]);
    }
    return sendNum;
}

int32 uartWriteDma(UartTypeDef *uartX,
                   const uint8 *sendBuff,
                   int32 sendLen,
                   dmaSelTypedef dmaId,
                   uint32 timeoutLoops)
{
    dmaPeriTypedef peripheral;

    if((sendBuff == NULL) || (sendLen <= 0) || (sendLen > 65535))
    {
        return 0;
    }
    if(uartX == UART0)
    {
        peripheral = PERI_SELECT_UART0TX;
    }
    else if(uartX == UART1)
    {
        peripheral = PERI_SELECT_UART1TX;
    }
    else if(uartX == UART2)
    {
        peripheral = PERI_SELECT_UART2TX;
    }
    else
    {
        return 0;
    }

    (void)setDmaDisable(dmaId);
    (void)setDmaAddr(dmaId, (uint32)sendBuff, (uint32)&uartX->DATA);
    setDmaStep(dmaId, 1u, 0u);
    (void)setDmaPriority(dmaId, DMA_PRIORITY1);
    (void)setDmaLen(dmaId, (uint16)sendLen);
    (void)setDmaTransferMode(dmaId, MEMTOPERI);
    /* DMA size 编码 0 对应 8 bit；UART DATA 只使用低 8 bit。 */
    (void)setDmaSize(dmaId, 0u, 0u);
    (void)setDmaPeriSel(dmaId, peripheral);
    (void)setDmaEnable(dmaId);

    if(dmaWaitForTransfer(dmaId, (uint16)sendLen, timeoutLoops) == 0u)
    {
        (void)setDmaDisable(dmaId);
        return -1;
    }

    (void)setDmaDisable(dmaId);
    return sendLen;
}

/**
 * @brief   uart receive specified byte data.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @param   recvBuff: uart buffered data ready to receive.
 * @param   recvLen: The length of the buffer data uart is ready to receive.
 * @return  len uart has receive the length of the data .
 */
int32 uartRead(UartTypeDef *uartX, uint8 *recvBuff, int32 recvLen)
{
    int32 recvNum;

    if((uartX == NULL) || (recvBuff == NULL) || (recvLen <= 0))
    {
        return 0;
    }

    for(recvNum = 0; recvNum < recvLen; recvNum++)
    {
        s_uartRecvFlag = 0;
        while((getUartState(uartX) & UART_STATE_RX_BUFF_FULL) != UART_STATE_RX_BUFF_FULL)
        {
        }
        s_uartRecvFlag = 1;
        recvBuff[recvNum] = getUartData(uartX);
    }
    return recvNum;
}

/**
 * @brief   uart0 and uart1 and uart2 setting.
 * @param   format: printf standard format.
 * @return  None.
 */
int16 printfDebug(const char *format, ...)
{
    va_list vl;
    va_start(vl, format);
    int16 ret = vprintf(format, vl);
    va_end(vl);
    return ret;
}

/**
 * @brief   uart0 and uart1 and uart2 printf.
 * @param   UartX: Uart number:UART0,UART1,UART2.
 * @param   format: printf standard format.
 * @return  None.
 */
#define USART_SENDBUFFLEN 256
void printfUart(UartTypeDef *uartX, const char *format, ...)
{
    assert_param(IS_UART_ALL_PERIPH(uartX));

    static char uartSendBuff[USART_SENDBUFFLEN];
    uint16 sendNum = 0;

    memset(uartSendBuff, 0, USART_SENDBUFFLEN);
    va_list vl;
    va_start(vl, format);
    vsprintf(uartSendBuff, format, vl);
    va_end(vl);
    while(uartSendBuff[sendNum])
    {
        while((getUartState(uartX) & UART_STATE_TX_BUFF_FULL) == UART_STATE_TX_BUFF_FULL);
        setUartData(uartX, uartSendBuff[sendNum++]);
    }
}

/**
 * @brief   get uart rx ready flag.
 * @param   None.
 * @return  uint8. uart rx ready flag value
 */
uint32 getUartRxReadyFlag(void)
{
    return s_uartRecvFlag;
}

/**
 * @brief   set uart rx ready flag.
 * @param   uint8. uart rx ready flag value
 * @return  None.
 */
void setUartRxReadyFlag(uint32 value)
{
    s_uartRecvFlag = value;
}
