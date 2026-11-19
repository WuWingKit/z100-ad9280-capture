/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_rx.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file contains all the functions implemented by the RX peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P_rx.h"
#include "pmu_ctype_map.h"
#include "rfc_ctype_map.h"
#include "dsp_ctype_map.h"


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   control rx lna enable.
 * @param   lna0Enable: rx0 enable, ENABLE/DISABLE.
 * @param   lna1Enable: rx1 enable, ENABLE/DISABLE.
 * @return  None.
 */
void rxLnaEnable(const stateTypedef lna0Enable, const stateTypedef lna1Enable)
{
    if(ENABLE == lna0Enable)
    {
        SET_BIT(RFC->rfc_rx_lna_reg, rfc_rx_lna_reg_lna0_en_bit);
    }
    else if(DISABLE == lna0Enable)
    {
        CLEAR_BIT(RFC->rfc_rx_lna_reg, rfc_rx_lna_reg_lna0_en_bit);
    }

    if(ENABLE == lna1Enable)
    {
        SET_BIT(RFC->rfc_rx_lna_reg, rfc_rx_lna_reg_lna1_en_bit);
    }
    else if(DISABLE == lna1Enable)
    {
        CLEAR_BIT(RFC->rfc_rx_lna_reg, rfc_rx_lna_reg_lna1_en_bit);
    }
    
    /* control LNA gain stability */
    //rf_plln_daca_reg: 0x4000319c
    uint32 regVel = 0;
    regVel = RFC->rf_plln_daca_reg;

    //the expected rf_plln_data_reg<23:16>=8'hB6, while default rf_plln_data_reg<23:16>=8'hA8
    //So clear rf_plln_data_reg<23:16> before set the expected value 8'hB6

    if((lna0Enable | lna1Enable) == ENABLE)
    {
        regVel &= ~(0xFF << rf_plln_daca_reg_lna_stab_ctrl_shift);
        regVel |= (0xB6 << rf_plln_daca_reg_lna_stab_ctrl_shift);
    }
    else
    {
        regVel &= ~(0xB6 << rf_plln_daca_reg_lna_stab_ctrl_shift);
    }

    RFC->rf_plln_daca_reg = regVel;
}


/**
 * @brief   control mixer enable.
 * @param   ch0MixerEnable: channel0 Mixer enable, ENABLE/DISABLE.
 * @param   ch1MixerEnable: channel1 Mixer enable, ENABLE/DISABLE.
 * @return  None.
 */
void mixerEnable(const stateTypedef ch0MixerEnable, const stateTypedef ch1MixerEnable)
{
    if(ENABLE == ch0MixerEnable)
    {
        SET_BIT(RFC->rfc_rx_lna_reg, (1 << BIT_4));
    }
    else if(DISABLE == ch1MixerEnable)
    {
        CLEAR_BIT(RFC->rfc_rx_lna_reg, (1 << BIT_4));
    }

    if(ENABLE == ch1MixerEnable)
    {
        SET_BIT(RFC->rfc_rx_lna_reg, (1 << BIT_5));
    }
    else if(DISABLE == ch1MixerEnable)
    {
        CLEAR_BIT(RFC->rfc_rx_lna_reg, (1 << BIT_5));
    }
}


/**
 * @brief   control dac lna bias. selection:
 *              CH0_LNA_BIAS1 = 8,
 *              CH0_LNA_BIAS2 = 12,
 *              CH0_LNA_BIAS3 = 16,
 *              CH1_LNA_BIAS1 = 20,
 *              CH1_LNA_BIAS2 = 24,
 *              CH1_LNA_BIAS3 = 28
 * @param   chxBias: channelx DAC bias1.
 * @param   val: channelx DAC bias1.
 * @return  None.
 */
void lnaDacBias(const lnaBiasChTypedef chxBias, const uint8 val)
{
    SET_VALUE(RFC->rfc_bias1_dina_reg, (uint8)chxBias, rfc_bias1_dina_reg_bias1_din2_mask, val);
}


/**
 * @brief   rf tia enable.
 * @param   tia0enable: enable TIA0, ENABLE/DISABLE.
 * @param   tia1enable: enable TIA1, ENABLE/DISABLE.
 * @return  None.
 */
void rfTiaEnable(const stateTypedef tia0Enable, const stateTypedef tia1Enable)
{
    if(ENABLE == tia0Enable)
    {
        SET_BIT(RFC->rfc_tia0_cfg_reg, rfc_tia0_cfg_reg_tia0_en_bit);
    }
    else if(DISABLE == tia0Enable)
    {
        CLEAR_BIT(RFC->rfc_tia0_cfg_reg, rfc_tia0_cfg_reg_tia0_en_bit);
    }

    if(ENABLE == tia1Enable)
    {
        SET_BIT(RFC->rfc_tia1_cfg_reg, rfc_tia1_cfg_reg_tia1_en_bit);
    }
    else if(DISABLE == tia1Enable)
    {
        CLEAR_BIT(RFC->rfc_tia1_cfg_reg, rfc_tia1_cfg_reg_tia1_en_bit);
    }
}


/**
 * @brief   control tia gain setting. selection:
 *          TIA_GAIN_6DB
 *          TIA_GAIN_12DB
 *          TIA_GAIN_18DB
 * @param   tia0IGain: TIA0I diff gain setting.
 * @param   tia0QGain: TIA0Q diff gain setting.
 * @param   tia1IGain: TIA1I diff gain setting.
 * @param   tia1QGain: TIA1Q diff gain setting.
 * @return  None.
 */
void rfTiaGainSetting(const tiaDiffGainTypedef tia0IGain, const tiaDiffGainTypedef tia0QGain, \
                      const tiaDiffGainTypedef tia1IGain, const tiaDiffGainTypedef tia1QGain)
{
    SET_VALUE(RFC->rfc_tia0_cfg_reg, \
              rfc_tia0_cfg_reg_tia0_i_gain_shift, \
              rfc_tia0_cfg_reg_tia0_i_gain_mask, \
              (uint8)tia0IGain);
    SET_VALUE(RFC->rfc_tia0_cfg_reg, \
              rfc_tia0_cfg_reg_tia0_q_gain_shift, \
              rfc_tia0_cfg_reg_tia0_q_gain_mask, \
              (uint8)tia0QGain);
    SET_VALUE(RFC->rfc_tia0_cfg_reg, 5, ((1<<2)-1), \
              (uint8)tia0IGain);
    
    SET_VALUE(RFC->rfc_tia1_cfg_reg, \
              rfc_tia1_cfg_reg_tia1_i_gain_shift, \
              rfc_tia1_cfg_reg_tia1_i_gain_mask, \
              (uint8)tia1IGain);
    SET_VALUE(RFC->rfc_tia1_cfg_reg, \
              rfc_tia1_cfg_reg_tia1_q_gain_shift, \
              rfc_tia1_cfg_reg_tia1_q_gain_mask, \
              (uint8)tia1QGain);
    SET_VALUE(RFC->rfc_tia1_cfg_reg, 5, ((1<<2)-1), \
              (uint8)tia1IGain);
}


/**
 * @brief   rf tia bias setting.selection:0~15
 * @param   tiaBiasI: I channel, tia bias.
 * @param   tiaBiasQ: Q channel, tia bias.
 * @return  None.
 */
void rfTiaDacBias(const uint8 tiaBiasI, const uint8 tiaBiasQ)
{
    SET_VALUE(RFC->rf_bias2_dina_reg, \
              rf_bias2_dina_reg_bias2_din0_shift, \
              rf_bias2_dina_reg_bias2_din0_mask, \
              tiaBiasI);
    
    
    SET_VALUE(RFC->rf_bias2_dina_reg, \
              rf_bias2_dina_reg_bias2_din2_shift, \
              rf_bias2_dina_reg_bias2_din2_mask, \
              tiaBiasQ);
}

/*                        PGA                             */

/**
 * @brief   rf vga enable.
 * @param   pga0Enable: enable VGA0, ENABLE/DISABLE.
 * @param   pga1Enable: enable VGA1, ENABLE/DISABLE.
 * @return  None.
 */
void rfVgaEnable(const stateTypedef pga0Enable, const stateTypedef pga1Enable)
{
    if(ENABLE == pga0Enable)
    {
        SET_BIT(RFC->rfc_vga0_reg, rfc_vga0_reg_pga0_en_bit);
    }
    else if(DISABLE == pga0Enable)
    {
        CLEAR_BIT(RFC->rfc_vga0_reg, rfc_vga0_reg_pga0_en_bit);
    }

    if(ENABLE == pga1Enable)
    {
        SET_BIT(RFC->rfc_vga1_reg, rfc_vga1_reg_pga1_en_bit);
    }
    else if(DISABLE == pga1Enable)
    {
        CLEAR_BIT(RFC->rfc_vga1_reg, rfc_vga1_reg_pga1_en_bit);
    }
}


/**
 * @brief   rf vga gain setting.selection:
 *          PGA_GAIN_0DB  PGA_GAIN_2DB  PGA_GAIN_4DB     PGA_GAIN_6DB
 *          PGA_GAIN_8DB  PGA_GAIN_10DB PGA_GAIN_12DB    PGA_GAIN_14DB
 *          PGA_GAIN_16DB PGA_GAIN_18DB PGA_GAIN_19DB    PGA_GAIN_20DB
 *          PGA_GAIN_22DB PGA_GAIN_24DB PGA_GAIN_26DB    PGA_GAIN_28DB
 *          PGA_GAIN_30DB PGA_GAIN_32DB PGA_GAIN_34DB    PGA_GAIN_36DB PGA_GAIN_38DB
 *          PGA_GAIN_40DB
 * @param   pga0IGain: TIA0I diff gain setting.
 * @param   pga0QGain: TIA0Q diff gain setting.
 * @param   pga1IGain: TIA1I diff gain setting.
 * @param   pga1QGain: TIA1Q diff gain setting.
 * @return  None.
 */
void rfVgaGainSetting(const pgaDiffGainTypedef vga0IGain, const pgaDiffGainTypedef vga0QGain, \
                      const pgaDiffGainTypedef vga1IGain, const pgaDiffGainTypedef vga1QGain)
{
    SET_VALUE(RFC->rfc_vga0_reg, \
              rfc_vga0_reg_pga0_i_gain_shift, \
              rfc_vga0_reg_pga0_i_gain_mask, \
              (uint8)vga0IGain);
    SET_VALUE(RFC->rfc_vga0_reg, \
              rfc_vga0_reg_pga0_q_gain_shift, \
              rfc_vga0_reg_pga0_q_gain_mask, \
              (uint8)vga0QGain);
    SET_VALUE(RFC->rfc_vga1_reg, \
              rfc_vga1_reg_pga1_i_gain_shift, \
              rfc_vga1_reg_pga1_i_gain_mask, \
              (uint8)vga1IGain);
    SET_VALUE(RFC->rfc_vga1_reg, \
              rfc_vga1_reg_pga1_q_gain_shift, \
              rfc_vga1_reg_pga1_q_gain_mask, \
              (uint8)vga1QGain);
}


/**
 * @brief   rfPgaGainCal. selection:
 *          PGA_CAL_N1 0  //-1DB
 *          PGA_CAL_N5 1  //-0.5DB
 *          PGA_CAL_P1 2  //0.5DB
 *          PGA_CAL_P5 3  //1DB
 * @param   pga0ICal: PGA0I calibration gain, ENABLE/DISABLE.
 * @param   pga0QCal: PGA0Q calibration gain, ENABLE/DISABLE.
 * @param   pga1ICal: PGA1I calibration gain, ENABLE/DISABLE.
 * @param   pga1QCal: PGA1Q calibration gain, ENABLE/DISABLE.
 * @return  None.
 */
void rfPgaGainCalEnable(const stateTypedef pga0ICalEnable, const stateTypedef pga0QCalEnable,
                        const stateTypedef pga1ICalEnable, const stateTypedef pga1QCalEnable)
{
    /*RW
    <7:6>: PGA1_REG3<1:0>: . <5:4>: PGA0_REG3<1:0>:     //pga calibration gain enable
    <3:2>: vblo_cal<3:2>: channel1, vbias for Mixer LO. //adjust TIA1 Gain uniformity(default)
    <1:0>: vblo_cal<1:0>: channel0, vbias for Mixer LO. //adjust TIA0 Gain uniformity(default)
    */
    if(ENABLE == pga0ICalEnable)
    {
        SET_BIT(RFC->rf_bias1_en_reg, (1 << BIT_4));
    }
    else if(DISABLE == pga0ICalEnable)
    {
        CLEAR_BIT(RFC->rf_bias1_en_reg, (1 << BIT_4));
    }

    if(ENABLE == pga0QCalEnable)
    {
        SET_BIT(RFC->rf_bias1_en_reg, (1 << BIT_5));
    }
    else if(DISABLE == pga0QCalEnable)
    {
        CLEAR_BIT(RFC->rf_bias1_en_reg, (1 << BIT_5));
    }

    if(ENABLE == pga1ICalEnable)
    {
        SET_BIT(RFC->rf_bias1_en_reg, (1 << BIT_6));
    }
    else if(DISABLE == pga1ICalEnable)
    {
        CLEAR_BIT(RFC->rf_bias1_en_reg, (1 << BIT_6));
    }

    if(ENABLE == pga1QCalEnable)
    {
        SET_BIT(RFC->rf_bias1_en_reg, (1 << BIT_7));
    }
    else if(DISABLE == pga1QCalEnable)
    {
        CLEAR_BIT(RFC->rf_bias1_en_reg, (1 << BIT_7));
    }
}

void rfPgaGainCalSetting(const pgaCalTypedef pga0ICal, const pgaCalTypedef pga0QCal, \
                         const pgaCalTypedef pga1ICal, const pgaCalTypedef pga1QCal)
{
    uint32 regVel = 0;
    regVel = RFC->rfc_tia0_cfg_reg;
    regVel &= ~(0x3 << 1);
    regVel |= pga0ICal << 1;

    regVel &= ~(0x1 << 0);
    regVel |= ((pga0QCal & 0x02) >> 1) << 0;
    RFC->rfc_tia0_cfg_reg = regVel;

    RFC->rfc_vga0_reg     &= ~(0x1 << 8);
    RFC->rfc_vga0_reg     |= (pga0QCal & 0x01) << 8;

    regVel = 0;
    regVel = RFC->rfc_tia1_cfg_reg;
    regVel &= ~(0x3 << 1);
    regVel |= pga1ICal << 1;

    regVel &= ~(0x1 << 0);
    regVel |= ((pga1QCal & 0x02) >> 1) << 0;
    RFC->rfc_tia1_cfg_reg = regVel;

    RFC->rfc_vga1_reg     &= ~(0x1 << 8);
    RFC->rfc_vga1_reg     |= (pga1QCal & 0x01) << 8;
}


/**
 * @brief   rf vga setting. selection:0~15
 * @param   pgaBiasI: DAC4 data, I channel, pga bia.
 * @param   pgaBiasQ: bias2_din1:DAC2 data, Q channel, pga bias.
 * @return  None.
 */
void rfVgaDacBias(const uint8 pgaBiasI, const uint8 pgaBiasQ)
{
    SET_VALUE(RFC->rf_bias2_dina_reg, \
              rf_bias2_dina_reg_bias2_din1_shift, \
              rf_bias2_dina_reg_bias2_din1_mask, \
              pgaBiasI);
    SET_VALUE(RFC->rf_bias2_dina_reg, \
              rf_bias2_dina_reg_bias2_din3_shift, \
              rf_bias2_dina_reg_bias2_din3_mask, \
              pgaBiasQ);
}


/**
 * @brief   tia hpf parameter. selection:
 *          RC_HPF_150KHZ
 *          RC_HPF_400KHZ
 * @param   tia0HpfEnable: ENABLE/DISABLE.
 * @param   tia1HpfEnable: ENABLE/DISABLE.
 * @param   tia0HpfParameter: tia0 hpf.
 * @param   tia1HpfParameter: tia0 hpf.
 * @return  None.
 */
void tiaHpfConfig(const stateTypedef tia0HpfEnable, const stateTypedef tia1HpfEnable, \
                  const tiaHpfTypedef tia0HpfParameter, const tiaHpfTypedef tia1HpfParameter)
{
    //select rc mode
    uint8 gain = 0;
    uint8 vcom_tia0, vcom_tia1;
    uint32 regVel = 0;

    regVel = RFC->rfc_tia0_cfg_reg;
    gain   = (RFC->rfc_tia0_cfg_reg >> rfc_tia0_cfg_reg_tia0_i_gain_shift) & 0x3;

    if (tia0HpfEnable == 1)
    {
        regVel |= 1<<7;
    }
    else
    {
        regVel &= ~(1<<7); 
    }

    regVel &= ~(0x3<<5);
    regVel |= gain<<5;
    RFC->rfc_tia0_cfg_reg = regVel;
    
    //select rc mode
    regVel = 0;
    regVel = RFC->rfc_tia1_cfg_reg;
    gain   = (RFC->rfc_tia1_cfg_reg >> rfc_tia1_cfg_reg_tia1_i_gain_shift) & 0x3;
    if (tia1HpfEnable == 1)
    {
        regVel |= 1<<7;
    }
    else
    {
        regVel &= ~(1<<7); 
    } 
    regVel &= ~(0x3<<5);
    regVel |= gain<<5;
    RFC->rfc_tia1_cfg_reg = regVel;

    // config hpf frequency and vcom
    regVel = RFC->rf_plln_dacb_reg;

    vcom_tia0 = 0;
    regVel &= ~(0x7 << rf_plln_dacb_reg_tia0_reg2_shift);
    regVel |= (vcom_tia0 << 2 | tia0HpfParameter) << rf_plln_dacb_reg_tia0_reg2_shift;

    vcom_tia1 = 0;
    regVel &= ~(0x7 << rf_plln_dacb_reg_tia1_reg2_shift);
    regVel |= (vcom_tia1 << 2 | tia1HpfParameter) << rf_plln_dacb_reg_tia1_reg2_shift;

    RFC->rf_plln_dacb_reg = regVel;
}


/**
 * @brief   rc hpf parameter.selection:
 *          RC_HPF_150KHZ    RC_HPF_400KHZ
 * @param   rcHpf0Enable: ENABLE/DISABLE.
 * @param   rcHpf1Enable: ENABLE/DISABLE.
 * @param   rc0HpfParameter: rf hpf.
 * @param   rc1HpfParameter: rf hpf.
 * @return  None.
 */
void rcHpfConfig(const stateTypedef  rcHpf0Enable, const stateTypedef  rcHpf1Enable, \
                 const rcHpfTyepdef rc0HpfParameter, const rcHpfTyepdef rc1HpfParameter)
{
    //select bypass mode
    if(ENABLE == rcHpf0Enable)
    {
        CLEAR_BIT(RFC->rfc_tia0_cfg_reg, (1 << BIT_4));
    }
    else if(DISABLE == rcHpf0Enable)
    {
        SET_BIT(RFC->rfc_tia0_cfg_reg, (1 << BIT_4));
    }

    if(RC_HPF_150KHZ == rc0HpfParameter)
    {
        CLEAR_BIT(RFC->rfc_tia0_cfg_reg, (1 << BIT_3));
    }
    else if(RC_HPF_400KHZ == rc0HpfParameter)
    {
        SET_BIT(RFC->rfc_tia0_cfg_reg, (1 << BIT_3));
    }

    //select bypass mode
    if(ENABLE == rcHpf1Enable)
    {
        CLEAR_BIT(RFC->rfc_tia1_cfg_reg, (1 << BIT_4));
    }
    else if(DISABLE == rcHpf1Enable)
    {
        SET_BIT(RFC->rfc_tia1_cfg_reg, (1 << BIT_4));
    }

    if(RC_HPF_150KHZ == rc1HpfParameter)
    {
        CLEAR_BIT(RFC->rfc_tia1_cfg_reg, (1 << BIT_3));
    }
    else if(RC_HPF_400KHZ == rc1HpfParameter)
    {
        SET_BIT(RFC->rfc_tia1_cfg_reg, (1 << BIT_3));
    }
}


/**
 * @brief   control lpf parameter. selection:
 *          PGA_LPF_2M    PGA_LPF_4M
 *          PGA_LPF_6M    PGA_LPF_8M
 *          <3>: lpf_fsel<0>: 00: 2M, 01: 4M, 10: 6M,, 11: 8MHz.
 *          <4>: lpf_fsel<1>: .
 *          <7>: pga_dcos_en: pga dc offset calibration enable, '0' disable.
 * @param   pga0LpfParameter: pga0 lpf.
 * @param   pga1LpfParameter: pga1 lpf.
 * @return  None.
 */
void pgaLpfConfig(const pgaLpfTypedef pga0LpfParameter, const pgaLpfTypedef pga1LpfParameter)
{
    SET_VALUE(RFC->rfc_vga0_reg, BIT_3, ((1 << BIT_2) - 1), (uint8)pga0LpfParameter);
    SET_VALUE(RFC->rfc_vga1_reg, BIT_3, ((1 << BIT_2) - 1), (uint8)pga1LpfParameter);
}


/**
 * @brief   control lpf cal.
 * @param   lpf0CalEn: ENABLE/DISABLE.
 * @param   lpf1CalEn: ENABLE/DISABLE.
 * @param   lpf0CalConf: .
 * @param   lpf1CalConf: .
 * @return  None.
 */
void pgaLpfCal(int32 lpf0CalEn, int32 lpf1CalEn, int32 lpf0CalConf, int32 lpf1CalConf)
{
    uint32 regVel = 0;
    regVel = RFC->rfc_vga0_reg;
    if(lpf0CalEn == 1)
    {
        regVel |= 1 << 2;
    }
    else
    {
        regVel &= ~(1 << 2);
    }
    regVel &= ~(0x3 << 14);
    regVel |= lpf0CalConf << 14;
    RFC->rfc_vga0_reg = regVel;

    regVel = 0;
    regVel = RFC->rfc_vga1_reg;
    if(lpf1CalEn == 1)
    {
        regVel |= 1 << 2;
    }
    else
    {
        regVel &= ~(1 << 2);
    }
    regVel &= ~(0x3 << 14);
    regVel |= lpf1CalConf << 14;
    RFC->rfc_vga1_reg = regVel;
}


/**
 * @brief   pga hpf parameter. selection:
 *          PGA_HPF_25KHZ    PGA_HPF_50KHZ
 *          PGA_HPF_100KHZ   PGA_HPF_500KHZ
 * @param   vga0HpfEnable: ENABLE/DISABLE.
 * @param   vga1HpfEnable: ENABLE/DISABLE.
 * @param   pga0HpfParameter: pga0 hpf.
 * @param   pga1HpfParameter: pga1 hpf.
 * @return  None.
 * @note
 */
void pgaHpfConfig(const stateTypedef pga0HpfEnable, const stateTypedef pga1HpfEnable, \
                  const pgaHpfTypedef pga0HpfParameter, const pgaHpfTypedef pga1HpfParameter)
{
    //dc offset
    if(ENABLE == pga0HpfEnable)
    {
        SET_BIT(RFC->rfc_vga0_reg, (1 << BIT_7));
    }
    else if(DISABLE == pga0HpfEnable)
    {
        CLEAR_BIT(RFC->rfc_vga0_reg, (1 << BIT_7));
    }

    if(ENABLE == pga1HpfEnable)
    {
        SET_BIT(RFC->rfc_vga1_reg, (1 << BIT_7));
    }
    else if(DISABLE == pga1HpfEnable)
    {
        CLEAR_BIT(RFC->rfc_vga1_reg, (1 << BIT_7));
    }

    SET_VALUE(RFC->rfc_vga0_reg, BIT_5, ((1 << BIT_2) - 1), pga0HpfParameter);
    SET_VALUE(RFC->rfc_vga1_reg, BIT_5, ((1 << BIT_2) - 1), pga1HpfParameter);
}


/**
 * @brief   set en_buffer VGA output buffer(to PAD).
 * @param   pga0BuffEn: ENABLE/DISABLE.
 * @param   pga1BuffEn: ENABLE/DISABLE.
 * @note
 */
void setPgaOutputBuff(const stateTypedef pga0BuffEn, const stateTypedef pga1BuffEn)
{
    if(ENABLE == pga0BuffEn)
    {
        SET_BIT(RFC->rfc_vga0_reg, (1 << BIT_9));
    }
    else if(DISABLE == pga0BuffEn)
    {
        CLEAR_BIT(RFC->rfc_vga0_reg, (1 << BIT_9));
    }

    if(ENABLE == pga1BuffEn)
    {
        SET_BIT(RFC->rfc_vga1_reg, (1 << BIT_9));
    }
    else if(DISABLE == pga1BuffEn)
    {
        CLEAR_BIT(RFC->rfc_vga1_reg, (1 << BIT_9));
    }
}
