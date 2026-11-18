/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    dma_bsp.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-21
 * @brief   This file references the DMA HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "dma_bsp.h"


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   dma0 setting.
 *
 * Example:
 * @code
 * dmaConfigStrTypedef dmaConfigStr;
 * dmaConfigStr.dmaId = DMA1;
 * dmaConfigStr.srcAddr = 0x20008000;
 * dmaConfigStr.dstAddr = 0x20010000;
 * dmaConfigStr.srcStep = 4;
 * dmaConfigStr.dstStep = 4;
 * dmaConfigStr.transLen = 128;
 * dmaConfigStr.periSelect = PERI_SELECT_SPI0TX;
 * dmaConfigStr.memTomemOrperi = MEMTOMEM;
 * dmaConfig(&dmaConfigStr);
 * @endcode
 * @param   dmaConfigStr: DMA configuration structure, see also @ref dmaConfigStrTypedef.
 * @return  None.
 */
void dmaConfig(dmaConfigStrTypedef *dmaConfigStr)
{
    if(dmaConfigStr == (dmaConfigStrTypedef *)0)
    {
        return;
    }

    setDmaAddr(dmaConfigStr->dmaId, dmaConfigStr->srcAddr, dmaConfigStr->dstAddr);
    setDmaStep(dmaConfigStr->dmaId, dmaConfigStr->srcStep, dmaConfigStr->dstStep);
    setDmaPriority(dmaConfigStr->dmaId, DMA_PRIORITY1);
    setDmaLen(dmaConfigStr->dmaId, dmaConfigStr->transLen);
    setDmaTransferMode(dmaConfigStr->dmaId, dmaConfigStr->memTomemOrperi);
    setDmaSize(dmaConfigStr->dmaId, 0x3, 0x3);
    setDmaPeriSel(dmaConfigStr->dmaId, dmaConfigStr->periSelect);
    setDmaEnable(dmaConfigStr->dmaId);
}

/**
 * @brief   Dma Interrupt Config.
 * @param   dmaConfigStr: DMA configuration structure, see also @ref dmaConfigStrTypedef.
 * @return  None.
 */
void dmaIntConfig(dmaSelTypedef dmaId, dmaInterruptTypedef dmaIntType, stateTypedef dmaIntStatus)
{
    if(ENABLE == dmaIntStatus)
    {
        enableDmaInterrupt(dmaId,dmaIntType);
    }
    else
    {
        disableDmaInterrupt(dmaId,dmaIntType);
    }
}

uint8 dmaWaitForTransfer(dmaSelTypedef dmaId, uint16 transLen, uint32 timeoutLoops)
{
    if(transLen == 0u)
    {
        return 1u;
    }

    if(timeoutLoops == 0u)
    {
        timeoutLoops = 1u;
    }

    while(timeoutLoops > 0u)
    {
        if(getDmaTransferCount(dmaId) >= transLen)
        {
            return 1u;
        }
        timeoutLoops--;
    }

    return (uint8)((getDmaTransferCount(dmaId) >= transLen) ? 1u : 0u);
}
