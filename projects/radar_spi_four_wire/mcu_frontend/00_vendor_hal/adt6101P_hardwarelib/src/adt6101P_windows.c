/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_windows.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-23
 * @brief   This file contains all the functions implemented by the Windows
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_windows.h"
#include "dsp_ctype_map.h"


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   Is ADC windowing bypassed.
 * @param   en: ADC_WINDOW_ENABLE/ADC_WINDOW_BYPASS, see also @ref adcWnTypedef.
 * @return  settings success/failure.
 */
uint8 isAdcWnBypass(const adcWnTypedef en)
{
    uint8 res = 0;
    if(en == ADC_WINDOW_ENABLE)
    {
        res = 1;
        CLEAR_BIT(DSP->wn_wn_para, wn_wn_para_bypass_bit);
    }
    else if(en == ADC_WINDOW_BYPASS)
    {
        res = 1;
        SET_BIT(DSP->wn_wn_para, wn_wn_para_bypass_bit);
    }

    return res;
}


/**
 * @brief   Set the effective length of the ADC window.
 * @param   len: Window length, see also @ref windowLenTypedef.
 * @return  return register specific single/multiple bit values.
 */
uint16 setAdcWnLen(const uint16 len)
{
    uint8 res = 0;

    res = 1;
    SET_VALUE(DSP->wn_wn_para, \
              wn_wn_para_wn_len_shift, \
              wn_wn_para_wn_len_mask, \
              len);
    return res;
}

/**
 * @brief   Get the effective length of the ADC window.
 * @return  Get window length.
 */
uint16 getAdcWnLen(void)
{
    return GET_BITS_VALUE(DSP->wn_wn_para, \
                          wn_wn_para_wn_len_shift, \
                          wn_wn_para_wn_len_mask);
}


/**
 * @brief   Get ADC window status.
 * @return  return register specific single/multiple bit values.
 */
uint8 adcWnStatus(void)
{
    return GET_BIT_VALUE(DSP->wn_wn_para, BIT_1);
}


/**
 * @brief   Set ADC window mode.
 * @param   mode: ADC window mode, see also @ref windowModeTypedef.
 * @return  settings success/failure.
 */
uint8 setAdcWnMode(const windowModeTypedef mode)
{
    uint8 res = 0;
    if(WINDOW_HANNING == mode)
    {
        res = 1;
        CLEAR_BIT(DSP->wn_wn_para, wn_wn_para_cof_sel_bit);
    }
    else if(WINDOW_HAMMING == mode)
    {
        res = 1;
        SET_BIT(DSP->wn_wn_para, wn_wn_para_cof_sel_bit);
    }

    return res;
}

/**
 * @brief   Get ADC window mode.
 * @return  return register specific single/multiple bit values.
 */
uint8 getAdcWnMode(void)
{
    return GET_BIT_VALUE(DSP->wn_wn_para, BIT_0);
}

