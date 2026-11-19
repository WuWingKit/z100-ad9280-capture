/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_adc.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file contains all the functions implemented by the ADC peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_adc.h"
#include "dsp_ctype_map.h"
#include "rfc_ctype_map.h"
#include "pmu_ctype_map.h"
#include "dsp_ctype_map.h"
#include "timer_bsp.h"

#include <string.h>
#include <arm_math.h>

/* Private variables ---------------------------------------------------------*/
static firDownsampleStrTypedef s_firds;


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   enable adc, and enable clk output to adc.
 * @param   adc0Enable: adc0 enable/disable.
 * @param   adc1Enable: adc1 enable/disable.
 * @return  None.
 * @note    When switching the ADC clock, the clock is from ADC0 to ADC1, see also @ref stateTypedef.
 *          ADC1 can only be enable and disable when ADC0 is on. When ADC1 is
 *          disable, ADC1 must be disable, see also @ref stateTypedef.
 */
void adcEnable(const stateTypedef adc0Enable, const stateTypedef adc1Enable)
{
    uint32 adc0Val = 0;
    uint32 adc1Val = 0;

    /* ADC1 can only be enable and disable when ADC0 is on. */
    if(ENABLE == adc0Enable)
    {
        /* Enable sar core for ADC-0 */
        SET_BIT(adc0Val, rf_adc0_regs_reg_adc0_sar_en_bit);
        /* Enable reference buffer for ADC-0 */
        SET_BIT(adc0Val, rf_adc0_regs_reg_adc0_ref_en_bit);

        if(ENABLE == adc1Enable)
        {
            /* Enable sar core for ADC-1 */
            SET_BIT(adc1Val, rf_adc1_regs_reg_adc1_sar_en_bit);
            /* Enable reference buffer for ADC-1 */
            SET_BIT(adc1Val, rf_adc1_regs_reg_adc1_ref_en_bit);
        }
    }
    else if(DISABLE == adc0Enable)
    {
        /* When ADC1 is disable, ADC1 must be disable. */
        if(DISABLE == adc1Enable)
        {
            CLEAR_VALUE(adc1Val);
        }
    }

    SET_VALUE(adc0Val, \
              rf_adc0_regs_reg_adc0_reg1_shift, \
              rf_adc0_regs_reg_adc0_reg1_mask, \
              0x1A);
    SET_VALUE(adc0Val, \
              rf_adc0_regs_reg_adc0_reg2_shift, \
              rf_adc0_regs_reg_adc0_reg2_mask, \
              0x1A);
    
    // Note: You need to turn off the adc clock first, turn on the adc, and then turn on the adc clock.
    CLEAR_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_pll_en_ckadc_bit);
    delayUs(100);// Note: pending effectiveness, PMU work needs to be delayed.
    
    SET_VALUE(RFC->rf_adc0_regs_reg, 0, ((1 << 23) - 1), adc0Val);
    SET_VALUE(RFC->rf_adc1_regs_reg, 0, ((1 << 19) - 1), adc1Val);
    
    SET_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_pll_en_ckadc_bit);
    delayUs(100);// Note: pending effectiveness, PMU work needs to be delayed.

    setRefBuffForADC1(0x2);
    /* NC */
    SET_VALUE(RFC->rf_atb_cfg_reg, \
              rf_atb_cfg_reg_atb_cfg1_shift, \
              rf_atb_cfg_reg_atb_cfg1_mask, \
              0x0);
    
    setRefBuffForADC0(0x7);
    setInputCoupledModeForADC0(INPUT_DC_COUPLED_MODE);
}


/**
 * @brief   adc bias setting.
 * @param   adc0Bias0~adc0Bias5:
 * @param   adc1Bias0~adc1Bias5:
 * @return  None.
 */
void adcBiasSetting(uint8 adc0Bias0, uint8 adc0Bias1, uint8 adc0Bias2, uint8 adc0Bias3,
                    uint8 adc0Bias4, uint8 adc0Bias5, uint8 adc1Bias0, uint8 adc1Bias1,
                    uint8 adc1Bias2, uint8 adc1Bias3, uint8 adc1Bias4, uint8 adc1Bias5)
{
    SET_VALUE(RFC->rf_bias2_dina_reg, rf_bias2_dina_reg_bias2_din4_shift, \
              rf_bias2_dina_reg_bias2_din4_mask, adc0Bias0);
    SET_VALUE(RFC->rf_bias2_dina_reg, rf_bias2_dina_reg_bias2_din5_shift, \
              rf_bias2_dina_reg_bias2_din5_mask, adc0Bias1);
    SET_VALUE(RFC->rf_bias2_dina_reg, rf_bias2_dina_reg_bias2_din6_shift, \
              rf_bias2_dina_reg_bias2_din6_mask, adc0Bias2);
    SET_VALUE(RFC->rf_bias2_dina_reg, rf_bias2_dina_reg_bias2_din7_shift, \
              rf_bias2_dina_reg_bias2_din7_mask, adc0Bias3);

    SET_VALUE(RFC->rf_bias2_dinb_reg, rf_bias2_dinb_reg_bias2_din8_shift, \
              rf_bias2_dinb_reg_bias2_din8_mask, adc1Bias0);
    SET_VALUE(RFC->rf_bias2_dinb_reg, rf_bias2_dinb_reg_bias2_din9_shift, \
              rf_bias2_dinb_reg_bias2_din9_mask, adc1Bias1);
    SET_VALUE(RFC->rf_bias2_dinb_reg, rf_bias2_dinb_reg_bias2_din10_shift, \
              rf_bias2_dinb_reg_bias2_din10_mask, adc1Bias2);
    SET_VALUE(RFC->rf_bias2_dinb_reg, rf_bias2_dinb_reg_bias2_din11_shift, \
              rf_bias2_dinb_reg_bias2_din11_mask, adc1Bias3);
}


/**
 * @brief   adc sample init, ch0 enable and ch1 enable.
 * @param   ch0En: adc0 enable/disable, see also @ref stateTypedef.
 * @param   ch1En: adc1 enable/disable, see also @ref stateTypedef.
 * @param   offset: Set to 1 to remove offset automatically, otherwise use register offsets, see also @ref adcAutoOffsetTypedef.
 * @param   swap: Is the I and Q path of ADC swapped, see also @ref adcChIQSwapTypedef.
 * @return  None.
 */
void setAdcSample(const stateTypedef ch0En, const stateTypedef ch1En,
                  const adcAutoOffsetTypedef offset, const adcChIQSwapTypedef swap)
{
    setAdcCh0AndCh1En(ch0En, ch1En);
    isAdcAutoOffset(offset);
    setAdcIQSwap(swap);
}


/**
 * @brief   correction of ADC sampling problem.
 *          Matlab code: if((floor(data_tmp/64)>31) | (floor(data_tmp/64)<31))
 *                          data_tmp = floor(data_tmp/64)64+mod(data_tmp,64)1.75;
 *                          end
 * @param   addr: Destination address.
 * @param   adcSamples: ADC sampling points.
 * @param   chirpNum: chirp number.
 * @return  None.
 */
void adcDataCorrection(uint32 addr, uint16 adcSamples, uint16 chirpNum)
{
    uint16 *address = (uint16 *)addr;
    int16 data_tmp;
    int32 data32b;
    uint16 temp = adcSamples * chirpNum;
    int16 dataHigh;
    int16 dataLow;
    for(int i = 0; i <= temp; i++)
    {
        data_tmp = *(address + i);
        dataHigh = data_tmp & 0xfc00;
        dataLow = data_tmp & 0x03ff;
        if(dataLow < 0x3FE) //for FIR coef16=0xfff instead of 1024
        {
            data32b = dataHigh + (dataLow << 1);
            if(data32b < 32768)
            {
                *(address + i) = (int16)data32b;
            }
            else
            {
                *(address + i) = 32767;
            }
        }
    }
}


/**
 * @brief   ADC sampling, user selects interval and end sampling.
 * @param   dsModStr: ADC sampling configuration structure, see also @ref dataSampleEndConfigStrTypedef.
 * @param   downSample: ADC sampling rate configuration, see also @ref downSampleRateTypedef.
 * @return  None.
 */
void setDataSampleConfigIntervalAndEnd(const dataSampleEndConfigStrTypedef dsModStr,   \
                                       const downSampleRateTypedef downSample)
{
    uint16 intervalNum  = dsModStr.intervalNum;
    uint16 endNum       = dsModStr.endNum;
    uint16 adcSampleNum = dsModStr.adcSampleNum;
    uint16 scanNum      = dsModStr.scanNum;

    setAdcStartNum(intervalNum - adcSampleNum * (downSample + 1) - endNum);
    setAdcVaildtNum(adcSampleNum * (downSample + 1));
    setAdcIntervalNum(intervalNum);
    setDataSampleScanChirpNum(scanNum);
}


/**
 * @brief   ADC sampling, user selects interval and start sampling.
 * @param   dsModStr: ADC sampling configuration structure, see also @ref dataSampleStartConfigStrTypedef.
 * @param   downSample: ADC sampling rate configuration, see also @ref downSampleRateTypedef.
 * @return  None.
 */
void setDataSampleConfigIntervalAndStart(const dataSampleStartConfigStrTypedef dsModStr,   \
        const downSampleRateTypedef downSample)
{
    uint16 intervalNum  = dsModStr.intervalNum;
    uint16 startNum     = dsModStr.startNum;
    uint16 adcSampleNum = dsModStr.adcSampleNum;
    uint16 scanNum      = dsModStr.scanNum;

    setAdcStartNum(startNum);
    setAdcVaildtNum(adcSampleNum * (downSample + 1));
    setAdcIntervalNum(intervalNum);
    setDataSampleScanChirpNum(scanNum);
}


/**
 * @brief   when FMCW trigger signal as ADC start signal, the funtion is start signal.
 * @param   None.
 * @return  None.
 */
void adcFmcwStart(void)
{
    /* if not in cascade appliaction, use ds_start to start fmcw and adc sampling.
        if in cascade appliaction, and as a master, use sync_out to start fmcw and adc sampling.
        if in cascade appliaction, and as a slave,  wait external sync_in to start fmcw and adc sampling. */
    #ifdef CASCADE_MASTER
    setFmcwSyncOut(SYNC_OUT_1);
    #else
    #ifdef CASCADE_SLAVE
    ;
    #else
    setAdcStart(ADC_START_TRIGGER);
    #endif
    #endif
}


/**
 * @brief   Generate adc data with object information.
 * @param   step     :siggen_step[31:16] :step.
 * @param   chgStep  :siggen_step[15:0] :chg_stp.
 * @param   comp     :siggen_ctrl[17] :comp.
 * @param   siggenEn :siggen_ctrl[16] :en
 * @param   ang      :siggen_ctrl[15:0]:ang.
 * @return  None.
 */
void siggen(uint16 step, uint16 chgStep, uint16 comp, uint16 siggenEn, uint16 ang)
{
    /*RW
    [31:16] step:Phase step for 256x sinewave table. [15:8] ingeter; [7:0] fraction.
                                        For example, [15:8] = 1, [7:0] = 0 will generate a waveform whose frequence is 1/256 of Fadc.

    [15:0] chg_stp:Phase change step. Phase step is increased by this value every sample.
                   Accumulated phase step is reset to original phase step on a adc start.
                   Set to 0 if want constant phase step.
                   Actual phase_step in clock n is phase_step[15:0]*2^16 + phase_chg_step[15:0]*n.
    */
    CLEAR_VALUE(DSP->siggen_step);
    CLEAR_VALUE(DSP->siggen_ctrl);

    SET_VALUE(DSP->siggen_step, \
              siggen_step_chg_stp_shift, \
              siggen_step_chg_stp_mask, \
              chgStep);
    SET_VALUE(DSP->siggen_step, \
              siggen_step_stp_shift, \
              siggen_step_stp_mask, \
              step);
    /*RW
    [17] comp:Data from signal generater is 2's complement, otherwise it's unsigned.
    [16] en:Enable signal generater
    [15:0] ang:Object angler [15:8] ingeter; [7:0] fraction.
           Phase advance of antennant1 comparing with antennant0.
           [15:8] = 32 means 45 (32/256*360) degree advance of ant1.
    */
    if(comp)
    {
        SET_BIT(DSP->siggen_ctrl, siggen_ctrl_comp_bit);
    }
    else
    {
        CLEAR_BIT(DSP->siggen_ctrl, siggen_ctrl_comp_bit);
    }

    if(siggenEn)
    {
        SET_BIT(DSP->siggen_ctrl, siggen_ctrl_en_bit);
    }
    else
    {
        CLEAR_BIT(DSP->siggen_ctrl, siggen_ctrl_en_bit);
    }

    SET_VALUE(DSP->siggen_ctrl, \
              siggen_ctrl_ang_shift, \
              siggen_ctrl_ang_mask, \
              ang);
}


/**
 * @brief   about power.
 * @param   enableSigInput:.
 * @return  None
 */
void gpadcEnable(const uint16 enableSigInput)
{
    CLEAR_VALUE(RFC->rf_bias2_dinc_reg);
    CLEAR_VALUE(RFC->rf_gpadc_reg_reg);

    // enable basebandbias setting
    baseBandBiasSetting(ENABLE);

    // change GPADC BIST 200uA to 50 uA.
    SET_VALUE(RFC->rf_bias2_dinc_reg, \
              rf_bias2_dinc_reg_bias2_din16_shift, \
              rf_bias2_dinc_reg_bias2_din16_mask, \
              0x2);

    // change GPADC_CLK from clk_high/2048 to clk_high/32.
    // by doing this, the first gpadc sampling data needs 0.13ms, and the 2nd data and others data need 46us.
    SET_VALUE(PMU->pmu_clk_high_reg, \
              pmu_clk_high_reg_ctrl_gpadc_clk_sel_shift, \
              pmu_clk_high_reg_ctrl_gpadc_clk_sel_mask, \
              0x5);

    SET_BIT(RFC->rf_gpadc_reg_reg, (1 << BIT_0));  /* gpadc enable. */
    SET_BIT(RFC->rf_gpadc_reg_reg, (1 << BIT_2));  /* enable internal bias, disconnect external bias */
    SET_BIT(RFC->rf_gpadc_reg_reg, (1 << BIT_5));  /* int bias control: 40uA */
    RFC->rf_gpadc_reg_reg |= (1 << (7 + enableSigInput));
}


/**
 * @brief   Get the data of gpadc.
 * @param   None:.
 * @return  return register specific single/multiple bit values.
 */
uint8 getGpadcData(void)
{
    return GET_BITS_VALUE(RFC->rf_gpadc_data_reg, \
                          rf_gpadc_data_reg_gpadc_data_shift, \
                          rf_gpadc_data_reg_gpadc_data_mask);
}


/**
 * @brief   gpadc calibration initialization.
 * @param   None.
 * @return  None.
 * @note
 */
void gpadcCalibrationInit(void)
{
    volatile uint8 gpadcValue;
    volatile int8 gpadcOffset;

    /* enable basebandbias setting */
    baseBandBiasSetting(ENABLE);

    /* change GPADC BIST 200uA to 50 uA. */
    SET_VALUE(RFC->rf_bias2_dinc_reg, \
                rf_bias2_dinc_reg_bias2_din16_shift, \
                rf_bias2_dinc_reg_bias2_din16_mask, \
                0x2);

    /** change GPADC_CLK from clk_high/2048 to clk_high/32. 
     *  by doing this, the first gpadc sampling data needs 0.13ms, and the 2nd data and others data need 46us. 
    */
    SET_VALUE(PMU->pmu_clk_high_reg, \
                pmu_clk_high_reg_ctrl_gpadc_clk_sel_shift, \
                pmu_clk_high_reg_ctrl_gpadc_clk_sel_mask, \
                0x5);
}


/**
 * @brief   Set up GPADC voltage test mode.
 * @param   en: ENABLE/DISABLE.
 * @return  None.
 * @note
 */
void setGpadcVTest(const stateTypedef en)
{
    (ENABLE == en) ? \
        WRITE_REG(RFC->rf_gpadc_reg_reg, 0x800009) : \
        WRITE_REG(RFC->rf_gpadc_reg_reg, 0);


    SET_VALUE(RFC->rf_plli_ldo_cfg_reg, \
                rf_plli_ldo_cfg_reg_gpadc_reg4_shift, \
                rf_plli_ldo_cfg_reg_gpadc_reg4_mask, \
                (en << BIT_3));
}


/**
 * @brief   baseBand Bias Setting, enable signal for ztc bias for BB.
 * @param   bbBiasEn: enable signal for ztc bias for BB..
 * @return  1: Success, 0: Failure.
 */
uint8 baseBandBiasSetting(const stateTypedef bbBiasEn)
{
    uint8 res = 0;
    if(ENABLE == bbBiasEn)
    {
        res = 1;
        SET_BIT(RFC->rf_bias2_regs_reg, rf_bias2_regs_reg_bias2_en_bit);
    }
    else if(DISABLE == bbBiasEn)
    {
        res = 1;
        CLEAR_BIT(RFC->rf_bias2_regs_reg, rf_bias2_regs_reg_bias2_en_bit);
    }

    return res;
}


/**
 * @brief   Fine tune Vref of reference buffer for ADC-1. Default = 0.8V.
 * @param   vrefVal: Vref value.
 * @return  1: Success, 0: Failure.
 */
uint8 setRefBuffForADC1(const uint8 vrefVal)
{
    uint8 res = 0;
    if(vrefVal < 8)
    {
        res = 1;
        SET_VALUE(RFC->rf_atb_cfg_reg, \
                  rf_atb_cfg_reg_atb_cfg2_shift, \
                  rf_atb_cfg_reg_atb_cfg2_mask, \
                  vrefVal);
    }

    return res;
}

/**
 * @brief   Get Vref of reference buffer for ADC-1. Default = 0.8V.
 * @return  return register specific single/multiple bit values.
 */
uint8 getRefBuffForADC1(void)
{
    return GET_BITS_VALUE(RFC->rf_atb_cfg_reg, \
                          rf_atb_cfg_reg_atb_cfg2_shift, \
                          rf_atb_cfg_reg_atb_cfg2_mask);
}


/**
 * @brief   Fine tune Vref of reference buffer for ADC-0. Default = 0.8V.
 * @param   vrefVal: Vref value.
 * @return  1: Success, 0: Failure.
 */
uint8 setRefBuffForADC0(const uint8 vrefVal)
{
    uint8 res = 0;
    if(vrefVal < 8)
    {
        res = 1;
        SET_VALUE(RFC->rf_spare_reg, \
                  rf_spare_reg_ar_spare_reg2_shift, \
                  rf_spare_reg_ar_spare_reg2_mask, \
                  vrefVal);
    }

    return res;
}

/**
 * @brief   Get Vref of reference buffer for ADC-0. Default = 0.8V.
 * @return  return register specific single/multiple bit values.
 */
uint8 getRefBuffForADC0(void)
{
    return GET_BITS_VALUE(RFC->rf_spare_reg, \
                          rf_spare_reg_ar_spare_reg2_shift, \
                          rf_spare_reg_ar_spare_reg2_mask);
}


/**
 * @brief   Enable input ac-coupled bypass for ADC-1.
 * @param   vrefVal: Vref value.
 * @return  1: Success, 0: Failure.
 */
uint8 setInputCoupledModeForADC1(const acCoupledModeTypedef mode)
{
    uint8 res = 0;
    if(NORMAL_OPERTION_MODE == mode)
    {
        res = 1;
        CLEAR_BIT(RFC->rf_spare_reg, (1 << BIT_2));
    }
    else if(INPUT_DC_COUPLED_MODE == mode)
    {
        res = 1;
        SET_BIT(RFC->rf_spare_reg, (1 << BIT_2));
    }

    return res;
}

/**
 * @brief   Get Enable input ac-coupled bypass for ADC-1 status.
 * @return  return register specific single/multiple bit values.
 */
uint8 getInputCoupledModeForADC1(void)
{
    return GET_BIT_VALUE(RFC->rf_spare_reg, BIT_1);
}


/**
 * @brief   Enable input ac-coupled bypass for ADC-0.
 * @param   mode: see also @ref acCoupledModeTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setInputCoupledModeForADC0(const acCoupledModeTypedef mode)
{
    uint8 res = 0;
    if(NORMAL_OPERTION_MODE == mode)
    {
        res = 1;
        CLEAR_BIT(RFC->rf_spare_reg, (1 << BIT_3));
    }
    else if(INPUT_DC_COUPLED_MODE == mode)
    {
        res = 1;
        SET_BIT(RFC->rf_spare_reg, (1 << BIT_3));
    }

    return res;
}

/**
 * @brief   Get Enable input ac-coupled bypass for ADC-0 status.
 * @return  return register specific single/multiple bit values.
 */
uint8 getInputCoupledModeForADC0(void)
{
    return GET_BIT_VALUE(RFC->rf_spare_reg, BIT_0) | \
           (GET_BIT_VALUE(RFC->rf_spare_reg, BIT_1) << 1);
}


/**
 * @brief   Set the specified mode to 20G buffer.
 * @param   mode: see also @ref specModeTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setSpecModeTo20Buff(const specModeTypedef mode)
{
    uint8 res = 0;
    if(SW_PLLINT == mode)
    {
        res = 1;
        CLEAR_BIT(RFC->rf_spare_reg, (1 << BIT_1));
        SET_BIT(RFC->rf_spare_reg, (1 << BIT_0));
    }
    else if(SW_FMCWIN == mode)
    {
        res = 1;
        CLEAR_BIT(RFC->rf_spare_reg, (1 << BIT_0));
        SET_BIT(RFC->rf_spare_reg, (1 << BIT_1));
    }

    return res;
}

/**
 * @brief   Get Enable input ac-coupled bypass for ADC-0 status.
 * @return  return register specific single/multiple bit values.
 */
uint8 getSpecModeTo20Buff(void)
{
    return (GET_BIT_VALUE(RFC->rf_spare_reg, BIT_0) | \
            GET_BIT_VALUE(RFC->rf_spare_reg, BIT_1) << 1);
}


/**
 * @brief   adc clock switch.
 * @param   adcClk: adc clock, see also @ref adcClockTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 adcClkSwitch(const adcClockTypedef adcClk)
{
    uint8 res = 0;

    if(ADC_CLOCK_7P14M == adcClk)
    {
        res = 1;
        /*  Enable divide-by-2 for adc clk output. */
        SET_BIT(RFC->rf_atb_cfg_reg, (1 << BIT_5));
    }

    return res;
}

/**
 * @brief   Set the i enable in channel 0 and channel 1 of adc.
 * @param   en: ENABLE/DISABLE, see also @ref stateTypedef.
 * @param   en: ENABLE/DISABLE, see also @ref stateTypedef.
 * @return  settings success/failure.
 * @note    Two channels must be set simultaneously, otherwise it will cause
 *          "jitter" in the phase of the same chirp!!!
 */
void setAdcCh0AndCh1En(const stateTypedef ch0En, const stateTypedef ch1En)
{
    uint32 regVal = 0;
    if(ENABLE == ch0En)
    {
        regVal |= (ds_adc_enable_ch0_i_en_bit | ds_adc_enable_ch0_q_en_bit);
    }
    else if(DISABLE == ch0En)
    {
        regVal &=  ~(ds_adc_enable_ch0_i_en_bit | ds_adc_enable_ch0_q_en_bit);
    }

    if(ENABLE == ch1En)
    {
        regVal |= (ds_adc_enable_ch1_i_en_bit | ds_adc_enable_ch1_q_en_bit);
    }
    else if(DISABLE == ch1En)
    {
        regVal &=  ~(ds_adc_enable_ch1_i_en_bit | ds_adc_enable_ch1_q_en_bit);
    }

    SET_VALUE(DSP->ds_adc_enable, 0, ((1 << 4) - 1), regVal);
}

/**
 * @brief   Get the i enable in channel 0 and channel 1 of adc.
 * @return  return register specific single/multiple bit values.
 */
uint8 getAdcCh0AndCh1En(void)
{
    return GET_BITS_VALUE(DSP->ds_adc_enable, 0, ((1 << 4) - 1));
}


/**
 * @brief   Whether ADC is automatically offset(Test function).
 * @param   mode: ADC offset mode, see also @ref adcAutoOffsetTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 isAdcAutoOffset(const adcAutoOffsetTypedef mode)
{
    uint8 res = 0;

    if(REGISTER_OFFSET == mode)
    {
        /* Set to 0 register offsets.. */
        res = 1;
        CLEAR_BIT(DSP->ds_adc_format_ctrl, ds_adc_format_ctrl_offset_auto_bit);
    }
    else if(AUTO_OFFSET == mode)
    {
        /* Set to 1 to remove offset automatically. */
        res = 1;
        SET_BIT(DSP->ds_adc_format_ctrl, ds_adc_format_ctrl_offset_auto_bit);
    }

    return res;
}


/**
 * @brief   Set the I/Q phase swap of ADC(Test function).
 * @param   mode: IQ phase swap mode, see also @ref adcChIQSwapTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setAdcIQSwap(const adcChIQSwapTypedef mode)
{
    uint8 res = 0;
    if(((uint8)mode) < 4)
    {
        res = 1;
        SET_VALUE(DSP->ds_adc_format_ctrl, \
                  ds_adc_format_ctrl_iq_swap_shift, \
                  ds_adc_format_ctrl_iq_swap_mask, \
                  (uint8)mode);
    }

    return res;
}

/**
 * @brief   Get the I/Q phase swap of ADC(Test function).
 * @return  return register specific single/multiple bit values.
 */
uint8 getAdcIQSwap(void)
{
    return GET_BITS_VALUE(DSP->ds_adc_format_ctrl, \
                          ds_adc_format_ctrl_iq_swap_shift, \
                          ds_adc_format_ctrl_iq_swap_mask);
}


/**
 * @brief   Set ADC polarity reversal(Test function).
 * @param   mode: IQ phase swap mode, see also @ref adcInvertTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setAdcInvert(const adcInvertTypedef mode)
{
    uint8 res = 0;
    if(((uint8)mode) < 5)
    {
        res = 1;
        SET_VALUE(DSP->ds_adc_format_ctrl, \
                  ds_adc_format_ctrl_dat_invt_shift, \
                  ds_adc_format_ctrl_dat_invt_mask, \
                  (uint8)mode);
    }

    return res;
}

/**
 * @brief   Get ADC polarity reversal(Test function).
 * @return  return register specific single/multiple bit values.
 */
uint8 getAdcInvert(void)
{
    return GET_BITS_VALUE(DSP->ds_adc_format_ctrl, \
                          ds_adc_format_ctrl_dat_invt_shift, \
                          ds_adc_format_ctrl_dat_invt_mask);
}


/**
 * @brief   Set the downsampling coefficient of ADC.
 * @param   div: Downsampling coefficient, see also @ref downSampleRateTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setDownSampleRate(const downSampleRateTypedef div)
{
    uint8 res = 0;
    if(((uint8)div) < 17)
    {
        res = 1;
        SET_VALUE(DSP->fir_cof_sample_rate, \
                  fir_cof_sample_rate_down_sample_rate_shift, \
                  fir_cof_sample_rate_down_sample_rate_mask, \
                  (uint8)div);
    }

    return res;
}

/**
 * @brief   Get the downsampling coefficient of ADC.
 * @return  get value.
 */
uint8 getDownSampleRate(void)
{
    return GET_BITS_VALUE(DSP->fir_cof_sample_rate, \
                          fir_cof_sample_rate_down_sample_rate_shift, \
                          fir_cof_sample_rate_down_sample_rate_mask);
}


/**
 * @brief   Whether to bypass downsampling.
 * @param   div: Bypass/Enable downsampling, see also @ref downsampleBypassTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 isDownSampleBypass(downsampleBypassTypedef mode)
{
    uint8 res = 0;

    if(FIR_ENABLE == mode)
    {
        res = 1;
        CLEAR_BIT(DSP->fir_cof_sample_rate, fir_cof_sample_rate_bypass_bit);
        SET_BIT(DSP->fir_grp_dly_select, fir_grp_dly_select_grp_dly_en_bit);
    }
    else if(FIR_BYPASS == mode)
    {
        res = 1;
        SET_BIT(DSP->fir_cof_sample_rate, fir_cof_sample_rate_bypass_bit);
        CLEAR_BIT(DSP->fir_grp_dly_select, fir_grp_dly_select_grp_dly_en_bit);
    }
    return res;
}


/**
 * @brief   Set the coefficient of FIR structure.
 * @param   firStr: coefficient structure, see also @ref firDownsampleStrTypedef.
 * @return  A 32-bit fir filter coefficient must be set simultaneously!!!
 */
void setFirCoef(const firDownsampleStrTypedef firStr)
{
    DSP->fir_reg0  = firStr.firReg0;
    DSP->fir_reg1  = firStr.firReg1;
    DSP->fir_reg2  = firStr.firReg2;
    DSP->fir_reg3  = firStr.firReg3;
    DSP->fir_reg4  = firStr.firReg4;
    DSP->fir_reg5  = firStr.firReg5;
    DSP->fir_reg6  = firStr.firReg6;
    DSP->fir_reg7  = firStr.firReg7;
    DSP->fir_reg8  = firStr.firReg8;
    DSP->fir_reg9  = firStr.firReg9;
    DSP->fir_reg10 = firStr.firReg10;
    DSP->fir_reg11 = firStr.firReg11;
    DSP->fir_reg12 = firStr.firReg12;
    DSP->fir_reg13 = firStr.firReg13;
    DSP->fir_reg14 = firStr.firReg14;
    DSP->fir_reg15 = firStr.firReg15;
}


/**
 * @brief   Set all coefficient of FIR structure.
 * @param   firStr: config fir filter coefficients, see also @ref firDownsampleStrTypedef.
 * @return  None.
 */
void setAllFirCoef(const firDownsampleStrTypedef *firStr)
{
    memcpy(&s_firds.firReg0, firStr, sizeof(firDownsampleStrTypedef));
}

/**
 * @brief   Get all coefficient of FIR structure.
 * @return  coefficient structure.
 */
firDownsampleStrTypedef *getAllFirCoef(void)
{
    return &s_firds;
}

/**
 * @brief   Set the coefficient in the middle of FIR.
 * @param   coef: coefficient.
 * @return  None.
 */
void setFirMidCoef(const uint16 coef)
{
    SET_VALUE(DSP->fir_cof_sample_rate, \
              fir_cof_sample_rate_cof32_shift, \
              fir_cof_sample_rate_cof32_mask, \
              coef);
}

/**
 * @brief   Get the coefficient in the middle of FIR.
 * @return  get value.
 */
uint16 getFirMidCoef(void)
{
    return GET_BITS_VALUE(DSP->fir_cof_sample_rate, \
                          fir_cof_sample_rate_cof32_shift, \
                          fir_cof_sample_rate_cof32_mask);
}



/**
 * @brief   Get number of intervals.
 * @param   t0~t2 : .
 * @return  return number of intervals.
 */
uint16 getIntervalNum(adcClockTypedef adcClk, uint32 t0, uint32 t1, uint32 t2)
{
    uint16 intervalNum;

    if(ADC_CLOCK_7P14M == adcClk)
    {
        intervalNum = ((uint16)floor((t0 + t1 + t2) * 1000 / 140)) << 0;    // the Least Common Multiple for ADC Period(56ns) and FMCW step(8ns) is 56ns.  
    }
            
    return intervalNum;
}

/**
 * @brief   Get number of end.
 * @param   t0~t2 : .
 * @return  return number of end.
 */
uint16 getEndNum(adcClockTypedef adcClk, uint32 t0, uint32 t1, uint32 t2)
{
    uint16 endNum;
    
    if(ADC_CLOCK_7P14M == adcClk)
    {
        endNum = ceil((t1 + t2) * (100.0 / 14));
    }
        
    return endNum;
}

/**
 * @brief   Set the starting sampling points of ADC.
 * @param   startNum: starting sampling points.
 * @return  None.
 */
void setAdcStartNum(const uint16 startNum)
{
    SET_VALUE(DSP->ds_ds_start_num, \
              ds_ds_start_num_start_num_shift, \
              ds_ds_start_num_start_num_mask, \
              startNum);
}

/**
 * @brief   Get the starting sampling points of ADC.
 * @return  starting sampling points.
 */
uint16 getAdcStartNum(void)
{
    return GET_BITS_VALUE(DSP->ds_ds_start_num, \
                          ds_ds_start_num_start_num_shift, \
                          ds_ds_start_num_start_num_mask);
}


/**
 * @brief   Set the vaild points of ADC.
 * @param   startNum: sampling points.
 * @return  .
 */
void setAdcVaildtNum(const uint16 vaildNum)
{
    SET_VALUE(DSP->ds_ds_adc_valid_num, \
              ds_ds_adc_valid_num_adc_valid_num_shift, \
              ds_ds_adc_valid_num_adc_valid_num_mask, \
              vaildNum);
}

/**
 * @brief   Get the vaild points of ADC.
 * @return  vaild sampling points.
 */
uint16 getAdcVaildNum(void)
{
    return GET_BITS_VALUE(DSP->ds_ds_adc_valid_num, \
                          ds_ds_adc_valid_num_adc_valid_num_shift, \
                          ds_ds_adc_valid_num_adc_valid_num_mask);
}


/**
 * @brief   Get the interval points of ADC.
 * @param   intervalNum: interval points.
 */
void setAdcIntervalNum(const uint16 intervalNum)
{
    SET_VALUE(DSP->ds_ds_interval_num, \
              ds_ds_interval_num_interval_num_shift, \
              ds_ds_interval_num_interval_num_mask, \
              intervalNum);
}

/**
 * @brief   Get the vaild points of ADC.
 * @return  return register specific single/multiple bit values..
 */
uint16 getAdcIntervalNum(void)
{
    return GET_BITS_VALUE(DSP->ds_ds_interval_num, \
                          ds_ds_interval_num_interval_num_shift, \
                          ds_ds_interval_num_interval_num_mask);
}


/**
 * @brief   Set the number of chirp scanned by the data sampling module.
 * @param   scanNum: Scan chirp number
 * @return  None.
 */
void setDataSampleScanChirpNum(const uint8 scanNum)
{
    SET_VALUE(DSP->ds_ds_scan_num, \
              ds_ds_scan_num_scan_num_shift, \
              ds_ds_scan_num_scan_num_mask, \
              scanNum);
}

/**
 * @brief   Get the number of chirp scanned by the data sampling module.
 * @return  return register specific single/multiple bit values..
 */
uint8 getDataSampleScanChirpNum(void)
{
    return GET_BITS_VALUE(DSP->ds_ds_scan_num, \
                          ds_ds_scan_num_scan_num_shift, \
                          ds_ds_scan_num_scan_num_mask);
}


/**
 * @brief   Set the mode of dma transporting ADC data.
 * @param   select: transporting mode, see also @ref adcDmaSelTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setAdcDmaSelect(const adcDmaSelTypedef select)
{
    uint8 res = 0;
    if(select < 7)
    {
        res = 1;
        SET_VALUE(DSP->ds_adc_dma_src_select, \
                  ds_adc_dma_src_select_adc_dma_select_shift, \
                  ds_adc_dma_src_select_adc_dma_select_mask, \
                  (uint8)select);
    }
    return res;
}
/**
 * @brief   Get the mode of dma transporting ADC data.
 * @return  return register specific single/multiple bit values.
 */
uint8 getAdcDmaSelect(void)
{
    return GET_BITS_VALUE(DSP->ds_adc_dma_src_select, \
                          ds_adc_dma_src_select_adc_dma_select_shift, \
                          ds_adc_dma_src_select_adc_dma_select_mask);
}


/**
 * @brief   Set FMCW synchronous output signal.
 * @param   out: out 0 or out 1, see also @ref syncOutTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setFmcwSyncOut(const syncOutTypedef out)
{
    uint8 res = 0;

    if(SYNC_OUT_0 == out)
    {
        res = 1;
        CLEAR_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_sync_out_bit);
    }
    else if(SYNC_OUT_1 == out)
    {
        res = 1;
        SET_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_sync_out_bit);
    }
    return res;
}

/**
 * @brief   Get FMCW synchronous output signal.
 * @return  return register specific single/multiple bit values.
 */
uint8 getFmcwSyncOut(void)
{
    return GET_BIT_VALUE(RFC->fmcw_fmcw_sync, BIT_0);
}


/**
 * @brief   Set FMCW synchronous output signal clock edge trigger.
 * @param   edge: negative/positive edge, see also @ref syncOutEdgeTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setSyncOutClockEdge(const syncOutEdgeTypedef edge)
{
    uint8 res = 0;

    if(PLLI50M_NEGATIVE_EDGE == edge)
    {
        res = 1;
        CLEAR_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_negedge_sel_bit);
    }
    else if(PLLI50M_POSITIVE_EDGE == edge)
    {
        res = 1;
        SET_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_negedge_sel_bit);
    }
    return res;
}

/**
 * @brief   Get FMCW synchronous output signal clock edge trigger.
 * @return  return register specific single/multiple bit values.
 */
uint8 getSyncOutClockEdge(void)
{
    return GET_BIT_VALUE(RFC->fmcw_fmcw_sync, BIT_1);
}


/**
 * @brief   Set whether the app supports cascade.
 * @param   mode: support/no support, see also @ref cascadeModeTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setCascadeApp(const cascadeModeTypedef mode)
{
    uint8 res = 0;

    if(NORMAL_APP == mode)
    {
        res = 1;
        CLEAR_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_en_ckadc_from_syncin_bit);
    }
    else if(CASCADE_APP == mode)
    {
        res = 1;
        SET_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_en_ckadc_from_syncin_bit);
    }
    return res;
}

/**
 * @brief   Get whether the app supports cascade.
 * @return  return register specific single/multiple bit values.
 */
uint8 getCascadeApp(void)
{
    return GET_BIT_VALUE(RFC->fmcw_fmcw_sync, BIT_2);
}


/**
 * @brief   Set the mode of FMCW synchronous input signal.
 * @param   mode: support/no support, see also @ref cascadeModeTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setCkadcFromSyncin(const cascadeModeTypedef mode)
{
    uint8 res = 0;

    if(NORMAL_APP == mode)
    {
        res = 1;
        CLEAR_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_en_ckadc_from_syncin_bit);
    }
    else if(CASCADE_APP == mode)
    {
        res = 1;
        SET_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_en_ckadc_from_syncin_bit);
    }
    return res;
}

/**
 * @brief   Get the mode of FMCW synchronous input signal.
 * @return  return register specific single/multiple bit values.
 */
uint8 getCkadcFromSyncin(void)
{
    return GET_BIT_VALUE(RFC->fmcw_fmcw_sync, BIT_2);
}


/**
 * @brief   Set the starting mode of adc from the synchronous input signal.
 * @param   mode: support/no support, see also @ref cascadeModeTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setAdcStartFromSyncin(const cascadeModeTypedef mode)
{
    uint8 res = 0;

    if(NORMAL_APP == mode)
    {
        res = 1;
        CLEAR_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_adc_start_from_syncin_bit);
    }
    else if(CASCADE_APP == mode)
    {
        res = 1;
        SET_BIT(RFC->fmcw_fmcw_sync, fmcw_fmcw_sync_adc_start_from_syncin_bit);
    }
    return res;
}

/**
 * @brief   Get the starting mode of adc from the synchronous input signal.
 * @return  return register specific single/multiple bit values.
 */
uint8 getAdcStartFromSyncin(void)
{
    return GET_BIT_VALUE(RFC->fmcw_fmcw_sync, BIT_3);
}


/**
 * @brief   ADC begins to trigger.
 * @param   mode: trigger/no trigger, see also @ref adcStartTypedef.
 * @return  1: Success, 0: Failure.
 */
uint8 setAdcStart(const adcStartTypedef mode)
{
    uint8 res = 0;

    if(ADC_START_NO_TRIGGER == mode)
    {
        res = 1;
        CLEAR_BIT(DSP->ds_start, ds_start_adc_start_bit);
    }
    else if(ADC_START_TRIGGER == mode)
    {
        res = 1;
        SET_BIT(DSP->ds_start, ds_start_adc_start_bit);
    }
    return res;
}

/**
 * @brief   Get the adc begins to trigger.
 * @return  return register specific single/multiple bit values.
 */
uint8 getAdcStart(void)
{
    return GET_BIT_VALUE(DSP->ds_start, BIT_0);
}


/**
 * @brief   Set ADC testing mode.
 * @param   en: ENABLE/DISABLE, see also @ref stateTypedef.
 * @return  .
 */
void setAdcTestMode(const stateTypedef en)
{
    if(ENABLE == en)
    {
        SET_BIT(RFC->rfc_vga0_reg, (1 << BIT_10));
    }
    else if(DISABLE == en)
    {
        CLEAR_BIT(RFC->rfc_vga0_reg, (1 << BIT_10));
    }
}

/**
 * @brief   Get ADC testing mode.
 * @return  0: disable, 1: enable.
 */
uint8 getAdcTestMode(void)
{
    return GET_BIT_VALUE(RFC->rfc_vga0_reg, BIT_10);
}
