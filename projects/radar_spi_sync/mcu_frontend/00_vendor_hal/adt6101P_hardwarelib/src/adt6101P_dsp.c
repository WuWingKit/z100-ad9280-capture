/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_dsp.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-21
 * @brief   This file contains all the functions implemented by the DSP hardware
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_dsp.h"
#include "dsp_ctype_map.h"
#include <stdio.h>


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   Set FFT enable.
 * @param   en: DSP_ENABLE/DSP_DISABLE, see also @ref dspCtrlTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setFftEnable(dspCtrlTypedef en)
{
    uint8 res = 1;

    switch(en)
    {
        case DSP_ENABLE:
        {
            SET_BIT(DSP->dsp_fft_ctrl, dsp_fft_ctrl_fft_en_bit);
            break;
        }
        case DSP_DISABLE:
        {
            CLEAR_BIT(DSP->dsp_fft_ctrl, dsp_fft_ctrl_fft_en_bit);
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
 * @brief   Get FFT status.
 * @param   None.
 * @return  1: FFT enable, 0 : FFT disable.
 */
uint8 getFftStatus(void)
{
    return GET_BIT_VALUE(DSP->dsp_fft_ctrl, BIT_10);
}


/**
 * @brief   Set FFT exponent enable.
 * @param   en: EXPONENT_ENABLE/EXPONENT_DISABLE, see also @ref dspExponentTypedef.
 * @return  1: Success, 0: Failure.
 * @note    Disable exponent, that is, use fixed point
 */
uint8 setFftExponentEnable(dspExponentTypedef en)
{
    uint8 res = 1;

    switch(en)
    {
        case EXPONENT_ENABLE:
        {
            CLEAR_BIT(DSP->dsp_fft_ctrl, dsp_fft_ctrl_fft_exp_bp_bit);
            break;
        }
        case EXPONENT_DISABLE:
        {
            SET_BIT(DSP->dsp_fft_ctrl, dsp_fft_ctrl_fft_exp_bp_bit);
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
 * @brief   Get FFT exponent status.
 * @param   None.
 * @return  0: enable exponent 1: disable exponent, that is, use fixed point.
 */
uint8 getFftExponentEnable(void)
{
    return GET_BIT_VALUE(DSP->dsp_fft_ctrl, BIT_11);
}

/**
 * @brief   Set FFT window enable.
 * @param   en: FFT_WINDOW_ENABLE/FFT_WINDOW_BYPASS, see also @ref fftWnTypedef.
 * @return  1: Success, 0: Failure.
 * @note    Disable exponent, that is, use fixed point
 */
uint8 setFftWindowtEnable(fftWnTypedef en)
{
    uint8 res = 1;

    switch(en)
    {
        case FFT_WINDOW_BYPASS:
        {
            SET_BIT(DSP->dsp_fft_ctrl, dsp_fft_ctrl_fft_wn_bypass_bit);
            break;
        }
        case FFT_WINDOW_ENABLE:
        {
            CLEAR_BIT(DSP->dsp_fft_ctrl, dsp_fft_ctrl_fft_wn_bypass_bit);
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
 * @brief   Get FFT window status.
 * @param   None.
 * @return  0: FFT_WINDOW_ENABLE 1: FFT_WINDOW_BYPASS.
 */
uint8 getFftWindowtEnable(void)
{
    return GET_BIT_VALUE(DSP->dsp_fft_ctrl, BIT_20);
}


/**
 * @brief   Set FFT length.
 * @param   len: FFT length.
 * @return  None.
 */
void setFftLen(uint16 len)
{
    SET_VALUE(DSP->dsp_fft_ctrl, \
              dsp_fft_ctrl_fft_len_shift, \
              dsp_fft_ctrl_fft_len_mask, \
              len);
}

/**
 * @brief   Get FFT length.
 * @param   None.
 * @return  fft length..
 */
uint8 getFftLen(void)
{
    return GET_BITS_VALUE(DSP->dsp_fft_ctrl, \
                          dsp_fft_ctrl_fft_len_shift, \
                          dsp_fft_ctrl_fft_len_mask);
}


/**
 * @brief   Set FFT start trigger signal.
 * @param   mode: SINGLE_START/SINGLE_STOP, see also @ref dspStartSingleTypedef.
 * @return  None.
 */
uint8 setFftStartSingle(dspStartSingleTypedef mode)
{
    uint8 res = 1;

    switch(mode)
    {
        case SINGLE_START:
        {
            SET_BIT(DSP->dsp_int_clear, dsp_int_clear_fft_start_bit);
            break;
        }
        case SINGLE_STOP:
        {
            CLEAR_BIT(DSP->dsp_int_clear, dsp_int_clear_fft_start_bit);
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
