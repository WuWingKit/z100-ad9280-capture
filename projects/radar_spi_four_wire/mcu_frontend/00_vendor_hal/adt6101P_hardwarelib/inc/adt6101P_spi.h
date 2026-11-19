/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_spi.h
 * @original_author  DingZhen(zding@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of SPI HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_SPI_H
#define __ADT6101P_SPI_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "spi_ctype_map.h"
#include <stdio.h>


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup SPI
 *  @{
 */


/** @addtogroup SPI_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
#define SPI_INTSTATUS_CLEAR      (uint16)(spiicr_roric_bit | spiicr_rtic_bit)

#define SPI_INTSTATUS_ROR        ((uint16)spimis_rormis_bit)       /*!< SPI_INTSTATUS_ROR : Receive overrun interrup */
#define SPI_INTSTATUS_RT         ((uint16)spimis_rtmis_bit)        /*!< SPI_INTSTATUS_RT : Receive timeout interrupt */
#define SPI_INTSTATUS_RX         ((uint16)spimis_rxmis_bit)        /*!< SPI_INTSTATUS_RX : Receive FIFO interrupt */
#define SPI_INTSTATUS_TX         ((uint16)spimis_txmis_bit)        /*!< SPI_INTSTATUS_TX : Transmit FIFO interrupt */

/* Private typedef -----------------------------------------------------------*/
typedef void (*SPI_HANDLER)(const uint8 mode);

/**
  * @brief  Configuration Mode enumeration
  */
typedef enum
{
    SPI_Master  = 0x0,    /*!< SPI is Master mode */
    SPI_slave   = 0x1     /*!< SPI is slave mode */
} spiModeTypedef;

/**
  * @brief  Configuration Gater Mode enumeration
  */
typedef enum
{
    SPI_CPOL_High_CPHA_1Edge  = (uint8)0x2,    /*!< CPOL_High:The idle state of clock is high leve */
    SPI_CPOL_High_CPHA_2Edge  = (uint8)0x3,    /*!< CPOL_Low: The idle state of clock is loew leve */
    SPI_CPOL_Low_CPHA_1Edge   = (uint8)0x4,    /*!< CPHA_1Edge:The first jump edge is sampled */
    SPI_CPOL_Low_CPHA_2Edge   = (uint8)0x5,    /*!< CPHA_1Edge:The second jump edge is sampled */
    ERROR_MODE                = (uint8)0x6
} spiGaterModeTypedef;

/**
  * @brief SPI NSS mode selection
  */
typedef enum
{
    SPI_NSS_Soft  = (uint8)0x0,    /*!<Using GPIO for pull-up and pull-down>*/
    SPI_NSS_Hard  = (uint8)0x1     /*!<GPIO is multiplexed as cs , the hardware automatically pull-up and pull-down>*/
} spiNSSTypedef;

/**
  * @brief  Configuration Date Size enumeration
  */
typedef enum
{
    SPI_DATASIZE_4BIT   = (uint8)0x3, /*!<  4-bit data size: SPICR0[3:0] = 0011 */
    SPI_DATASIZE_5BIT   = (uint8)0x4, /*!<  5-bit data size: SPICR0[3:0] = 0100 */
    SPI_DATASIZE_6BIT   = (uint8)0x5, /*!<  6-bit data size: SPICR0[3:0] = 0101 */
    SPI_DATASIZE_7BIT   = (uint8)0x6, /*!<  7-bit data size: SPICR0[3:0] = 0110 */
    SPI_DATASIZE_8BIT   = (uint8)0x7, /*!<  8-bit data size: SPICR0[3:0] = 0111 */
    SPI_DATASIZE_9BIT   = (uint8)0x8, /*!<  9-bit data size: SPICR0[3:0] = 1000 */
    SPI_DATASIZE_10BIT  = (uint8)0x9, /*!<  10-bit data size: SPICR0[3:0] = 1001 */
    SPI_DATASIZE_11BIT  = (uint8)0xA, /*!<  11-bit data size: SPICR0[3:0] = 1010 */
    SPI_DATASIZE_12BIT  = (uint8)0xB, /*!<  12-bit data size: SPICR0[3:0] = 1011 */
    SPI_DATASIZE_13BIT  = (uint8)0xC, /*!<  13-bit data size: SPICR0[3:0] = 1100 */
    SPI_DATASIZE_14BIT  = (uint8)0xD, /*!<  14-bit data size: SPICR0[3:0] = 1101 */
    SPI_DATASIZE_15BIT  = (uint8)0xE, /*!<  15-bit data size: SPICR0[3:0] = 1110 */
    SPI_DATASIZE_16BIT  = (uint8)0xF  /*!<  16-bit data size: SPICR0[3:0] = 1111 */
} spiDataSizeTypedef;

/**
  * @brief  Configuration Frequency division coefficient enumeration
  */
typedef enum
{
    SPI_Prescaler_2     = (uint16)2,
    SPI_Prescaler_4     = (uint16)4,
    SPI_Prescaler_8     = (uint16)8,
    SPI_Prescaler_16    = (uint16)16,
    SPI_Prescaler_32    = (uint16)32,
    SPI_Prescaler_64    = (uint16)64,
    SPI_Prescaler_128   = (uint16)128,
    SPI_Prescaler_254   = (uint16)254
} spiPrescalerTypedef;

/**
  * @brief  Configuration Mode enumeration
  */
typedef enum
{
    SPI_INTERRUPT_ROR  = (uint16)0x0,  /*!<  Receive overrun interrup */
    SPI_INTERRUPT_RT   = (uint16)0x1,  /*!<  Receive timeout interrupt */
    SPI_INTERRUPT_RX   = (uint16)0x2,  /*!<  Receive FIFO interrupt */
    SPI_INTERRUPT_TX   = (uint16)0x3,  /*!<  Transmit FIFO interrupt */
} spiInterruptTypedef;
/** @} */

/* Private functions ---------------------------------------------------------*/
/** @addtogroup SPI_Private_Functions
  * @{
  */
uint16 getSpiSCR(SPI_TypeDef *SpiX);

void setSpiSCR(SPI_TypeDef *SpiX, uint16 scr);

void setSpiGaterMode(SPI_TypeDef *SpiX, spiGaterModeTypedef spiGaterMode);

spiGaterModeTypedef getSpiGaterMode(SPI_TypeDef *SpiX);

void spiWrite(SPI_TypeDef *SpiX, uint8 txData);

uint8 spiRead(SPI_TypeDef *SpiX);

void spiWriteBurst(SPI_TypeDef *SpiX, uint8 *pbuf, uint16 len);

void spiReadBurst(SPI_TypeDef *SpiX, uint8 *pbuf, uint16 len);

uint8 spiReadWriteByte(SPI_TypeDef *SpiX, uint8 TxData);

uint8 getSpiFrameFormat(SPI_TypeDef *SpiX);

void setSpiFrameFormat(SPI_TypeDef *SpiX, uint8 format);

uint8 getSpiDataSize(SPI_TypeDef *SpiX);

void setSpiDataSize(SPI_TypeDef *SpiX, spiDataSizeTypedef size);

void setSpiSpiLoopBackMode(SPI_TypeDef *SpiX, uint8 lbm);

uint8 getSpiSpiLoopBackMode(SPI_TypeDef *SpiX);

void setSpiSsp(SPI_TypeDef *SpiX, uint8 ssp);

uint8 getSpiSs(SPI_TypeDef *SpiX);

void setSpiMode(SPI_TypeDef *SpiX, spiModeTypedef mode);

uint8 getSpiSpiMode(SPI_TypeDef *SpiX);

void setSpiSod(SPI_TypeDef *SpiX, uint8 sod);

uint8 getSpiSod(SPI_TypeDef *SpiX);

void spiWriteData(SPI_TypeDef *SpiX, uint8 data);

uint8 spiReadData(SPI_TypeDef *SpiX);

uint8 isTransmitFifoEmpty(SPI_TypeDef *SpiX);

uint8 isTransmitFifoNotFull(SPI_TypeDef *SpiX);

uint8 isReceiveFifoNotEmpty(SPI_TypeDef *SpiX);

uint8 isReceiveFifoFull(SPI_TypeDef *SpiX);

uint8 isSpiBusy(SPI_TypeDef *SpiX);

void setSpiClockPrescaler(SPI_TypeDef *SpiX, uint16 prescaler);

uint16 getSpiClockPrescaler(SPI_TypeDef *SpiX);

void enableSpiInterrupt(SPI_TypeDef *SpiX, uint8 tmpreg);

void disableSpiInterrupt(SPI_TypeDef *SpiX);

uint16 getSpiRawInterruptState(SPI_TypeDef *SpiX);

uint16 getSpiMaskedInterruptState(SPI_TypeDef *SpiX);

void setSpiMaskedInterruptState(SPI_TypeDef *SpiX);

void clearSpiInterrupt(SPI_TypeDef *SpiX);

void setSpiReceiveDmaEnable(SPI_TypeDef *SpiX, uint8 enable);

uint8 getSpiReceiveDmaEnable(SPI_TypeDef *SpiX);

void setSpiTransmitDmaEnable(SPI_TypeDef *SpiX, uint8 enable);

uint8 getSpiTransmitDmaEnable(SPI_TypeDef *SpiX);

void setSpiIntHandler(SPI_TypeDef *SpiX, const SPI_HANDLER handler);

void spi0Handler(void);

void spi1Handler(void);

/** @} */
/** @} */
/** @} */

#endif
