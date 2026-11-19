/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    dma_bsp.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-21
 * @brief   This file provides a DMA universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __DMA_BSP_H
#define __DMA_BSP_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_dma.h"


/** @addtogroup BSP
 *  @{
 */

/** @addtogroup DMA_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/** @addtogroup DMA_Exported_Macros
  * @{
  */
/**
  * @brief  DMA configuration structure
  */
typedef struct
{
    dmaSelTypedef dmaId;        /*!< Select the DMA channel */
    uint32 srcAddr;             /*!< Set the DMA source address  */
    uint32 dstAddr;             /*!< Set the DMA destination address */
    uint16 srcStep;             /*!< Set the DMA source step */
    uint16 dstStep;             /*!< Set the DMA destination step */
    uint16 transLen;            /*!< Set the length of DMA transmission */
    dmaPeriTypedef periSelect;  /*!< Select the peripherals that can support DMA */
    dmaTransferModeTypedef memTomemOrperi;  /*!< Select the transmission mode of DMA */
} dmaConfigStrTypedef;
/**
  * @}
  */


/* Exported functions --------------------------------------------------------*/
/** @addtogroup DMA_Exported_Functions
  * @{
  */
void dmaConfig(dmaConfigStrTypedef *dmaConfigStr);
void dmaIntConfig(dmaSelTypedef dmaId, dmaInterruptTypedef dmaIntType, stateTypedef dmaIntStatus);
uint8 dmaWaitForTransfer(dmaSelTypedef dmaId, uint16 transLen, uint32 timeoutLoops);

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
