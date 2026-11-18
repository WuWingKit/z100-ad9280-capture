/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_pmu.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file contains all the functions implemented by the PMU peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_pmu.h"
#include "adt6101P_gpio.h"
/* Private functions ---------------------------------------------------------*/
/**
 * @brief   Clear all wakeup singals.
 * @return  None.
 * @note    If the "reserve" bit is not cleared, the sleep mode current will be high, increasing
 *          from 300uA to 10mA!!!
 */
void clearAllWakeUpEn(void)
{
    CLEAR_VALUE(PMU->pmu_int_en_reg);
}


/**
 * @brief   Enable IO wake-up mode.
 * @param   en: ENABLE/DISABLE, see also @ref stateTypedef.
 * @return  None.
 */
void setIoWakeUpEn(const stateTypedef en)
{
    if(ENABLE == en)
    {
        SET_BIT(PMU->pmu_int_en_reg, pmu_int_en_reg_ctrl_io_wakeup_en_bit);
    }
    else if(DISABLE == en)
    {
        CLEAR_BIT(PMU->pmu_int_en_reg, pmu_int_en_reg_ctrl_io_wakeup_en_bit);
    }
}

/**
 * @brief   Get IO wake-up mode status.
 * @return  1: Enable. 0: Disable.
 */
uint8 getIoWakeUpEn(void)
{
    return GET_BIT_VALUE(PMU->pmu_int_en_reg, BIT_0);
}


/**
 * @brief   Enable RTC timer wake-up mode.
 * @param   en: ENABLE/DISABLE, see also @ref stateTypedef.
 * @return  None.
 */
void setRtcTimerWakeUpEn(const stateTypedef en)
{
    if(ENABLE == en)
    {
        SET_BIT(PMU->pmu_int_en_reg, pmu_int_en_reg_ctrl_rtc_wakeup_en_bit);
    }
    else if(DISABLE == en)
    {
        CLEAR_BIT(PMU->pmu_int_en_reg, pmu_int_en_reg_ctrl_rtc_wakeup_en_bit);
    }
}

/**
 * @brief   Get RTC timer wake-up mode status.
 * @return  1: Enable. 0: Disable.
 */
uint8 getRtcTimerWakeUpEn(void)
{
    return GET_BIT_VALUE(PMU->pmu_int_en_reg, BIT_1);
}


/**
 * @brief   Enable debug wake-up mode.
 * @param   en: ENABLE/DISABLE, see also @ref stateTypedef.
 * @return  None.
 */
void setDebugWakeUpEn(const stateTypedef en)
{
    if(ENABLE == en)
    {
        SET_BIT(PMU->pmu_int_en_reg, pmu_int_en_reg_ctrl_rtc_wakeup_en_bit);
    }
    else if(DISABLE == en)
    {
        CLEAR_BIT(PMU->pmu_int_en_reg, pmu_int_en_reg_ctrl_rtc_wakeup_en_bit);
    }
}

/**
 * @brief   Get debug wake-up mode status.
 * @return  1: Enable. 0: Disable.
 */
uint8 getDebugWakeUpEn(void)
{
    return GET_BIT_VALUE(PMU->pmu_int_en_reg, BIT_2);
}


/**
 * @brief   Set the powering down time.
 * @param   val: (powering_down_time+1) for powering down stage, @32k cycle.
 * @note    from clk_en=0 to iso_n=0 and ldo_on=0.
 */
void setPoweringDownTime(uint16 val)
{
    SET_VALUE(PMU->pmu_pmu_stage_time, \
              pmu_pmu_stage_time_powering_down_time_shift, \
              pmu_pmu_stage_time_powering_down_time_mask, \
              val);
}

/**
 * @brief   Get the powering down time.
 * @return  (powering_down_time+1) for powering down stage, @32k cycle..
 */
uint16 getPoweringDownTime(void)
{
    return GET_BITS_VALUE(PMU->pmu_pmu_stage_time, \
                          pmu_pmu_stage_time_powering_down_time_shift, \
                          pmu_pmu_stage_time_powering_down_time_mask);
}


/**
 * @brief   Set the powering up time.
 * @param   val: (powering_up_time+1) for powering up stage, @32k cycle.
 * @return  None.
 * @note    from ldo_on=1 to clk_en=1.
 *          recommend value is 0x2a, almost 1.5ms.
 *          the total wake up time from wake_up signal to MCU running is (powering_up_time + 4) * T_clk_32k.
 *          for example 0x2a, means (0x2a + 4)*32us = 1.5ms approximately.
 */
void setPoweringUpTime(uint16 val)
{
    SET_VALUE(PMU->pmu_pmu_stage_time, \
              pmu_pmu_stage_time_powering_up_time_shift, \
              pmu_pmu_stage_time_powering_up_time_mask, \
              val);
}

/**
 * @brief   Get the powering up time.
 * @return  (powering_up_time+1) for powering up stage, @32k cycle.
 */
uint16 getPoweringUpTime(void)
{
    return GET_BITS_VALUE(PMU->pmu_pmu_stage_time, \
                          pmu_pmu_stage_time_powering_up_time_shift, \
                          pmu_pmu_stage_time_powering_up_time_mask);
}


/**
 * @brief   Set the 32k clock soruce.
 * @param   sel: select clk_32k source.
 * @return  None.
 */
void set32kClkSrc(const clk32kSrcTypedef sel)
{
    SET_VALUE(PMU->pmu_clk_high_reg, \
              pmu_clk_high_reg_ctrl_32k_clk_sel_shift, \
              pmu_clk_high_reg_ctrl_32k_clk_sel_mask, \
              (uint8)sel);
}

/**
 * @brief   Get the 32k clock soruce.
 * @return  clk_high source.
 */
uint8 get32kClkSrc(void)
{
    return GET_BITS_VALUE(PMU->pmu_clk_high_reg, \
                          pmu_clk_high_reg_ctrl_32k_clk_sel_shift, \
                          pmu_clk_high_reg_ctrl_32k_clk_sel_mask);
}


/**
 * @brief   Set the can clock soruce.
 * @param   sel: select can clock source.
 * @return  None.
 */
void setCanClkDiv(const canClockTypedef sel)
{
    SET_VALUE(PMU->pmu_clk_high_reg, \
              pmu_clk_high_reg_ctrl_can_clk_sel_shift, \
              pmu_clk_high_reg_ctrl_can_clk_sel_mask, \
              (uint8)sel);
}

/**
 * @brief   Get the can clock soruce.
 * @return  can clock source.
 */
uint8 getCanClkDiv(void)
{
    return GET_BITS_VALUE(PMU->pmu_clk_high_reg, \
                          pmu_clk_high_reg_ctrl_can_clk_sel_shift, \
                          pmu_clk_high_reg_ctrl_can_clk_sel_mask);
}

/**
 * @brief   Set the bias of XO50M core and outbuf.
 * @param   val: <3:0>:xo50M_core_bias<3:0> Oscillator core current selection, 0~1.125mA, 75uA/bit
 *               <7:4>: xo50M_outbuf_bias<3:0> Reference clk buffer current selection, 0~375uA, 25uA/bit.
 * @return  None.
 */
void setXo50MBiasAndCap(const uint8 bias, const uint8 cap)
{
    SET_VALUE(RFC->rfc_xo50m_cfg_reg, \
              rfc_xo50m_cfg_reg_xo50m_reg2_shift, \
              rfc_xo50m_cfg_reg_xo50m_reg2_mask,  \
              bias);

    SET_VALUE(RFC->rfc_xo50m_cfg_reg, \
              rfc_xo50m_cfg_reg_xo50m_reg1_shift, \
              rfc_xo50m_cfg_reg_xo50m_reg1_mask,  \
              cap);
}

/**
 * @brief   Get the bias of XO50M core and outbuf.
 * @return  <3:0>:xo50M_core_bias<3:0> Oscillator core current selection, 0~1.125mA, 75uA/bit
 *          <7:4>: xo50M_outbuf_bias<3:0> Reference clk buffer current selection, 0~375uA, 25uA/bit..
 */
uint8 getXo50MBias(void)
{
    return GET_BITS_VALUE(RFC->rfc_xo50m_cfg_reg,   \
                          rfc_xo50m_cfg_reg_xo50m_reg2_shift, \
                          rfc_xo50m_cfg_reg_xo50m_reg2_mask);
}

/**
 * @brief   Getting of internal cap array of crystal oscillator.
 * @return  single-end cap value: 640f~10.24pF, 640fF/bit.
 */
uint8 getXo50MCap(void)
{
    return GET_BITS_VALUE(RFC->rfc_xo50m_cfg_reg,   \
                          rfc_xo50m_cfg_reg_xo50m_reg1_shift, \
                          rfc_xo50m_cfg_reg_xo50m_reg1_mask);
}


void setSlp(const uint32 xo50M, const uint32 plliCtrl, const uint32 ldoBg)
{
    PMU->pmu_slp_xo50m_cfg      = xo50M;
    PMU->pmu_slp_plli_ctrl_reg  = plliCtrl;
    PMU->pmu_slp_ldo_bg_reg_reg = ldoBg;
}


/**
 * @brief   Get xo50m in sleep mode.
 * @return  .
 */
uint32 getSlpXo50M(void)
{
    return PMU->pmu_slp_xo50m_cfg;
}

/**
 * @brief   Get PLLI control in sleep mode.
 * @return  .
 */
uint32 getSlpPlliCtrl(void)
{
    return PMU->pmu_slp_plli_ctrl_reg;
}

/**
 * @brief   Get LDO control in sleep mode.
 * @return  .
 */
uint32 getSlpLdoBg(void)
{
    return PMU->pmu_slp_ldo_bg_reg_reg;
}


/**
 * @brief   Set power control in sleep mode.
 * @param   val: .
 * @return  None.
 */
void setSlpPowerCtrl(const uint8 val)
{
    PMU->pmu_slp_power_ctrl_reg = val;
}

/**
 * @brief   Get power control in sleep mode.
 * @return  .
 */
uint8 getSlpPowerCtrl(void)
{
    return PMU->pmu_slp_power_ctrl_reg;
}


/**
 * @brief   Adjust the output of low-power LDO, from 0.7 to 1.2v, only valid when powering_up stage,
 *          or setting pmu's slp_nxt_ctrl_disable = 1.
 * @param   voltage: 0.7v~1.3v, see also @ref ldo12LPVolTypdef.
 * @return
 * @note    voltage control for LDO12 low power mode when in sleep mode.
 */
void setLdoLpVol(const ldo12LPVolTypdef voltage)
{
    SET_VALUE(PMU->pmu_slp_ldo12n_rega_reg, \
              pmu_slp_ldo12n_rega_reg_slp_ldo12_lp_vcfg_shift, \
              pmu_slp_ldo12n_rega_reg_slp_ldo12_lp_vcfg_mask, \
              (uint8)voltage);
}

/**
 * @brief   Get the output of low-power LDO, from 0.7 to 1.2v, only valid when powering_up stage,
 *          or setting pmu's slp_nxt_ctrl_disable = 1.
 * @return  voltage control for LDO12 low power mode when in sleep mode.
 */
uint8 getLdoLpVol(void)
{
    return GET_BITS_VALUE(PMU->pmu_slp_ldo12n_rega_reg, \
                          pmu_slp_ldo12n_rega_reg_slp_ldo12_lp_vcfg_shift, \
                          pmu_slp_ldo12n_rega_reg_slp_ldo12_lp_vcfg_mask);
}


/**
 * @brief   Set the PLLI bias.
 * @param   val: all bias.
 * @return  None.
 */
void setPlliDaca(const uint32 val)
{
    RFC->rf_plli_daca_reg = val;
}

/**
 * @brief   Get the PLLI bias.
 * @param   val: .
 * @return  all bias.
 */
uint32 getPlliDaca(void)
{
    return RFC->rf_plli_daca_reg;
}

/**
 * @brief   Set the PLLI bias.
 * @param   val: all bias.
 * @return  None.
 */
void setPlliDacb(const uint32 val)
{
    RFC->rf_plli_dacb_reg = val;
}

/**
 * @brief   Get the PLLI bias.
 * @param   val: .
 * @return  all bias.
 */
uint32 getPlliDacb(void)
{
    return RFC->rf_plli_dacb_reg;
}


/**
 * @brief   Select AHB clock, based on clk_high.
 * @param   clock: AHB clock, CLK_HIGH ~ CLK_HIGH_DIV_64, see also @ref AHBClockTypedef.
 * @return  None.
 * @note    This changing need 16 clk_high cycles to finish.
 */
void setAHBClock(const AHBClockTypedef clock)
{
    SET_VALUE(PMU->pmu_clk_high_reg, \
              pmu_clk_high_reg_ctrl_ahb_clk_sel_shift, \
              pmu_clk_high_reg_ctrl_ahb_clk_sel_mask, \
              (uint8)clock);
}

/**
 * @brief   Get AHB clock, based on clk_high.
 * @return  AHB clock, 0 ~ 6.
 */
uint8 getAHBClock(void)
{
    return GET_BITS_VALUE(PMU->pmu_clk_high_reg, \
                          pmu_clk_high_reg_ctrl_ahb_clk_sel_shift, \
                          pmu_clk_high_reg_ctrl_ahb_clk_sel_mask);
}


/**
 * @brief   Manual control to LDO and Clock, such as XO and PLL.
 * @param   en: ENABLE, so those signals are controlled by MCU and software.
 *              DISABLE, and those signals are controlled by PMU automatically,
 *              see also @ref stateTypedef.
 * @return  None.
 * @note    It's ONLY for test purpose. The recommended value is 1'h0.
 */
void setManualCtrlToLdoAndClk(const stateTypedef en)
{
    if(ENABLE == en)
    {
        SET_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_manu_ctrl_en_bit);
    }
    else if(DISABLE == en)
    {
        CLEAR_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_manu_ctrl_en_bit);
    }
}

/**
 * @brief   Get Manual control to LDO and Clock, such as XO and PLL status.
 * @return  1: ENABLE, so those signals are controlled by MCU and software.
 *          0: DISABLE, and those signals are controlled by PMU automatically.
 * @note    It's ONLY for test purpose. The recommended value is 1'h0.
 */
uint8 getManualCtrlToLdoAndClk(void)
{
    return GET_BIT_VALUE(PMU->pmu_pmu_ctrl_reg, BIT_4);
}


/**
 * @brief   Set the high power LDO12 power down.
 * @param   state: POWER_ON/POWER_DOWN, see also @ref powerStateTypedef.
 * @return  None.
 */
void setLdo12PowerDown(const powerStateTypedef state)
{
    if(POWER_ON == state)
    {
        CLEAR_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_ctrl_ldo12_pd_bit);
    }
    else if(POWER_DOWN == state)
    {
        SET_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_ctrl_ldo12_pd_bit);
    }
}

/**
 * @brief   Get the high power LDO12 power down status.
 * @return  1: POWER_ON. 0: POWER_DOWN.
 */
uint8 getLdo12PowerDown(void)
{
    return GET_BIT_VALUE(PMU->pmu_pmu_ctrl_reg, BIT_7);
}


/**
 * @brief   Set the PMU current status.
 * @param   state: PMU current states, see also @ref pmuStateTypedef.
 * @return  None.
 */
void setPmuStatus(const pmuStateTypedef state)
{
    SET_VALUE(PMU->pmu_pmu_state, \
              pmu_pmu_state_cur_state_shift, \
              pmu_pmu_state_cur_state_mask, \
              (uint8)state);
}

/**
 * @brief   Set the PMU current status.
 * @return  PMU current states..
 */
uint8 getPmuStatus(void)
{
    return GET_BITS_VALUE(PMU->pmu_pmu_state, \
                          pmu_pmu_state_cur_state_shift, \
                          pmu_pmu_state_cur_state_mask);
}


/**
 * @brief   Set external clock gate en for each apb/ahb reg intf.
 * @param   en: ENABLE, so the clock is controlled by local_clk_en, which is an interanal state machine.
 *              DISABLE, so the clock is free runing, see also @ref stateTypedef.
 * @param   clkGating: clock gating, see also @ref extClkTypedef.
 * @return  None.
 */
void setExtClkEn(const stateTypedef en, const extClkTypedef clkGating)
{
    if(ENABLE == en)
    {
        SET_BIT(PMU->pmu_reg_ext_clk_en_reg, (1 << (uint8)clkGating));
    }
    else if(DISABLE == en)
    {
        CLEAR_BIT(PMU->pmu_reg_ext_clk_en_reg, (1 << (uint8)clkGating));
    }
}

/**
 * @brief   Get external clock gate en for each apb/ahb reg intf status.
 * @param   clkGating: clock gating, see also @ref extClkTypedef.
 * @return  0, so the clock is controlled by local_clk_en, which is an interanal state machine.
 *          1, so the clock is free runing.
 */
uint8 getExtClkEn(const extClkTypedef clkGating)
{
    return GET_BIT_VALUE(PMU->pmu_reg_ext_clk_en_reg, (uint8)clkGating);
}


/**
 * @brief   Set the clock gate valid with 3 cycle delay. 3@16mhz or 3@32khz.
 * @param   en: ENABLE, Only for test purpose. 1'h0 is recommended value.
 *              DISABLE, see also @ref stateTypedef.
 * @param   ctrl: the clock gate, see also @ref ctrlClkTypedef.
 * @return  None.
 */
void setClkGateEn(const stateTypedef en, const ctrlClkTypedef ctrl)
{
    if(ENABLE == en)
    {
        SET_BIT(PMU->pmu_clk_gate_reg, (1 << (uint8)ctrl));
    }
    else if(DISABLE == en)
    {
        CLEAR_BIT(PMU->pmu_clk_gate_reg, (1 << (uint8)ctrl));
    }
}

/**
 * @brief   Get the clock gate valid with 3 cycle delay. 3@16mhz or 3@32khz status.
 * @param   ctrl: the clock gate, see also @ref ctrlClkTypedef.
 * @return  1, Only for test purpose. 1'h0 is recommended value.
 *          0, DSIABLE.
 */
uint8 getClkGateEn(const ctrlClkTypedef ctrl)
{
    return GET_BIT_VALUE(PMU->pmu_clk_gate_reg, (uint8)ctrl);
}


/**
 * @brief   reset except cpu/ahb/apb.
 * @param   en: ENABLE, reset.
 *              DISABLE, no reset, see also @ref stateTypedef.
 * @param   ctrl: the clock gate, see also @ref pmuSoftRstTypedef.
 * @return  None.
 */
void setSoftwareReset(const stateTypedef en, const pmuSoftRstTypedef ctrl)
{
    if(ENABLE == en)
    {
        SET_BIT(PMU->pmu_soft_reset_reg, (1 << (uint8)ctrl));
    }
    else if(DISABLE == en)
    {
        CLEAR_BIT(PMU->pmu_soft_reset_reg, (1 << (uint8)ctrl));
    }
}

/**
 * @brief   Get reset except cpu/ahb/apb.
 * @return  the clock gate, see also @ref pmuSoftRstTypedef.
 */
uint8 getSoftwareReset(const pmuSoftRstTypedef ctrl)
{
    return GET_BIT_VALUE(PMU->pmu_soft_reset_reg, (uint8)ctrl);
}


/**
 * @brief   Coefficent N of RC32K calibration filter,
 *          set smaller N to get result fast but coarser, set larger N to get result slower but fine.
 * @param   val: if change the value, then filter will be initialized, and calculate again with new coefficent value.
 *                  y = y*(2^N-1)/2^N + x/2^N ;
 * @return  None.
 */
void setRc32kCailFilter(const uint8 val)
{
    SET_VALUE(PMU->pmu_rc32k_calibration_reg, \
              pmu_rc32k_calibration_reg_ctrl_rc32k_cali_alpha_shift, \
              pmu_rc32k_calibration_reg_ctrl_rc32k_cali_alpha_mask, \
              val);
}

/**
 * @brief   Get coefficent N of RC32K calibration filter,
 * @return  coefficent N.
 */
uint8 getRc32kCailFilter(void)
{
    return GET_BITS_VALUE(PMU->pmu_rc32k_calibration_reg, \
                          pmu_rc32k_calibration_reg_ctrl_rc32k_cali_alpha_shift, \
                          pmu_rc32k_calibration_reg_ctrl_rc32k_cali_alpha_mask);
}


/**
 * @brief   Set the RC32K frequency calibration value.
 * @param   val: frequency calibration.
 * @return  None.
 */
void setRc32kCail(const uint8 val)
{
    SET_VALUE(RFC->rf_rc32k_reg_reg, \
              rf_rc32k_reg_reg_rc32k_reg_shift, \
              rf_rc32k_reg_reg_rc32k_reg_mask, \
              val);
}

/**
 * @brief   Get the RC32K frequency calibration value.
 * @return  frequency calibration.
 */
uint8 getRc32kCail(void)
{
    return GET_BITS_VALUE(RFC->rf_rc32k_reg_reg, \
                          rf_rc32k_reg_reg_rc32k_reg_shift, \
                          rf_rc32k_reg_reg_rc32k_reg_mask);
}


/**
 * @brief   por_test output.
 * @param   sel: see also @ref selPorTypedef.
 *              POR_INPUT: por_int output.
 *              POR_OUTPUT: por output.
 * @return  None.
 */
void setPorTest(const selPorTypedef sel)
{
    if(POR_INPUT == sel)
    {
        SET_BIT(RFC->rf_bias1_en_reg, (1 << BIT_11));
    }
    else if(POR_OUTPUT == sel)
    {
        CLEAR_BIT(RFC->rf_bias1_en_reg, (1 << BIT_11));
    }
}

/**
 * @brief   Get por_test output.
 * @return  0: por_int output.
 *          1: por output.
 */
uint8 getPorTest(void)
{
    return GET_BIT_VALUE(RFC->rf_bias1_en_reg, BIT_11);
}


/**
 * @brief   rc 32KHz osc output to gpio30.
 * @param   None.
 * @return  None.
 */
void rc32kOutputP30(void)
{
    //gpio mux to output
    setGpioConfigMode(GPIO_CFG_P30, GPIO_MODE_OUTPUT);
    //RFC->rf_rc32k_reg_reg = 0x0A;
}


