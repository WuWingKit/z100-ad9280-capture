/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_dma.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-21
 * @brief   This file contains all the functions implemented by the DMA peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_dma.h"
#include "dma_ctype_map.h"


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   Set the interrupt number 1 for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   num: INT_HANLDER_0: The interrupt number is 0; INT_HANLDER_1: The interrupt number is 1, see also @ref dmaInterruptHandlerNumTypedef.
 * @return  None.
 */
void setDmaIntHandler(dmaSelTypedef sel, dmaInterruptHandlerNumTypedef num)
{
    if(INT_HANLDER_0 == num)
    {
        CLEAR_BIT(DMA->dma_int_sel, (1 << (uint8)sel));
    }
    else if(INT_HANLDER_1 == num)
    {
        SET_BIT(DMA->dma_int_sel, (1 << (uint8)sel));
    }
}

/**
 * @brief   Get the interrupt number for DMA.
 * @param   None.
 * @return  Get the set interrupt number.
 */
uint8 getDmaAllInterrupt(void)
{
    return GET_BITS_VALUE(DMA->dma_int_sel, 0, ((1 << 5) - 1));
}

/**
 * @brief   Set the DMA interrupt.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   dmaInttype: Dma interrupt type.
 * @return  None.
 */
void enableDmaInterrupt(dmaSelTypedef sel, dmaInterruptTypedef dmaInttype)
{
    switch(sel)
    {
        case DMA0:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << (uint8)dmaInttype));
            SET_BIT(DMA->dmair_dma_int_mask, (1 << (uint8)dmaInttype));
            break;
        }
        case DMA1:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 5)));
            SET_BIT(DMA->dmair_dma_int_mask, (1 << ((uint8)dmaInttype + 5)));
            break;
        }
        case DMA2:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 10)));
            SET_BIT(DMA->dmair_dma_int_mask, (1 << ((uint8)dmaInttype + 10)));
            break;
        }
        case DMA3:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 15)));
            SET_BIT(DMA->dmair_dma_int_mask, (1 << ((uint8)dmaInttype + 15)));
            break;
        }
        case DMA4:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 20)));
            SET_BIT(DMA->dmair_dma_int_mask, (1 << ((uint8)dmaInttype + 20)));
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
 * @brief   Set the DMA interrupt.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   dmaInttype: Dma interrupt type.
 * @return  None.
 */
void disableDmaInterrupt(dmaSelTypedef sel, dmaInterruptTypedef dmaInttype)
{
    switch(sel)
    {
        case DMA0:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << (uint8)dmaInttype));
            CLEAR_BIT(DMA->dmair_dma_int_mask, (1 << (uint8)dmaInttype));
            break;
        }
        case DMA1:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 5)));
            CLEAR_BIT(DMA->dmair_dma_int_mask, (1 << ((uint8)dmaInttype + 5)));
            break;
        }
        case DMA2:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 10)));
            CLEAR_BIT(DMA->dmair_dma_int_mask, (1 << ((uint8)dmaInttype + 10)));
            break;
        }
        case DMA3:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 15)));
            CLEAR_BIT(DMA->dmair_dma_int_mask, (1 << ((uint8)dmaInttype + 15)));
            break;
        }
        case DMA4:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 20)));
            CLEAR_BIT(DMA->dmair_dma_int_mask, (1 << ((uint8)dmaInttype + 20)));
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
 * @brief   Clear the DMA interrupt DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   dmaInttype: Dma interrupt type.
 * @return  None.
 */
void clearDmaInterrupt(dmaSelTypedef sel, dmaInterruptTypedef dmaInttype)
{
    switch(sel)
    {
        case DMA0:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << (uint8)dmaInttype));
            break;
        }
        case DMA1:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 5)));
            break;
        }
        case DMA2:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 10)));
            break;
        }
        case DMA3:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 15)));
            break;
        }
        case DMA4:
        {
            SET_BIT(DMA->dmair_dma_int_clear, (1 << ((uint8)dmaInttype + 20)));
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
 * @brief   Set the address for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   srcAddr: The source memory address of the data.
 * @param   dstAddr: The destination memory address of the data.
 * @return  1: Success, 0: Failure.
 */
uint8 setDmaAddr(dmaSelTypedef sel, uint32 srcAddr, uint32 dstAddr)
{
    uint8 res   = 1;

    switch(sel)
    {
        case DMA0:
        {
            CLEAR_VALUE(DMA->dma_0_src_addr);
            CLEAR_VALUE(DMA->dma_0_dst_addr);
            DMA->dma_0_src_addr = srcAddr;
            DMA->dma_0_dst_addr = dstAddr;
            break;
        }
        case DMA1:
        {
            CLEAR_VALUE(DMA->dma_1_src_addr);
            CLEAR_VALUE(DMA->dma_1_dst_addr);
            DMA->dma_1_src_addr = srcAddr;
            DMA->dma_1_dst_addr = dstAddr;
            break;
        }
        case DMA2:
        {
            CLEAR_VALUE(DMA->dma_2_src_addr);
            CLEAR_VALUE(DMA->dma_2_dst_addr);
            DMA->dma_2_src_addr = srcAddr;
            DMA->dma_2_dst_addr = dstAddr;
            break;
        }
        case DMA3:
        {
            CLEAR_VALUE(DMA->dma_3_src_addr);
            CLEAR_VALUE(DMA->dma_3_dst_addr);
            DMA->dma_3_src_addr = srcAddr;
            DMA->dma_3_dst_addr = dstAddr;
            break;
        }
        case DMA4:
        {
            CLEAR_VALUE(DMA->dma_4_src_addr);
            CLEAR_VALUE(DMA->dma_4_dst_addr);
            DMA->dma_4_src_addr = srcAddr;
            DMA->dma_4_dst_addr = dstAddr;
            break;
        }
        default:
        {
            res = 0;
            break;
        }
    }

    return res;
}

/**
 * @brief   Get the source memory address for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  Get the set dma source address.
 */
uint32 getDmaSrcAddr(dmaSelTypedef sel)
{
    uint32 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = DMA->dma_0_src_addr;
            break;
        }
        case DMA1:
        {
            val = DMA->dma_1_src_addr;
            break;
        }
        case DMA2:
        {
            val = DMA->dma_2_src_addr;
            break;
        }
        case DMA3:
        {
            val = DMA->dma_3_src_addr;
            break;
        }
        case DMA4:
        {
            val = DMA->dma_4_src_addr;
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}

/**
 * @brief   Get the destination memory address for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  Get the set dma destination address.
 */
uint32 getDmaDstAddr(dmaSelTypedef sel)
{
    uint32 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = DMA->dma_0_dst_addr;
            break;
        }
        case DMA1:
        {
            val = DMA->dma_1_dst_addr;
            break;
        }
        case DMA2:
        {
            val = DMA->dma_2_dst_addr;
            break;
        }
        case DMA3:
        {
            val = DMA->dma_3_dst_addr;
            break;
        }
        case DMA4:
        {
            val = DMA->dma_4_dst_addr;
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Set the step for DMA.
 * @param   sel:DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   srcStep: The source step of the data.
 * @param   dstStep: The destination setp of the data.
 * @return  1: Success, 0: Failure.
 */
void setDmaStep(dmaSelTypedef sel, uint16 srcStep, uint16 dstStep)
{
    switch(sel)
    {
        case DMA0:
        {
            SET_VALUE(DMA->dma_0_addr_step, \
                      dma_0_addr_step_0_src_addr_step_shift, \
                      dma_0_addr_step_0_src_addr_step_mask, \
                      srcStep);
            SET_VALUE(DMA->dma_0_addr_step, \
                      dma_0_addr_step_0_dst_addr_step_shift, \
                      dma_0_addr_step_0_dst_addr_step_mask, \
                      dstStep);
            break;
        }
        case DMA1:
        {
            SET_VALUE(DMA->dma_1_addr_step, \
                      dma_1_addr_step_1_src_addr_step_shift, \
                      dma_1_addr_step_1_src_addr_step_mask, \
                      srcStep);
            SET_VALUE(DMA->dma_1_addr_step, \
                      dma_1_addr_step_1_dst_addr_step_shift, \
                      dma_1_addr_step_1_dst_addr_step_mask, \
                      dstStep);
            break;
        }
        case DMA2:
        {
            SET_VALUE(DMA->dma_2_addr_step, \
                      dma_2_addr_step_2_src_addr_step_shift, \
                      dma_2_addr_step_2_src_addr_step_mask, \
                      srcStep);
            SET_VALUE(DMA->dma_2_addr_step, \
                      dma_2_addr_step_2_dst_addr_step_shift, \
                      dma_2_addr_step_2_dst_addr_step_mask, \
                      dstStep);
            break;
        }
        case DMA3:
        {
            SET_VALUE(DMA->dma_3_addr_step, \
                      dma_3_addr_step_3_src_addr_step_shift, \
                      dma_3_addr_step_3_src_addr_step_mask, \
                      srcStep);
            SET_VALUE(DMA->dma_3_addr_step, \
                      dma_3_addr_step_3_dst_addr_step_shift, \
                      dma_3_addr_step_3_dst_addr_step_mask, \
                      dstStep);
            break;
        }
        case DMA4:
        {
            SET_VALUE(DMA->dma_4_addr_step, \
                      dma_4_addr_step_4_src_addr_step_shift, \
                      dma_4_addr_step_4_src_addr_step_mask, \
                      srcStep);
            SET_VALUE(DMA->dma_4_addr_step, \
                      dma_4_addr_step_4_dst_addr_step_shift, \
                      dma_4_addr_step_4_dst_addr_step_mask, \
                      dstStep);
            break;
        }
        default:
        {
            break;
        }
    }
}


/**
 * @brief   Get the source memory step for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  Get the set dma source step.
 */
uint16 getDmaSrcStep(dmaSelTypedef sel)
{
    uint16 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BITS_VALUE(DMA->dma_0_addr_step, \
                                 dma_0_addr_step_0_src_addr_step_shift, \
                                 dma_0_addr_step_0_src_addr_step_mask);
            break;
        }
        case DMA1:
        {
            val = GET_BITS_VALUE(DMA->dma_1_addr_step, \
                                 dma_1_addr_step_1_src_addr_step_shift, \
                                 dma_1_addr_step_1_src_addr_step_mask);
            break;
        }
        case DMA2:
        {
            val = GET_BITS_VALUE(DMA->dma_2_addr_step, \
                                 dma_2_addr_step_2_src_addr_step_shift, \
                                 dma_2_addr_step_2_src_addr_step_mask);
            break;
        }
        case DMA3:
        {
            val = GET_BITS_VALUE(DMA->dma_3_addr_step, \
                                 dma_3_addr_step_3_src_addr_step_shift, \
                                 dma_3_addr_step_3_src_addr_step_mask);
            break;
        }
        case DMA4:
        {
            val = GET_BITS_VALUE(DMA->dma_4_addr_step, \
                                 dma_4_addr_step_4_src_addr_step_shift, \
                                 dma_4_addr_step_4_src_addr_step_mask);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}

/**
 * @brief   Get the destination step address for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  Get the set dma destination step.
 */
uint16 getDmaDstStep(dmaSelTypedef sel)
{
    uint16 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BITS_VALUE(DMA->dma_0_addr_step, \
                                 dma_0_addr_step_0_dst_addr_step_shift, \
                                 dma_0_addr_step_0_dst_addr_step_mask);
            break;
        }
        case DMA1:
        {
            val = GET_BITS_VALUE(DMA->dma_1_addr_step, \
                                 dma_1_addr_step_1_dst_addr_step_shift, \
                                 dma_1_addr_step_1_dst_addr_step_mask);
            break;
        }
        case DMA2:
        {
            val = GET_BITS_VALUE(DMA->dma_2_addr_step, \
                                 dma_2_addr_step_2_dst_addr_step_shift, \
                                 dma_2_addr_step_2_dst_addr_step_mask);
            break;
        }
        case DMA3:
        {
            val = GET_BITS_VALUE(DMA->dma_3_addr_step, \
                                 dma_3_addr_step_3_dst_addr_step_shift, \
                                 dma_3_addr_step_3_dst_addr_step_mask);
            break;
        }
        case DMA4:
        {
            val = GET_BITS_VALUE(DMA->dma_4_addr_step, \
                                 dma_4_addr_step_4_dst_addr_step_shift, \
                                 dma_4_addr_step_4_dst_addr_step_mask);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Enable DMA settings.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setDmaEnable(dmaSelTypedef sel)
{
    uint8 res = 1;
    switch(sel)
    {
        case DMA0:
        {
            SET_BIT(DMA->dma_0_config, dma_0_config_0_en_bit);
            break;
        }
        case DMA1:
        {
            SET_BIT(DMA->dma_1_config, dma_1_config_1_en_bit);
            break;
        }
        case DMA2:
        {
            SET_BIT(DMA->dma_2_config, dma_2_config_2_en_bit);
            break;
        }
        case DMA3:
        {
            SET_BIT(DMA->dma_3_config, dma_3_config_3_en_bit);
            break;
        }
        case DMA4:
        {
            SET_BIT(DMA->dma_4_config, dma_4_config_4_en_bit);
            break;
        }
        default:
        {
            res = 0;
            break;
        }
    }

    return res;
}

/**
 * @brief   Dsiable DMA settings.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setDmaDisable(dmaSelTypedef sel)
{
    uint8 res = 1;
    switch(sel)
    {
        case DMA0:
        {
            CLEAR_BIT(DMA->dma_0_config, dma_0_config_0_en_bit);
            break;
        }
        case DMA1:
        {
            CLEAR_BIT(DMA->dma_1_config, dma_1_config_1_en_bit);
            break;
        }
        case DMA2:
        {
            CLEAR_BIT(DMA->dma_2_config, dma_2_config_2_en_bit);
            break;
        }
        case DMA3:
        {
            CLEAR_BIT(DMA->dma_3_config, dma_3_config_3_en_bit);
            break;
        }
        case DMA4:
        {
            CLEAR_BIT(DMA->dma_4_config, dma_4_config_4_en_bit);
            break;
        }
        default:
        {
            res = 0;
            break;
        }
    }

    return res;
}

/**
 * @brief   Get the status of DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  1: Enable, 0: Disable.
 */
uint8 getDmaStatus(dmaSelTypedef sel)
{
    uint8 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BIT_VALUE(DMA->dma_0_config, BIT_31);
            break;
        }
        case DMA1:
        {
            val = GET_BIT_VALUE(DMA->dma_1_config, BIT_31);
            break;
        }
        case DMA2:
        {
            val = GET_BIT_VALUE(DMA->dma_2_config, BIT_31);
            break;
        }
        case DMA3:
        {
            val = GET_BIT_VALUE(DMA->dma_3_config, BIT_31);
            break;
        }
        case DMA4:
        {
            val = GET_BIT_VALUE(DMA->dma_4_config, BIT_31);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Set the peripherals used by DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   peri: Peripherals support by DMA, see also @ref dmaPeriTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setDmaPeriSel(dmaSelTypedef sel, dmaPeriTypedef peri)
{
    uint8 res = 1;
    switch(sel)
    {
        case DMA0:
        {
            SET_VALUE(DMA->dma_0_config, \
                      dma_0_config_0_peri_sel_shift, \
                      dma_0_config_0_peri_sel_mask, \
                      (uint8)peri);
            break;
        }
        case DMA1:
        {
            SET_VALUE(DMA->dma_1_config, \
                      dma_1_config_1_peri_sel_shift, \
                      dma_1_config_1_peri_sel_mask, \
                      (uint8)peri);
            break;
        }
        case DMA2:
        {
            SET_VALUE(DMA->dma_2_config, \
                      dma_2_config_2_peri_sel_shift, \
                      dma_2_config_2_peri_sel_mask, \
                      (uint8)peri);
            break;
        }
        case DMA3:
        {
            SET_VALUE(DMA->dma_3_config, \
                      dma_3_config_3_peri_sel_shift, \
                      dma_3_config_3_peri_sel_mask, \
                      (uint8)peri);
            break;
        }
        case DMA4:
        {
            SET_VALUE(DMA->dma_4_config, \
                      dma_4_config_4_peri_sel_shift, \
                      dma_4_config_4_peri_sel_mask, \
                      (uint8)peri);
            break;
        }
        default:
        {
            res = 0;
            break;
        }
    }

    return res;
}

/**
 * @brief   Get whether specific DMA peripherals are being used.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  1: Used, 0: No Used.
 */
uint8 getDmaPeriSel(dmaSelTypedef sel)
{
    uint8 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BITS_VALUE(DMA->dma_0_config, \
                                 dma_0_config_0_peri_sel_shift, \
                                 dma_0_config_0_peri_sel_mask);
            break;
        }
        case DMA1:
        {
            val = GET_BITS_VALUE(DMA->dma_1_config, \
                                 dma_1_config_1_peri_sel_shift, \
                                 dma_1_config_1_peri_sel_mask);
            break;
        }
        case DMA2:
        {
            val = GET_BITS_VALUE(DMA->dma_2_config, \
                                 dma_2_config_2_peri_sel_shift, \
                                 dma_2_config_2_peri_sel_mask);
            break;
        }
        case DMA3:
        {
            val = GET_BITS_VALUE(DMA->dma_3_config, \
                                 dma_3_config_3_peri_sel_shift, \
                                 dma_3_config_3_peri_sel_mask);
            break;
        }
        case DMA4:
        {
            val = GET_BITS_VALUE(DMA->dma_4_config, \
                                 dma_4_config_4_peri_sel_shift, \
                                 dma_4_config_4_peri_sel_mask);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Set the pripority for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   pri: DMA priority level, see also @ref dmaPriorityTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setDmaPriority(dmaSelTypedef sel, dmaPriorityTypedef pri)
{
    uint8 res = 1;

    switch(sel)
    {
        case DMA0:
        {
            if(DMA_PRIORITY0 == pri)
            {
                CLEAR_BIT(DMA->dma_0_config, dma_0_config_0_pri_bit);
            }
            else if(DMA_PRIORITY1 == pri)
            {
                SET_BIT(DMA->dma_0_config, dma_0_config_0_pri_bit);
            }
            break;
        }
        case DMA1:
        {
            if(DMA_PRIORITY0 == pri)
            {
                CLEAR_BIT(DMA->dma_1_config, dma_1_config_1_pri_bit);
            }
            else if(DMA_PRIORITY1 == pri)
            {
                SET_BIT(DMA->dma_1_config, dma_1_config_1_pri_bit);
            }
            break;
        }
        case DMA2:
        {
            if(DMA_PRIORITY0 == pri)
            {
                CLEAR_BIT(DMA->dma_2_config, dma_2_config_2_pri_bit);
            }
            else if(DMA_PRIORITY1 == pri)
            {
                SET_BIT(DMA->dma_2_config, dma_2_config_2_pri_bit);
            }
            break;
        }
        case DMA3:
        {
            if(DMA_PRIORITY0 == pri)
            {
                CLEAR_BIT(DMA->dma_3_config, dma_3_config_3_pri_bit);
            }
            else if(DMA_PRIORITY1 == pri)
            {
                SET_BIT(DMA->dma_3_config, dma_3_config_3_pri_bit);
            }
            break;
        }
        case DMA4:
        {
            if(DMA_PRIORITY0 == pri)
            {
                CLEAR_BIT(DMA->dma_4_config, dma_4_config_4_pri_bit);
            }
            else if(DMA_PRIORITY1 == pri)
            {
                SET_BIT(DMA->dma_4_config, dma_4_config_4_pri_bit);
            }
            break;
        }
        default:
        {
            res = 0;
            break;
        }
    }

    return res;
}

/**
 * @brief   Get DMA priority level.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  DMA priority level(0/1).
 */
uint8 getDmaPriority(dmaSelTypedef sel)
{
    uint8 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BIT_VALUE(DMA->dma_0_config, BIT_17);
            break;
        }
        case DMA1:
        {
            val = GET_BIT_VALUE(DMA->dma_1_config, BIT_17);
            break;
        }
        case DMA2:
        {
            val = GET_BIT_VALUE(DMA->dma_2_config, BIT_17);
            break;
        }
        case DMA3:
        {
            val = GET_BIT_VALUE(DMA->dma_3_config, BIT_17);
            break;
        }
        case DMA4:
        {
            val = GET_BIT_VALUE(DMA->dma_4_config, BIT_17);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Set the length for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   len: DMA transmission length.
 * @return  1: Success, 0: Failure.
 */
uint8 setDmaLen(dmaSelTypedef sel, uint16 len)
{
    uint8 res = 1;
    switch(sel)
    {
        case DMA0:
        {
            SET_VALUE(DMA->dma_0_config, \
                      dma_0_config_0_cfg_cnt_shift, \
                      dma_0_config_0_cfg_cnt_mask, \
                      len);
            break;
        }
        case DMA1:
        {
            SET_VALUE(DMA->dma_1_config, \
                      dma_1_config_1_cfg_cnt_shift, \
                      dma_1_config_1_cfg_cnt_mask, \
                      len);
            break;
        }
        case DMA2:
        {
            SET_VALUE(DMA->dma_2_config, \
                      dma_2_config_2_cfg_cnt_shift, \
                      dma_2_config_2_cfg_cnt_mask, \
                      len);
            break;
        }
        case DMA3:
        {
            SET_VALUE(DMA->dma_3_config, \
                      dma_3_config_3_cfg_cnt_shift, \
                      dma_3_config_3_cfg_cnt_mask, \
                      len);
            break;
        }
        case DMA4:
        {
            SET_VALUE(DMA->dma_4_config, \
                      dma_4_config_4_cfg_cnt_shift, \
                      dma_4_config_4_cfg_cnt_mask, \
                      len);
            break;
        }
        default:
        {
            res = 0;
            break;
        }
    }

    return res;
}

/**
 * @brief   Get the length for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  length.
 */
uint16 getDmaLen(dmaSelTypedef sel)
{
    uint16 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BITS_VALUE(DMA->dma_0_config, \
                                 dma_0_config_0_cfg_cnt_shift, \
                                 dma_0_config_0_cfg_cnt_mask);
            break;
        }
        case DMA1:
        {
            val = GET_BITS_VALUE(DMA->dma_1_config, \
                                 dma_1_config_1_cfg_cnt_shift, \
                                 dma_1_config_1_cfg_cnt_mask);
            break;
        }
        case DMA2:
        {
            val = GET_BITS_VALUE(DMA->dma_2_config, \
                                 dma_2_config_2_cfg_cnt_shift, \
                                 dma_2_config_2_cfg_cnt_mask);
            break;
        }
        case DMA3:
        {
            val = GET_BITS_VALUE(DMA->dma_3_config, \
                                 dma_3_config_3_cfg_cnt_shift, \
                                 dma_3_config_3_cfg_cnt_mask);
            break;
        }
        case DMA4:
        {
            val = GET_BITS_VALUE(DMA->dma_4_config, \
                                 dma_4_config_4_cfg_cnt_shift, \
                                 dma_4_config_4_cfg_cnt_mask);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Set DMA transmission mode.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   mode: DMA transmission mode, see also @ref dmaTransferModeTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setDmaTransferMode(dmaSelTypedef sel, dmaTransferModeTypedef mode)
{
    uint8 res = 1;

    switch(sel)
    {
        case DMA0:
        {
            if(PERITOMEM == mode)
            {
                CLEAR_BIT(DMA->dma_0_config, dma_0_config_0_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_0_config, dma_0_config_0_mem_to_peri_bit);
            }
            else if(MEMTOPERI == mode)
            {
                CLEAR_BIT(DMA->dma_0_config, dma_0_config_0_mem_to_mem_bit);
                SET_BIT(DMA->dma_0_config, dma_0_config_0_mem_to_peri_bit);
            }
            else if(MEMTOMEM == mode)
            {
                SET_BIT(DMA->dma_0_config, dma_0_config_0_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_0_config, dma_0_config_0_mem_to_peri_bit);
            }
            break;
        }
        case DMA1:
        {
            if(PERITOMEM == mode)
            {
                CLEAR_BIT(DMA->dma_1_config, dma_1_config_1_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_1_config, dma_1_config_1_mem_to_peri_bit);
            }
            else if(MEMTOPERI == mode)
            {
                CLEAR_BIT(DMA->dma_1_config, dma_1_config_1_mem_to_mem_bit);
                SET_BIT(DMA->dma_1_config, dma_1_config_1_mem_to_peri_bit);
            }
            else if(MEMTOMEM == mode)
            {
                SET_BIT(DMA->dma_1_config, dma_1_config_1_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_1_config, dma_1_config_1_mem_to_peri_bit);
            }
            break;
        }
        case DMA2:
        {
            if(PERITOMEM == mode)
            {
                CLEAR_BIT(DMA->dma_2_config, dma_2_config_2_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_2_config, dma_2_config_2_mem_to_peri_bit);
            }
            else if(MEMTOPERI == mode)
            {
                CLEAR_BIT(DMA->dma_2_config, dma_2_config_2_mem_to_mem_bit);
                SET_BIT(DMA->dma_2_config, dma_2_config_2_mem_to_peri_bit);
            }
            else if(MEMTOMEM == mode)
            {
                SET_BIT(DMA->dma_2_config, dma_2_config_2_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_2_config, dma_2_config_2_mem_to_peri_bit);
            }
            break;
        }
        case DMA3:
        {
            if(PERITOMEM == mode)
            {
                CLEAR_BIT(DMA->dma_3_config, dma_3_config_3_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_3_config, dma_3_config_3_mem_to_peri_bit);
            }
            else if(MEMTOPERI == mode)
            {
                CLEAR_BIT(DMA->dma_3_config, dma_3_config_3_mem_to_mem_bit);
                SET_BIT(DMA->dma_3_config, dma_3_config_3_mem_to_peri_bit);
            }
            else if(MEMTOMEM == mode)
            {
                SET_BIT(DMA->dma_3_config, dma_3_config_3_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_3_config, dma_3_config_3_mem_to_peri_bit);
            }
            break;
        }
        case DMA4:
        {
            if(PERITOMEM == mode)
            {
                CLEAR_BIT(DMA->dma_4_config, dma_4_config_4_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_4_config, dma_4_config_4_mem_to_peri_bit);
            }
            else if(MEMTOPERI == mode)
            {
                CLEAR_BIT(DMA->dma_4_config, dma_4_config_4_mem_to_mem_bit);
                SET_BIT(DMA->dma_4_config, dma_4_config_4_mem_to_peri_bit);
            }
            else if(MEMTOMEM == mode)
            {
                SET_BIT(DMA->dma_4_config, dma_4_config_4_mem_to_mem_bit);
                CLEAR_BIT(DMA->dma_4_config, dma_4_config_4_mem_to_peri_bit);
            }
            break;
        }
        default:
        {
            res = 0;
            break;
        }
    }

    return res;
}

/**
 * @brief   Get DMA transmission mode.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  DMA transmission mode.
 */
uint8 getDmaTransferMode(dmaSelTypedef sel)
{
    uint8 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BIT_VALUE(DMA->dma_0_config, BIT_23);
            break;
        }
        case DMA1:
        {
            val = GET_BIT_VALUE(DMA->dma_1_config, BIT_23);
            break;
        }
        case DMA2:
        {
            val = GET_BIT_VALUE(DMA->dma_2_config, BIT_23);
            break;
        }
        case DMA3:
        {
            val = GET_BIT_VALUE(DMA->dma_3_config, BIT_23);
            break;
        }
        case DMA4:
        {
            val = GET_BIT_VALUE(DMA->dma_4_config, BIT_23);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Set the size for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @param   srcAddr: The source size of the data.
 * @param   dstAddr: The destination size of the data.
 * @return  1: Success, 0: Failure.
 */
uint8 setDmaSize(dmaSelTypedef sel, uint8 srcSize, uint32 dstSize)
{
    uint8 res = 1;

    switch(sel)
    {
        case DMA0:
        {
            SET_VALUE(DMA->dma_0_config, \
                      dma_0_config_0_src_size_shift, \
                      dma_0_config_0_src_size_mask, \
                      srcSize);

            SET_VALUE(DMA->dma_0_config, \
                      dma_0_config_0_dst_size_shift, \
                      dma_0_config_0_dst_size_mask, \
                      dstSize);
            break;
        }
        case DMA1:
        {
            SET_VALUE(DMA->dma_1_config, \
                      dma_1_config_1_src_size_shift, \
                      dma_1_config_1_src_size_mask, \
                      srcSize);

            SET_VALUE(DMA->dma_1_config, \
                      dma_1_config_1_dst_size_shift, \
                      dma_1_config_1_dst_size_mask, \
                      dstSize);
            break;
        }
        case DMA2:
        {
            SET_VALUE(DMA->dma_2_config, \
                      dma_2_config_2_src_size_shift, \
                      dma_2_config_2_src_size_mask, \
                      srcSize);

            SET_VALUE(DMA->dma_2_config, \
                      dma_2_config_2_dst_size_shift, \
                      dma_2_config_2_dst_size_mask, \
                      dstSize);
            break;
        }
        case DMA3:
        {
            SET_VALUE(DMA->dma_3_config, \
                      dma_3_config_3_src_size_shift, \
                      dma_3_config_3_src_size_mask, \
                      srcSize);

            SET_VALUE(DMA->dma_3_config, \
                      dma_3_config_3_dst_size_shift, \
                      dma_3_config_3_dst_size_mask, \
                      dstSize);
            break;
        }
        case DMA4:
        {
            SET_VALUE(DMA->dma_4_config, \
                      dma_4_config_4_src_size_shift, \
                      dma_4_config_4_src_size_mask, \
                      srcSize);

            SET_VALUE(DMA->dma_4_config, \
                      dma_4_config_4_dst_size_shift, \
                      dma_4_config_4_dst_size_mask, \
                      dstSize);
            break;
        }
        default:
        {
            res = 0;
            break;
        }
    }

    return res;
}

/**
 * @brief   Get the soruce size for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  source size.
 */
uint8 getDmaSrcSize(dmaSelTypedef sel)
{
    uint8 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BITS_VALUE(DMA->dma_0_config, \
                                 dma_0_config_0_src_size_shift, \
                                 dma_0_config_0_src_size_mask);
            break;
        }
        case DMA1:
        {
            val = GET_BITS_VALUE(DMA->dma_1_config, \
                                 dma_1_config_1_src_size_shift, \
                                 dma_1_config_1_src_size_mask);
            break;
        }
        case DMA2:
        {
            val = GET_BITS_VALUE(DMA->dma_2_config, \
                                 dma_2_config_2_src_size_shift, \
                                 dma_2_config_2_src_size_mask);
            break;
        }
        case DMA3:
        {
            val = GET_BITS_VALUE(DMA->dma_3_config, \
                                 dma_3_config_3_src_size_shift, \
                                 dma_3_config_3_src_size_mask);
            break;
        }
        case DMA4:
        {
            val = GET_BITS_VALUE(DMA->dma_4_config, \
                                 dma_4_config_4_src_size_shift, \
                                 dma_4_config_4_src_size_mask);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}

/**
 * @brief   Get the destination size for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  destination size.
 */
uint8 getDmaDstSize(dmaSelTypedef sel)
{
    uint8 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BITS_VALUE(DMA->dma_0_config, \
                                 dma_0_config_0_dst_size_shift, \
                                 dma_0_config_0_dst_size_mask);
            break;
        }
        case DMA1:
        {
            val = GET_BITS_VALUE(DMA->dma_1_config, \
                                 dma_1_config_1_dst_size_shift, \
                                 dma_1_config_1_dst_size_mask);
            break;
        }
        case DMA2:
        {
            val = GET_BITS_VALUE(DMA->dma_2_config, \
                                 dma_2_config_2_dst_size_shift, \
                                 dma_2_config_2_dst_size_mask);
            break;
        }
        case DMA3:
        {
            val = GET_BITS_VALUE(DMA->dma_3_config, \
                                 dma_3_config_3_dst_size_shift, \
                                 dma_3_config_3_dst_size_mask);
            break;
        }
        case DMA4:
        {
            val = GET_BITS_VALUE(DMA->dma_4_config, \
                                 dma_4_config_4_dst_size_shift, \
                                 dma_4_config_4_dst_size_mask);
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Get the transfered count for DMA.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  transfered count.
 */
uint16 getDmaTransferCount(dmaSelTypedef sel)
{
    uint16 val = 0;

    switch(sel)
    {
        case DMA0:
        {
            val = GET_BITS_VALUE(DMA->dma_0_tcnt, \
                                 0, \
                                 ((1 << 16) - 1));
            break;
        }
        case DMA1:
        {
            val = GET_BITS_VALUE(DMA->dma_1_tcnt, \
                                 0, \
                                 ((1 << 16) - 1));
            break;
        }
        case DMA2:
        {
            val = GET_BITS_VALUE(DMA->dma_2_tcnt, \
                                 0, \
                                 ((1 << 16) - 1));
            break;
        }
        case DMA3:
        {
            val = GET_BITS_VALUE(DMA->dma_3_tcnt, \
                                 0, \
                                 ((1 << 16) - 1));
            break;
        }
        case DMA4:
        {
            val = GET_BITS_VALUE(DMA->dma_4_tcnt, \
                                 0, \
                                 ((1 << 16) - 1));
            break;
        }
        default:
        {
            break;
        }
    }

    return val;
}


/**
 * @brief   Waiting for DMA transfer to complete.
 * @param   sel: DMA0~DMA4, see also @ref dmaSelTypedef.
 * @return  transfered count.
 */
void waitingForDmaTransfer(dmaSelTypedef sel, uint16 cnt)
{
    uint32 regVal = 0;

    while(1)
    {
        regVal = getDmaTransferCount(sel);
        if(regVal >= cnt)
        {
            break;
        }
    }
}
