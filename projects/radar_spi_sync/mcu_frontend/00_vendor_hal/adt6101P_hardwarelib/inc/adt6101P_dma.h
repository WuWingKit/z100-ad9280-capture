/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_dma.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-21
 * @brief   Header file containing functions prototypes of DMA HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_DMA__H
#define __ADT6101P_DMA__H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup DMA
 *  @{
 */

/** @addtogroup DMA_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
/**
  * @brief  Select the DMA channel.
  */
typedef enum
{
    DMA0 = 0x0,
    DMA1,
    DMA2,
    DMA3,
    DMA4
} dmaSelTypedef;

/**
  * @brief  DMA can freely choose the interrupt service handler
  */
typedef enum
{
    INT_HANLDER_0 = 0x0,    /*!< select dmaAHandler service handler */
    INT_HANLDER_1           /*!< select dmaBHandler service handler */
} dmaInterruptHandlerNumTypedef;

/**
  * @brief  Dma Interrupt type.
  */
typedef enum
{
    DMA_TIME_OUT = 0x0,
    DMA_4_4_DONE,
    DMA_3_4_DONE,
    DMA_2_4_DONE,
    DMA_1_4_DONE,
} dmaInterruptTypedef;

/**
  * @brief  Select the peripherals that can support DMA.
  */
typedef enum
{
    PERI_NONE = 0x0,
    PERI_SELECT_UART0TX,
    PERI_SELECT_UART1TX,
    PERI_SELECT_UART2TX,
    PERI_SELECT_SPI0TX,
    PERI_SELECT_SPI0RX,
    PERI_SELECT_SPI1TX,
    PERI_SELECT_SPI1RX,
    PERI_SELECT_IIC0TX,
    PERI_SELECT_IIC0RX,
    PERI_SELECT_IIC1TX,
    PERI_SELECT_IIC1RX,
    PERI_SELECT_ADC0,
    PERI_SELECT_ADC1
} dmaPeriTypedef;

/**
  * @brief  Select the priority of DMA
  *         Firstly, DMA will response request from all channels with priority = 1 by round robin mode.
  *         Then, when there is no request from priority=1,
  *         DMA will process request from all channels with priority = 0 by round robin mode.
  * @note   While 1 is higher priority than 0
  */
typedef enum
{
    DMA_PRIORITY0 = 0x0,
    DMA_PRIORITY1,
} dmaPriorityTypedef;

/**
  * @brief  Select the transmission mode of DMA
  */
typedef enum
{
    PERITOMEM = 0x0,    /*!< peripheral to memory */
    MEMTOPERI,          /*!< memory to peripheral */
    MEMTOMEM,           /*!< memory to memory */
} dmaTransferModeTypedef;
/**
  * @}
  */


/* Private functions ---------------------------------------------------------*/
/** @addtogroup DMA_Private_Functions
  * @{
  */
void setDmaIntHandler(dmaSelTypedef sel, dmaInterruptHandlerNumTypedef num);
uint8 getDmaAllInterrupt(void);

void enableDmaInterrupt(dmaSelTypedef sel, dmaInterruptTypedef dmaInttype);
void disableDmaInterrupt(dmaSelTypedef sel, dmaInterruptTypedef dmaInttype);

uint8 setDmaAddr(dmaSelTypedef sel, uint32 srcAddr, uint32 dstAddr);
uint32 getDmaSrcAddr(dmaSelTypedef sel);
uint32 getDmaDstAddr(dmaSelTypedef sel);

void setDmaStep(dmaSelTypedef sel, uint16 srcStep, uint16 dstStep);
uint16 getDmaSrcStep(dmaSelTypedef sel);
uint16 getDmaDstStep(dmaSelTypedef sel);

uint8 setDmaEnable(dmaSelTypedef sel);
uint8 setDmaDisable(dmaSelTypedef sel);
uint8 getDmaStatus(dmaSelTypedef sel);

uint8 setDmaPeriSel(dmaSelTypedef sel, dmaPeriTypedef peri);
uint8 getDmaPeriSel(dmaSelTypedef sel);

uint8 setDmaPriority(dmaSelTypedef sel, dmaPriorityTypedef pri);
uint8 getDmaPriority(dmaSelTypedef sel);

uint8 setDmaLen(dmaSelTypedef sel, uint16 len);
uint16 getDmaLen(dmaSelTypedef sel);

uint8 setDmaTransferMode(dmaSelTypedef sel, dmaTransferModeTypedef mode);
uint8 getDmaTransferMode(dmaSelTypedef sel);

uint8 setDmaSize(dmaSelTypedef sel, uint8 srcSize, uint32 dstSize);
uint8 getDmaSrcSize(dmaSelTypedef sel);
uint8 getDmaDstSize(dmaSelTypedef sel);

uint16 getDmaTransferCount(dmaSelTypedef sel);

void waitingForDmaTransfer(dmaSelTypedef sel, uint16 cnt);

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
