/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    spi_bsp.h
 * @original_author  DingZhen(zding@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file provides a SPI universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __SPI_BSP_H
#define __SPI_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_spi.h"
#include "gpio_bsp.h"

/** @addtogroup BSP
 *  @{
 */

/** @addtogroup SPI_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/** @addtogroup SPI_Exported_Macros
  * @{
  */
typedef struct
{
    spiGaterModeTypedef     SpiGaterMode;    // SPI communication mode (CPOL and CPHA)
    spiNSSTypedef           SpiNSS;          // SPI CS mode (Soft or Hard)
    spiModeTypedef          SpiMode;         // SPI mode (Master/Slave)
    spiDataSizeTypedef      SpiDataSize;     // SPI data size
    spiPrescalerTypedef     SpiPrescaler;    // SPI clock prescaler
    uint16                  intEn;           // SPI interrupt enable. */
    SPI_HANDLER             intHandler;      // SPI interrupt handler.
} SpiInitStrTypedef;
/** @} */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup SPI_Exported_Functions
  * @{
  */
void spiInit(SPI_TypeDef *SPIx, SpiInitStrTypedef *spiInitStr);

void spiGpioInit(SPI_TypeDef *SPIx, SpiInitStrTypedef *spiInitStr);

void spiCsInit(SPI_TypeDef *SpiX, SpiInitStrTypedef *spiInitStr);

void spiCsSelect(SPI_TypeDef *SpiX);

void spiCsDeselect(SPI_TypeDef *SpiX);

void waitBsy(SPI_TypeDef *SpiX);

void flashPd(SPI_TypeDef *SpiX);

void flashReleasePd(SPI_TypeDef *SpiX);

/** @} */
/** @} */
/** @} */

#endif
