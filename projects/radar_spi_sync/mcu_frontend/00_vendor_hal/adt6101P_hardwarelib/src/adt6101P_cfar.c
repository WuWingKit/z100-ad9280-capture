/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_cfar.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file contains all the functions implemented by the cfar peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_cfar.h"
#include "dsp_ctype_map.h"

/* Private functions --------------------------------------------------------*/
/**
 * @brief   set Cfar Str
 * @param   value
 * @return  None.
 */
void setCfarStr(uint32 value)
{
    WRITE_REG(DSP->cfar_str, value);
}

/**
 * @brief   set Cfar Str.
 * @param   value
 * @return  None.
 */
void setCfarDatNum(uint32 value)
{
    SET_VALUE(DSP->cfar_dat_number, cfar_dat_number_dat_num_shift, \
              cfar_dat_number_dat_num_mask, value);
}

/**
 * @brief   set Cfar Da tAcc Sum Frz.
 * @param   bitStateTypedef: ENABLE/DISABLE, see also @ref stateTypedef.
 * @return  None.
 */
void setCfarDatAccSumFrz(bitStateTypedef set)
{
    if(RESET == set)
    {
        CLEAR_BIT(DSP->cfar_dat_number, cfar_dat_number_acc_sum_frz_bit);
    }
    else
    {
        SET_BIT(DSP->cfar_dat_number, cfar_dat_number_acc_sum_frz_bit);
    }
}

/**
 * @brief   get Cfar Dat Number.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarDatNumber(void)
{
    return DSP->cfar_dat_number;
}

/**
 * @brief   set Cfar Mask0.
 * @param   value
 * @return  None.
 */
void setCfarMask0(uint32 value)
{
    WRITE_REG(DSP->cfar_mask0, value);
}

/**
 * @brief   get Cfar Mask0.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarMask0(void)
{
    return DSP->cfar_mask0;
}

/**
 * @brief   set Cfar Mask1.
 * @param   value
 * @return  None.
 */
void setCfarMask1(uint32 value)
{
    WRITE_REG(DSP->cfar_mask1, value);
}

/**
 * @brief   get Cfar Mask1.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarMask1(void)
{
    return DSP->cfar_mask1;
}

/**
 * @brief   get Cfar Done Acc.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarDoneAcc(void)
{
    return DSP->cfar_done & cfar_done_acc_done_bit;
}

/**
 * @brief   get Cfar Done Ord.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarDoneOrd(void)
{
    return DSP->cfar_done & cfar_done_ord_done_bit;
}

/**
 * @brief   get Cfar Acc Sum H.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarAccSumH(void)
{
    return DSP->cfar_acc_sum_h;
}

/**
 * @brief   get Cfar Acc Sum L.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarAccSumL(void)
{
    return DSP->cfar_acc_sum_l;
}

/**
 * @brief   set Cfar ord Idx.
 * @param   value
 * @return  None.
 */
void setCfarOrdIdx(uint32 value)
{
    WRITE_REG(DSP->cfar_ord_idx, value);
}

/**
 * @brief   get Cfar Ord Idx.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarOrdIdx(void)
{
    return DSP->cfar_ord_idx;
}

/**
 * @brief   get Cfar Ord Value.
 * @param   None.
 * @return  uint32.
 */
uint32 getCfarOrdValue(void)
{
    return DSP->cfar_ord_value;
}
