/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_tx.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file contains all the functions implemented by the TX
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_tx.h"
#include "timer_bsp.h"
#include "adt6101P_type_define.h"
#include "pmu_ctype_map.h"
#include "rfc_ctype_map.h"
#include <stdio.h>
#include <arm_math.h>


/* Private functions ---------------------------------------------------------*/
/*-----------------------PLLI-----------------------------*/
/**
 * @brief   pll vco Ldo setting, pll pll vco ldo is enable default, cloes through FSM.
 * @param   pllVcoLdoEnable: pll Vco Ldo Enable.
 * @param   pllPllLdoEnable: pll Pll Ldo Enable.
 * @return  None.
 * @note
 */
void pllLDOEnable(int32 pllVcoLdoEnable, int32 pllPllLdoEnable)
{
    uint32 regVel = 0;
    /* xo_core_bias=15 config. From Yuan. 2021.3.12. */
    RFC->rfc_xo50m_cfg_reg = 0x000008F8;

    /* Disable bit4:low noise function, especially for hare_switch project.
    Because this bit will spend about 30ms for avdd18_plli to become stable. From XX. */
    RFC->rfc_ldo_bg_reg_reg = 0x00000200;

    /* Firstly, only open essential modules. */
    regVel =	(rfc_plli_ctrl_reg_pll_en_ldopll_bit |
                 rfc_plli_ctrl_reg_pll_en_bias_bit |
                 rfc_plli_ctrl_reg_xo50m_en_bit |
                 rfc_plli_ctrl_reg_xo50m_en_ckdig_bit);
    /* Then, if VCO enable, then open other modules. otherwise, do not open them. */
    /* there are 2 modes, mode 1 is all on. mode 2 is only clock doubler on, for MCU running on 100mhz. */
    /* So mode 1 means pllVcoLdoEnable = 1.  and mode 2 means (pllVcoLdoEnable = 0 and pllPllLdoEnable = 1). XXiong. */ 
    if(pllVcoLdoEnable == 1)
    {
        regVel |=   (rfc_plli_ctrl_reg_pll_en_ldovco_bit | 
                    rfc_plli_ctrl_reg_pll_en_pll_bit |
                    rfc_plli_ctrl_reg_pll_en_frefx2_bit |
                    rfc_plli_ctrl_reg_pll_en_ckdig_100m_bit |
                    rfc_plli_ctrl_reg_pll_en_ckadc_bit 
                    );
    }
    else if(pllPllLdoEnable)
    {
        regVel |=   (rfc_plli_ctrl_reg_pll_en_pll_bit |
                    rfc_plli_ctrl_reg_pll_en_frefx2_bit |
                    rfc_plli_ctrl_reg_pll_en_ckdig_100m_bit
                    );        
    }
    RFC->rfc_plli_ctrl_reg = regVel;

    delayUs(100);
}

/*-----------------------PLLN-----------------------------*/
/**
 * @brief   pll vco config. selection:
 *          PLLN_VCO_LDO_1P8     PLLN_VCO_LDO_2P2
 *          PLLN_VCO_LDO_2P0     PLLN_VCO_LDO_2P5
 * [11:8]   4'h8    plln_bias_reg1:
 * <0>: ibias_sel_biasgen: 0: Select V-to-I biasing from BG_1P8, 1: V-to-I biasing is generated internally.
 * <3:1>: ibias_ztc_sel<2:0>: V-to-I output current selection,
 * '0' *1.8, '1' *1.55, '2' *1.325, '3' *1.15, '4' *1(default), '5' *0.9, '6' *0.825, '7' *0.75.
 * [7:0]    8'h0    plln_ldo_reg1:
 * <0>: NC: NC.
 * <2:1>: : adjust avdd_VCO, '00' 1.8v, '01' 2.0v, '10' 2.2v, '11' 2.5v.
 * <3>: : ldo_1p8_lown for VCO.
 * <5:4>: : adjust avdd_VCO, '00' 1.8v, '01' 2.0v, '10' 2.2v, '11' 2.5v.
 * <6>: : ldo_1p8_lown for PLL. <7>: : NC.
 * @param   pllVcoLdoEnable: enable VCO LDO.
 * @param   pllVcoLdo: adjust avdd_VCO for PLL VCO
 * @return  None.
 * @note
 */
void pllVcoConfig(int32 pllVcoLdoEnable, int32 pllVcoLdo)
{
    // Actually, we only configure PLL VCO's voltage in this function. 
    // and don't need to turn on/off pll's LDO in this function. because it's already in pllLDOEnable function. XXiong. 
//    if(pllVcoLdoEnable == 1)
//    {
//        SET_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_pll_en_ldovco_bit);
//    }
//    else
//    {
//        CLEAR_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_pll_en_ldovco_bit);
//    }

    RFC->rf_plln_ldo_cfg_reg &= ~(3 << 0);
    RFC->rf_plln_ldo_cfg_reg |= pllVcoLdo << 0;
}


/**
 * @brief   plln pll config.selection:
 *          PLLN_VCO_LDO_1P8     PLLN_VCO_LDO_2P2
 *          PLLN_VCO_LDO_2P0     PLLN_VCO_LDO_2P5
 * [11:8]	4'h8	plln_bias_reg1:
 * <0>: ibias_sel_biasgen: 0: Select V-to-I biasing from BG_1P8, 1: V-to-I biasing is generated internally.
 * <3:1>: ibias_ztc_sel<2:0>: V-to-I output current selection,
 * '0' *1.8, '1' *1.55, '2' *1.325, '3' *1.15, '4' *1(default), '5' *0.9, '6' *0.825, '7' *0.75.
 * [7:0]	8'h0	plln_ldo_reg1:
 * <0>: NC: NC.
 * <2:1>: : adjust avdd_VCO, '00' 1.8v, '01' 2.0v, '10' 2.2v, '11' 2.5v.
 * <3>: : ldo_1p8_lown for VCO.
 * <5:4>: : adjust avdd_VCO, '00' 1.8v, '01' 2.0v, '10' 2.2v, '11' 2.5v.
 * <6>: : ldo_1p8_lown for PLL. <7>: : NC.
 * @param   pllnPllLdoEnable: enable PLLN LDO.
 * @param   pllnPllLdo: adjust avdd for PLLN.
 * @param   pllnEnBias: plln enable bias.
 * @return  None.
 * @note
 */
void pllPllConfig(int32 pllPllLdoEnable, int32 pllPllLdo, int32 pllEnBias)
{
    // Actually, we only configure PLL LDO's voltage in this function. 
    // and don't need to turn on/off pll's LDO in this function. because it's already in pllLDOEnable function. XXiong. 
    
//    if(pllPllLdoEnable == 1)
//    {
//        SET_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_pll_en_ldopll_bit);
//    }
//    else
//    {
//        CLEAR_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_pll_en_ldopll_bit);
//    }

//    if(pllEnBias == 1)
//    {
//        SET_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_pll_en_bias_bit);
//    }
//    else
//    {
//        CLEAR_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_pll_en_bias_bit);
//    }

    RFC->rf_plln_ldo_cfg_reg &= ~(3 << 4);
    RFC->rf_plln_ldo_cfg_reg |= pllPllLdo << 4;
}


/**
 * @brief   PLLI dac setting.
 * @param   None.
 * @return  None.
 * @note
 */
void plliDacSetting(void)
{
    uint32 regVel = 0;
    //rf_plli_daca_reg: 0x40003158
    //from yuan for PLLI default value
    regVel = 0x108A4888;
    RFC->rf_plli_daca_reg = regVel;

    //rf_plli_dacb_reg: 0x4000315c
    regVel = 0x00000044;
    RFC->rf_plli_dacb_reg = regVel;
}


/**
 * @brief   plln config.
 * @param   vcoEn: ENABLE/DISABLE.
 * @return  None.
 * @note    Loop filter: 1MHz.
 */
void pllnConfig(int32 vcoEn)
{
    /* PLL-N configuration for PLL-N BW=1MHz. External loop filter value: R2=200 Ohm, C2=22nF, no external C1 required.
        PLL-N configuration for PLL-N BW=300KHz. External loop filter value: R2=130 Ohm, C2=4.7nF, no external C1 required. */
    /*
        ___________oVtunt
       |        |
       |        -
       |       | |R2
       |       | |
       |        -
       |        |
      -----    -----
    C1-----  c2-----
       |        |
       |        |
       |________|____Gnd
    */
    /* config for haifeng plln reg1~4. cp_offip=cp_offin=3'b0, lpf3d_sel=2'b00 */

    uint32 regVel = 0;

    /* in low power mode switch operation, write rf_plln_rega_reg default value without change vcoEn bit */

    /* change to 0xB722887F following yuan's advice  0x40003184 = 0xB722887F */
    RFC->rf_plln_rega_reg = 0xB722887F;//0x0220887f

    // in low power mode switch operation, write vcoEn bit only in rf_plln_rega_reg
    // pllVcoEnable(vcoEn);

    /* becase of lowpower, close plln, so don't write vco switch */

    //rf_plln_regb_reg: 0x40003188:   0x202440FF
    //following yuan's suggestion, 0x40003188 = 0x202440FF
    regVel = (RFC->rf_plln_regb_reg) & 0xf00;
    RFC->rf_plln_regb_reg = 0x202440FF|regVel;
    
    //config for haifeng
    //plln reg9~12. en_ld=1, ld_vh=ld_vl=3'b001
    //from yuan
    //rf_plln_regc_reg:  0x4000318c
    RFC->rf_plln_regc_reg = 0x00000492;
    
    //from yuan
    //rf_plln_regd_reg: 0x40003190
    RFC->rf_plln_regd_reg = 0x00000007;
}


/**
 * @brief   plln vco config.
 * @param   vcoEn: ENABLE/DISABLE.
 * @return  None.
 * @note
 */
void pllVcoEnable(int32 vcoEn)
{
    if(vcoEn == 1)
    {
        RFC->rf_plln_rega_reg |= 1 << 1;
    }
    else
    {
        RFC->rf_plln_rega_reg &= ~(1 << 1);
    }
}


/**
 * @brief   Set plln vco cap.
 * @param   val: vco cap.
 * @return  None.
 * @note
 */
void setPllVcoCap(uint8 val)
{
    SET_VALUE(RFC->rf_plln_rega_reg,                \
              rf_plln_regb_reg_pll_reg6_shift,   \
              ((1 << 4) - 1),    \
              val);
}

/**
 * @brief   Get plln vco cap.
 * @return  vco cap.
 * @note
 */
uint8 getPllVcoCap(void)
{
    return GET_BITS_VALUE(RFC->rf_plln_rega_reg,    \
                          rf_plln_regb_reg_pll_reg6_shift,   \
                          ((1 << 4) - 1));
}


/**
 * @brief   Get the status of the PLLN lock, pll lock detecter, can output to GPIO.
 * @return  1: lock, 0: not locked.
 * @note
 */
uint8 getPllnLockStatus(void)
{
    return GET_BIT_VALUE(RFC->rf_plln_rege_reg, BIT_2);
}


/**
 * @brief   plln buf enable.
 * @param   bufEn: ENABLE/DISABLE.
 * @return  None.
 * @note
 */
void pllBufEnable(int32 bufEn)
{
    if (bufEn == 1)
    {
        RFC->rf_plln_rega_reg |= 3<<3;
    }
    else
    {
        RFC->rf_plln_rega_reg &= ~(3<<3);
    }
}

/**
 * @brief   plln MMD enable.
 * @param   MMDEn: ENABLE/DISABLE.
 * @return  None.
 * @note
 */
void pllMMDEnable(int32 MMDEn)
{
    if (MMDEn == 1)
    {
        RFC->rf_plln_rega_reg |= 1<<2;
    }
    else
    {
        RFC->rf_plln_rega_reg &= ~(1<<2);
    }
}

/*------------------------LDO-----------------------------*/
/**
 * @brief   adjust LDO output, '00' 1.8v, '01' 2v, '10' 2.2v, '11' 2.5v.
 * register*RW*rfc_rx_lna_reg
 * [5:3]	3'h0	ldo18bb_reg1:
 * <1:0>: ldo_out<1:0>: adjust LDO output, '00' 1.8v, '01' 2v, '10' 2.2v, '11' 2.5v.
 * <2>: en_fbc: 1: Enable diode-connected MOS feedback.
 * [2:0]	3'h0	ldo18bb_reg0:
 * <1:0>: ldo_out<1:0>: adjust LDO output, '00' 1.8v, '01' 2v, '10' 2.2v, '11' 2.5v.
 * <2>: en_fbc: 1: Enable diode-connected MOS feedback.
 * @param   bb1Enable: baseband1 ldo enable.
 * @param   bb1Adjust: adjust baseband1 LDO output.
 * @param   bb2Enable: baseband2 ldo enable.
 * @param   bb2Adjust: adjust baseband2 LDO output.
 * @return  None.
 * @note
 */
void baseband18LDO(int32 bb1Enable, int32 bb1Adjust, int32 bb2Enable, int32 bb2Adjust)
{
    /* option 1: this register only has bb1Adjust and bb2Adjust, and there is no other bit. */
    CLEAR_VALUE(RFC->rf_ldo18bb_regb_reg);
    SET_VALUE(RFC->rf_ldo18bb_regb_reg, \
              rf_ldo18bb_regb_reg_ldo18bb_reg1_shift, \
              rf_ldo18bb_regb_reg_ldo18bb_reg1_mask, \
              bb2Adjust);
    SET_VALUE(RFC->rf_ldo18bb_regb_reg, \
              rf_ldo18bb_regb_reg_ldo18bb_reg0_shift, \
              rf_ldo18bb_regb_reg_ldo18bb_reg0_mask, \
              bb1Adjust);

    /* option 2: this register has other bits except bb1Adjust and bb2Adjust. */
    /*
    SET_VALUE(RFC->rf_ldo18bb_regb_reg, \
              rf_ldo18bb_regb_reg_ldo18bb_reg1_shift, \
              rf_ldo18bb_regb_reg_ldo18bb_reg1_mask, \
              bb2Adjust);
    SET_VALUE(RFC->rf_ldo18bb_regb_reg, \
              rf_ldo18bb_regb_reg_ldo18bb_reg0_shift, \
              rf_ldo18bb_regb_reg_ldo18bb_reg0_mask, \
              bb1Adjust);
    */

    if(bb1Enable == 1)
    {
        SET_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_ctrl_ldo18bb1_on_bit);
    }
    else
    {
        CLEAR_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_ctrl_ldo18bb1_on_bit);
    }

    if(bb2Enable == 1)
    {
        SET_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_ctrl_ldo18bb2_on_bit);
    }
    else
    {
        CLEAR_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_ctrl_ldo18bb2_on_bit);
    }

}


/**
 * @brief   Adjust the out of the LDO_1p2_HP, '00' 1.2v, '01' 1.1v, '10' 1.3v, '11' 1.4v.
 *          selection:  POWER_ON        DVDD1P2_HP DVDD1P3_HP
 *                      POWER_OFF       DVDD1P1_HP DVDD1P4_HP
 * register*RW*rfc_ldo12n_rega_reg
 * [9:8]    2'h0    ldo12_hp_vcfg:
 * Adjust the out of the LDO_1p2_HP, '00' 1.2v, '01' 1.1v, '10' 1.3v, '11' 1.4v
 * @param   powerOnDown: usually we do not turn off vdd.
 * @param   dvdd12Adjust: Adjust the out of the LDO_1p2_HP.
 * @return  None.
 * @note
 */
void dvdd12HpLDO(int32 powerOnDown, int32 dvdd12Adjust)
{
    if(powerOnDown == 1)
    {
        SET_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_ctrl_ldo12_pd_bit);
    }
    else
    {
        CLEAR_BIT(PMU->pmu_pmu_ctrl_reg, pmu_pmu_ctrl_reg_ctrl_ldo12_pd_bit);
    }

    SET_VALUE(RFC->rfc_ldo12n_rega_reg, \
              rfc_ldo12n_rega_reg_ldo12_hp_vcfg_shift, \
              rfc_ldo12n_rega_reg_ldo12_hp_vcfg_mask, \
              dvdd12Adjust);

    delayUs(100);
}


/*-----------------------Mixer---------------------------*/
/**
 * @brief   o_reg:FMCW buffer mode.
 *          selection:  MODE_INTER    : 20G to internal doubler
 *                      MODE_EXTERN   : 20G from external source
 *                      MODE_CASCADED : 20G to internal doubler and output other chip
 *                      MODE_TEST     : 20G output test port
 *
 *  register*RW*rf_buf20g_db_reg
 *  [3:2]2'h0	lo_reg:FMCW buffer mode, 00': int32 mode, '01' external mode, '10' cascaded mode, '11' test mode
 * @param   fmcwMode: 00': int32 mode, '01' external mode,
 *                      '10' cascaded mode, '11' test mode.
 * @return  None.
 * @note
 */
void fmcwModeSel(int32 fmcwMode)
{
    SET_VALUE(RFC->rf_buf20g_db_reg, \
              rf_buf20g_db_reg_lo_reg_shift, \
              rf_buf20g_db_reg_lo_reg_mask, \
              fmcwMode);
}


/**
 * @brief   20G buffer and doubler enable.
 * register*RW*rf_buf20g_db_reg
 * [3:2]2'h0    lo_reg:FMCW buffer mode, 00': int32 mode, '01' external mode, '10' cascaded mode, '11' test mode
 * [1]  1'h0    buf20g_en:20G buffer enable
 * [0]  1'h0    dbl_en:doubler enable
 * @param   buffer20gEn: 20G buffer enable.
 * @param   doublerEn: doubler enable.
 * @return  None.
 * @note
 */
void rfbuff20G(const stateTypedef buffer20gEn, const stateTypedef triplerEn)
{
    if(ENABLE == triplerEn)
    {
        SET_BIT(RFC->rf_buf20g_db_reg, rf_buf20g_db_reg_tripler_en_bit);
    }
    else if(DISABLE == triplerEn)
    {
        CLEAR_BIT(RFC->rf_buf20g_db_reg, rf_buf20g_db_reg_tripler_en_bit);
    }

    if(ENABLE == buffer20gEn)
    {
        SET_BIT(RFC->rf_buf20g_db_reg, rf_buf20g_db_reg_buf20g_en_bit);
    }
    else if(DISABLE == buffer20gEn)
    {
        CLEAR_BIT(RFC->rf_buf20g_db_reg, rf_buf20g_db_reg_buf20g_en_bit);
    }
    
    /* add for tripler buffer enable for adt6101P */
    if(ENABLE == triplerEn)
    {
        SET_BIT(RFC->rfc_rx_lna_reg, 1<<BIT_2);
    }
    else if(DISABLE == triplerEn)
    {
        CLEAR_BIT(RFC->rfc_rx_lna_reg, 1<<BIT_2);
    }
    
}


/**
 * @brief   about 60G buffer dac enable.
 * @param   biasdb60G: ib_db60.
 * @param   biasBuff38: ib_buf38.
 * @param   biasDb38: ib_db38.
 * @param   bias2Buff20G: 20G buf bias2.
 * @param   biasbuff20G: 20G buf.
 * @param   txBuff1To4: buf_1to4, tx loop.
 * @param   loBuff1To4: buf_1to4, lo loop.
 * @param   rxBuff1To4: buf_1to4, rx loop.
 * @return  None.
 * @note
 */
void bias20GTo60G(int32 bias20Grf, int32 biasTripler20G1, int32 biasTripler20G2, int32 bias20Ginput,
                  int32 bias20Goutput, int32 biasTx60G, int32 bias60G, int32 biasRx60G)
{
    uint32 val = 0;
    SET_VALUE(val, \
              rf_bias1_dinb_reg_bias1_din8_shift, \
              rf_bias1_dinb_reg_bias1_din8_mask,  \
              biasRx60G);
    SET_VALUE(val, \
              rf_bias1_dinb_reg_bias1_din9_shift, \
              rf_bias1_dinb_reg_bias1_din9_mask,  \
              bias60G);
    SET_VALUE(val, \
              rf_bias1_dinb_reg_bias1_din10_shift, \
              rf_bias1_dinb_reg_bias1_din10_mask, \
              biasTx60G);
    SET_VALUE(val, \
              rf_bias1_dinb_reg_bias1_din11_shift, \
              rf_bias1_dinb_reg_bias1_din11_mask, \
              bias20Goutput);
    SET_VALUE(val, \
              rf_bias1_dinb_reg_bias1_din12_shift, \
              rf_bias1_dinb_reg_bias1_din12_mask, \
              bias20Ginput);
    SET_VALUE(val, \
              rf_bias1_dinb_reg_bias1_din13_shift, \
              rf_bias1_dinb_reg_bias1_din13_mask, \
              biasTripler20G2);
    SET_VALUE(val, \
              rf_bias1_dinb_reg_bias1_din14_shift, \
              rf_bias1_dinb_reg_bias1_din14_mask, \
              biasTripler20G1);
    SET_VALUE(val, \
              rf_bias1_dinb_reg_bias1_din15_shift, \
              rf_bias1_dinb_reg_bias1_din15_mask, \
              bias20Grf);

    RFC->rf_bias1_dinb_reg = val;
}

/**
 * @brief   txMIMOEn
 * register*RW*rfc_rx_lna_reg
 * [3:2]2'h0	lna_reg:
 * <1>: MIMO_MODE: '1' PA0/1_EN control both TX0/1 buffer and PA0/1. 
 *                 '0' BUF60G_EN_TX0/1 control TX0/1 buffer and PA0/1_EN control PA0/1. 
 * <0>: EN_TRIPLER_BUF: Enable tripler buffer.
 * @param   mimoEn: MIMO enable
 * @return  None.
 * @note
 */
void txMIMO(int32 mimoEn)
{
    if(mimoEn == 0)
    {
        SET_BIT(RFC->rfc_rx_lna_reg, 1<<BIT_3);
    }
    else
    {
        CLEAR_BIT(RFC->rfc_rx_lna_reg, 1<<BIT_3);
        
        // Please make sure g_tx0En=g_tx1En=1.  g_PaGain0 and g_PaGain0 not be zero. 
        // TD_MIMO could be confirmed by sending two-chirp data through the application protocol.  
        // such as the application protocol can send two-chirp data here if needed; 
        // Please add more delayUs between frames, because protocol transmission needs more time for two-chirp data.
        uint32 chirp_channel_num;
        uint32 chirp_channel_bits;
        chirp_channel_num = RFC->fmcw_reg6_reg + 1;        // chirp_channel_num  
        chirp_channel_bits = (1 << chirp_channel_num) - 1; //for example,  2 channels: 0b11.  3 channels:0b111. 
        RFC->fmcw_mimo_tx0_on = chirp_channel_bits;
        RFC->fmcw_mimo_tx1_on = chirp_channel_bits << chirp_channel_num;
        RFC->fmcw_mimo_ctrl   = (chirp_channel_num << 1) | fmcw_mimo_ctrl_auto_tx_on_en_bit;   //chirp_channel_num*2.
    }
}

/**
 * @brief   enable for dac<9:24> and bias1 power down, '0' enable, '1' disable.
 * register*RW*rf_buf20g_db_reg
 * [31:28]  4'h8    bias1_din15:DAC16 data,ib_db77
 * [27:24]  4'h8    bias1_din14:DAC15 data,ib_buf38
 * [23:20]  4'h8    bias1_din13:DAC14 data, ib_db38
 * [19:16]  4'h8    bias1_din12:DAC13 data, 20G buf bias2
 * [15:12]  4'h8    bias1_din11:DAC12 data, 20G buf
 * [11:8]   4'h8    bias1_din10:DAC11 data, buf_1to4, tx loop
 * [7:4]    4'h8    bias1_din9: DAC10 data, buf_1to4, lo loop
 * [3:0]    4'h8    bias1_din8: DAC9 data, buf_1to4, rx loop
 * @param   dacEnable: enable for dac<9:24>.
 * @param   Bias1PowerDown: bias1 power down, '0' enable, '1' disable.
 * @return  None.
 * @note
 */
void dacEnBias1Pd(int32 dacEnable, int32 Bias1PowerDown)
{
    /*RW
    <3>: dac_en<9:24>: enable for dac<9:24>.
    <0>: pd: bias power down, '0' enable, '1' disable.
    */
    //dac9~24 enable
    //RFC->rf_bias1_regs_reg |= 1<<3;
    uint32 regVel = 0;
    // regVel = RFC->rf_bias1_regs_reg;
    regVel = 0x0e050;

    if(dacEnable == 1)
    {
        regVel |= 1 << 3;
    }
    else
    {
        regVel &= ~(1 << 3);
    }
    if(Bias1PowerDown == 1)
    {
        regVel |= 1 << 0;
    }
    else
    {
        regVel &= ~(1 << 0);
    }
    RFC->rf_bias1_regs_reg = regVel;
}


/**
 * @brief   bias Power dector
 * @param   biasPowerDector: biasPowerDector
 * @return  None.
 * @note
 */
void biasPowerdector(int32 biasPowerDector)
{
    SET_VALUE(RFC->rf_bias1_dinc_reg, \
              rf_bias1_dinc_reg_bias1_din19_shift, \
              rf_bias1_dinc_reg_bias1_din19_mask, \
              biasPowerDector);
}

/**
 * @brief   bist bias setting.
 * @param   biasBistIptat0: bias1_din18:DAC18, bist IPTAT<1>.
 * @param   biasBistIptat1: bias1_din17:DAC17, bist IPTAT<0>.
 * @param   biasBist0: bias2_din18:DAC15 data, Bist bias<1>.
 * @param   biasBist1: bias2_din17:DAC14 data, Bist bias<0>.
 * @return  None.
 * @note
 */
void biasBist(int32 biasBistIptat0, int32 biasBistIptat1, int32 biasBist0, int32 biasBist1)
{
    // 22/12/2021: Updated descriptions based on ADT600 registers by Diep

    /*RW
    [23:20]	4'h8	bias1_din5:DAC6, bist IPTAT<1>
    [19:16]	4'h8	bias1_din4:DAC5, bist IPTAT<0>
    */
    SET_VALUE(RFC->rfc_bias1_dina_reg, \
              rfc_bias1_dina_reg_bias1_din4_shift, \
              rfc_bias1_dina_reg_bias1_din4_mask, \
              biasBistIptat0);

    SET_VALUE(RFC->rfc_bias1_dina_reg, \
              rfc_bias1_dina_reg_bias1_din5_shift, \
              rfc_bias1_dina_reg_bias1_din5_mask, \
              biasBistIptat1);
    

    /*RW
    [11:8]	4'h0	bias2_din18:DAC19 data, Bist IZTC<1>
    [7:4]	  4'h0	bias2_din17:DAC18 data, Bist IZTC<0>
    */    
    SET_VALUE(RFC->rf_bias2_dinc_reg, \
              rf_bias2_dinc_reg_bias2_din17_shift, \
              rf_bias2_dinc_reg_bias2_din17_mask, \
              biasBist0);
    
    SET_VALUE(RFC->rf_bias2_dinc_reg, \
              rf_bias2_dinc_reg_bias2_din18_shift, \
              rf_bias2_dinc_reg_bias2_din18_mask, \
              biasBist1);
}


/**
 * @brief   bist gpadc.
 * @param   gpadcBias1test: DAC24 data: RF bias iout24 for GPADC test
 * @param   gpadcBias2test: BB bias iout20 for GPADC test
 * @param   gpadcBias: GPADC bias
 * @return  None.
 * @note
 */
void biasGpadc(int32 gpadcBias1test, int32 gpadcBias2test, int32 gpadcBias)
{
    SET_VALUE(RFC->rf_bias1_dinc_reg, \
              rf_bias1_dinc_reg_bias1_din23_shift, \
              rf_bias1_dinc_reg_bias1_din23_mask, \
              gpadcBias1test);

    SET_VALUE(RFC->rf_bias2_dinc_reg, \
              rf_bias2_dinc_reg_bias2_din16_shift, \
              rf_bias2_dinc_reg_bias2_din16_mask, \
              gpadcBias);

    SET_VALUE(RFC->rf_bias2_dinc_reg, \
              rf_bias2_dinc_reg_bias2_din19_shift, \
              rf_bias2_dinc_reg_bias2_din19_mask, \
              gpadcBias2test);
}


/**
 * @brief   Enable bist.
 * @param   bistSorSel: bist source select. 1 : from RF 0: from baseband.
 * @param   bistDivSel: bist divided frequency from 50MHz.
 * @return  None.
 * @note
 */
void bistOn(uint16 bistSorSel, uint16 bistDivSel)
{
    SET_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_xo50m_en_ckbist_bit);

    delayUs(100);
    
    //modified by Diep, from 2022.06.08 email. 
    biasBist(10, 10, 1, 4);

    if(bistSorSel == 1)
    {
        //BIST_EN_ONOFF_AMP ON
        WRITE_REG(RFC->rf_bist_cfg_reg, 0x07 | bistDivSel << 8 | 0x01 << 16 | 0x00 << 24);
    }
    else
    {
        //BIST_RX0_ABB_EN BIST_RX1_ABB_EN ON
        WRITE_REG(RFC->rf_bist_cfg_reg, 0x35 | bistDivSel << 8 | 0x01 << 16 | 0x00 << 24);
    }
    WRITE_REG(RFC->rf_bist_load, 1);
}

/**
 * @brief   Disable bist.
 * @param   bistSorSel: bist source select. 1 : from RF 0: from baseband.
 * @param   bistDivSel: bist divided frequency from 50MHz.
 * @return  None.
 * @note
 */
void bistOff(uint16 bistSorSel, uint16 bistDivSel)
{
    CLEAR_BIT(RFC->rfc_plli_ctrl_reg, rfc_plli_ctrl_reg_xo50m_en_ckbist_bit);

    delayUs(100);
    
    biasBist(10, 10, 1, 4);
    
    WRITE_REG(RFC->rf_bist_cfg_reg, 0x00 | 0x21 << 8 | 0x01 << 16| 0x00 << 24);

    WRITE_REG(RFC->rf_bist_load, 0);
}

/**
 * @brief   60G buffer enable.
 * @param   stage1Buff60G: 60G buffer 1st stage enable.
 * @param   tx0BufferEn60G: tx0 buffer enable.
 * @param   tx1BufferEn60G: tx1 buffer enable.
 * @param   rx0BufferEn60G: rx0 buffer enable.
 * @param   rx1BufferEn60G: rx1 buffer enable.
 * @return  None.
 * @note
 */
void rfBuff60GEnable(const stateTypedef stage1Buff60G, \
                     const stateTypedef tx0BufferEn60G, const stateTypedef tx1BufferEn60G,
                     const stateTypedef rx0BufferEn60G, const stateTypedef rx1BufferEn60G)
{
    if(ENABLE == stage1Buff60G)
    {
        SET_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_bit);
    }
    else if(DISABLE == stage1Buff60G)
    {
        CLEAR_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_bit);
    }

    if(ENABLE == tx0BufferEn60G)
    {
        SET_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_tx0_bit);
    }
    else if(DISABLE == tx0BufferEn60G)
    {
        CLEAR_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_tx0_bit);
    }

    if(ENABLE == tx1BufferEn60G)
    {
        SET_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_tx1_bit);
    }
    else if(DISABLE == tx1BufferEn60G)
    {
        CLEAR_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_tx1_bit);
    }

    if(ENABLE == rx0BufferEn60G)
    {
        SET_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_rx0_bit);
    }
    else if(DISABLE == rx0BufferEn60G)
    {
        CLEAR_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_rx0_bit);
    }

    if(ENABLE == rx1BufferEn60G)
    {
        SET_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_rx1_bit);
    }
    else if(DISABLE == rx1BufferEn60G)
    {
        CLEAR_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_rx1_bit);
    }
    
    
    /* add control 60G buffer stability for adt6101P */

    //rf_plli_rega_reg: 0x40003164: 0x3F011FFFF
    uint32 regVel = 0;
    
    regVel = RFC->rf_plli_rega_reg;
    
    if((rx1BufferEn60G | rx0BufferEn60G | tx1BufferEn60G | tx0BufferEn60G) == 1)
    {
        regVel |= (0x1FF << rf_plli_rega_reg_buf60g_stab_ctrl_shift);
    } else 
    {
        regVel &= ~(0x1FF << rf_plli_rega_reg_buf60g_stab_ctrl_shift);
    }
    RFC->rf_plli_rega_reg = regVel;
    
}


/**
 * @brief   Set the tx0 buffer enable.
 * @param   en: ENABLE/DISABLE.
 * @return  None.
 * @note    Switch channels without using it.
 */
void set60GBuffTx0En(const stateTypedef tx0En)
{
    if(ENABLE == tx0En)
    {
        SET_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_tx0_bit);
    }
    else if(DISABLE == tx0En)
    {
        CLEAR_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_tx0_bit);
    }
}

/**
 * @brief   Set the tx1 buffer enable.
 * @param   en: ENABLE/DISABLE.
 * @return  Switch channels without using it..
 * @note
 */
void set60GBuffTx1En(const stateTypedef tx1En)
{
    if(ENABLE == tx1En)
    {
        SET_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_tx1_bit);
    }
    else if(DISABLE == tx1En)
    {
        CLEAR_BIT(RFC->rf_buf77g_ctrl_reg, rf_buf77g_ctrl_reg_buf60g_en_tx1_bit);
    }
}


/**
 * @brief   Get the tx0/tx1 buffer enable.
 * @return  1: Enable, 0: Disable.
 * @note
 */
uint8 get60GBuffTxEn(void)
{
    return GET_BIT_VALUE(RFC->rf_buf77g_ctrl_reg, BIT_2) | \
           (GET_BIT_VALUE(RFC->rf_buf77g_ctrl_reg, BIT_3) << 1);
}



/*-------------------------PA-----------------------------*/
/**
 * @brief   control pa enable.
 * register*RW*rfc_tx_pa_reg
 * [31:11]  21'h0   Reserved
 * [10:8]   3'h0    rf_atb_sel:atb select
 * [7:2]    6'h0    rf_atb_en:atb enable:<1:0> TX pwr en;<2> RX pwr en; <3> 77G buf atb enable;<4> 20G atb en;<5> db atb enable
 * [1]      1'h0    pa1_en:pa1 enable
 * [0]      1'h0    pa0_en:pa0 enable
 * @param   pa0Enable: channel0 DAC bias1.
 * @param   pa1Enable: channel0 DAC bias2.
 * @return  None.
 * @note
 */
void txPaEnable(const stateTypedef pa0Enable, const stateTypedef pa1Enable)
{
    if(pa0Enable == 1)
    {
        SET_BIT(RFC->rfc_tx_pa_reg, rfc_tx_pa_reg_pa0_en_bit);
    }
    else
    {
        CLEAR_BIT(RFC->rfc_tx_pa_reg, rfc_tx_pa_reg_pa0_en_bit);
    }

    if(pa1Enable == 1)
    {
        SET_BIT(RFC->rfc_tx_pa_reg, rfc_tx_pa_reg_pa1_en_bit);
    }
    else
    {
        CLEAR_BIT(RFC->rfc_tx_pa_reg, rfc_tx_pa_reg_pa1_en_bit);
    }
}


/**
 * @brief   control dac pa bias.
 * register*RW*rfc_bias1_dina_reg
 * [7:4]    4'h8    bias1_din1:DAC2 data, ch1 PA
 * [3:0]    4'h8    bias1_din0:DAC1 data, ch0 PA
 * @param   pa0Bias: channel0 PA bias1.
 * @param   pa1Bias: channel0 PA bias2.
 * @return  None.
 * @note
 */
void txPaBias(const uint8 pa0Bias, const uint8 pa1Bias)
{
    SET_VALUE(RFC->rfc_bias1_dina_reg, \
              rfc_bias1_dina_reg_bias1_din1_shift, \
              rfc_bias1_dina_reg_bias1_din1_mask, \
              pa1Bias);

    SET_VALUE(RFC->rfc_bias1_dina_reg, \
              rfc_bias1_dina_reg_bias1_din0_shift, \
              rfc_bias1_dina_reg_bias1_din0_mask, \
              pa0Bias);
}


/**
 * @brief   phase shifter data setting.
 * @param   ps_ip: phase shifter i posedge.
 * @param   ps_in: phase shifter i negedge.
 * @param   ps_qp: phase shifter q posedge.
 * @param   ps_qn: phase shifter q negedge.
 * @return  None.
 * @note
 */
// PhaseShift_1:  Direct mode.
void rfPhaseShift(int32 ps_ip, int32 ps_in, int32 ps_qp, int32 ps_qn)
{
    SET_VALUE(RFC->rfc_phase_shift_reg, \
              rfc_phase_shift_reg_ps_qn_shift, \
              rfc_phase_shift_reg_ps_qn_mask, \
              ps_qn);
    SET_VALUE(RFC->rfc_phase_shift_reg, \
              rfc_phase_shift_reg_ps_qp_shift, \
              rfc_phase_shift_reg_ps_qp_mask, \
              ps_qp);
    SET_VALUE(RFC->rfc_phase_shift_reg, \
              rfc_phase_shift_reg_ps_in_shift, \
              rfc_phase_shift_reg_ps_in_mask, \
              ps_in);
    SET_VALUE(RFC->rfc_phase_shift_reg, \
              rfc_phase_shift_reg_ps_ip_shift, \
              rfc_phase_shift_reg_ps_ip_mask, \
              ps_ip);
    delayUs(1);
    SET_BIT(RFC->rfc_nload_reg, rfc_nload_reg_ps0_en_inv_bit);
    SET_BIT(RFC->rfc_nload_reg, rfc_nload_reg_ps1_en_inv_bit);
    delayUs(1); /* Must have delay before loading. */
    SET_BIT(RFC->rfc_nload_reg, rfc_nload_reg_nload_inv_bit);
    /* Notice, RF's PhaseShift register is powered by VDD_RF. So when VDD_RF is powered off, the value will lose.
        And when VDD_RF is turned on, PhaseShift will be configured again. */
}

// PhaseShift_2:  LUT mode.
void rfPhaseShiftLUT(uint8 ch0_idx, uint8 ch1_idx)
{
    /* ch0_idx and ch1_idx belongs 0~63. they are indexs of phase shift LUT table. */
    /* enable LUT mode. */
    SET_BIT(RFC->rfc_phase_shift_reg, rfc_phase_shift_reg_ps_auto_en_bit);
    delayUs(1); /* Must have delay after writing rfc_phase_shift_reg_ps_auto_en_bit. */

    SET_VALUE(RFC->rf_psr0_reg, \
              rf_psr0_reg_psr0_shift, \
              rf_psr0_reg_psr0_mask, \
              ch0_idx);
    SET_VALUE(RFC->rf_psr1_reg, \
              rf_psr1_reg_psr1_shift, \
              rf_psr1_reg_psr1_mask, \
              ch1_idx);
    delayUs(1);  /* Must have delay before loding. */
    SET_BIT(RFC->rfc_nload_reg, rfc_nload_reg_nload_inv_bit);
}


/*------------------------FMCW----------------------------*/
/**
 * @brief   temperature compensation.
 * @param   miniBais: .
 * @param   kc1Reg: .
 * @param   kc2Reg: .
 * @return  None.
 * @note
 */
void temperatureCompensation(uint32 miniBais, uint32 kc1Reg, uint32 kc2Reg)
{
    uint32 regVel = 0;
    regVel = RFC->rf_bias1_regs_reg;
    regVel |= miniBais << 14 | kc1Reg << 8 | kc2Reg << 11;
    RFC->rf_bias1_regs_reg = regVel;
}


/**
 * @brief   fmcw enable mode.
 * @param   startMode: fmcw_en select. 1'h1: from data_sample module. 1'h0: from register.
 * @param   chirpMode: Chirp mode. 1 : single mode. 0 : continue mode.
 * @return  None.
 * @note
 */
void fmcwEnmode(const fmcwEnSelectTypedef startMode, const fmcwModeTypedef chirpMode)
{
    /* Every time reset the FMCW mode, must first clear these 3 bits!!!,
        otherwise, it will result in poor waveform quality during the first FMCW. */
    CLEAR_BIT(RFC->fmcw_reg0_reg,                       \
              fmcw_reg0_reg_rst_fmcw_n_bit |          \
              fmcw_reg0_reg_chirp_single_mode_bit |   \
              fmcw_reg0_reg_fmcw_en_bit);

    /* Start fmcw chirp with single_mode. */
    if(SINGLE_MODE == chirpMode)
    {
        SET_BIT(RFC->fmcw_reg0_reg, fmcw_reg0_reg_chirp_single_mode_bit);
    }
    else if(CONTINUE_MODE == chirpMode)
    {
        CLEAR_BIT(RFC->fmcw_reg0_reg, fmcw_reg0_reg_chirp_single_mode_bit);
    }

    SET_BIT(RFC->fmcw_reg0_reg, fmcw_reg0_reg_rst_fmcw_n_bit);
    SET_BIT(RFC->fmcw_reg0_reg, fmcw_reg0_reg_dsm_divn_negedge_bit);

    /* configure FMCW trigger signal as ADC start signal. */
    if(DATA_SAMPLE_MODULE == startMode)
    {
        SET_BIT(RFC->fmcw_reg7_reg, fmcw_reg7_reg_fmcw_en_from_ds_bit);
    }
    else if(REGISTER == startMode)
    {
        SET_BIT(RFC->fmcw_reg0_reg, fmcw_reg0_reg_fmcw_en_bit);
        CLEAR_BIT(RFC->fmcw_reg7_reg, fmcw_reg7_reg_fmcw_en_from_ds_bit);
    }
}

void waitFmcwcycEnd(void)
{
    while((*(uint32 *)0x40003930) == 0);
}


/**
 * @brief   Set the starting frequency of FMCW.
 * @param   fstart: starting frequency.
 * @return  None.
 * @note
 */
void setFmcwStartFreq(const uint32 fstart)
{
    SET_VALUE(RFC->fmcw_reg12_reg,                      \
              fmcw_reg12_reg_freq_start_lsb_shift,    \
              fmcw_reg12_reg_freq_start_lsb_mask,     \
              (fstart & 0xFF));
    SET_VALUE(RFC->fmcw_reg13_reg,                      \
              fmcw_reg13_reg_freq_start_mid_shift,    \
              fmcw_reg13_reg_freq_start_mid_mask,     \
              ((fstart >> 8) & 0xff));
    SET_VALUE(RFC->fmcw_reg14_reg,                      \
              fmcw_reg14_reg_freq_start_msb_shift,    \
              fmcw_reg14_reg_freq_start_msb_mask,     \
              ((fstart >> 16) & 0x7F));
}

/**
 * @brief   Get the starting frequency of FMCW.
 * @return  Starting frequency.
 * @note
 */
uint32 getFmcwStartFreq(void)
{
    return GET_BITS_VALUE(RFC->fmcw_reg12_reg,              \
                          fmcw_reg12_reg_freq_start_lsb_shift,        \
                          fmcw_reg12_reg_freq_start_lsb_mask) |       \
           (GET_BITS_VALUE(RFC->fmcw_reg13_reg,            \
                           fmcw_reg13_reg_freq_start_mid_shift,        \
                           fmcw_reg13_reg_freq_start_mid_mask) << 8) | \
           (GET_BITS_VALUE(RFC->fmcw_reg14_reg,            \
                           fmcw_reg14_reg_freq_start_msb_shift,        \
                           fmcw_reg14_reg_freq_start_msb_mask) << 16);
}


/**
 * @brief   tdMimo.
 *          t0,t1       != 0  channelNum=2
 *          t0,t1,t2    != 0  channelNum=3
 *          t0,t1,t2,t3 != 0  channelNum=4
 * @param   t0:.
 * @param   t1:.
 * @param   t2:.
 * @return  None.
 * @note
 */
void tdMimo(float t0, float t1, float t2)
{
    uint8 channelNum;
    txPaEnable(DISABLE, DISABLE);
    if(t1 == 0)
    {
        channelNum = 1;
    }
    else if(t2 == 0)
    {
        channelNum = 2;
    }
    else
    {
        channelNum = 3;
    }
    RFC->fmcw_mimo_tx0_on = (int32)pow(2, channelNum) - 1;
    RFC->fmcw_mimo_tx1_on = ((int32)pow(2, channelNum) - 1) << channelNum;

    RFC->fmcw_mimo_ctrl &= ~0xff;
    RFC->fmcw_mimo_ctrl |= 1 << 16 | channelNum * 2;
}


/**
 * @brief   Enable dither for delta-sigma modulator.
 * @param   en: ENABLE/DISABLE.
 * @return  1: Success, 0: Failure.
 * @note
 */
uint8 setEnableDitherForDSM(const stateTypedef en)
{
    uint8 res = 0;
    if(ENABLE == en)
    {
        res = 1;
        SET_BIT(RFC->fmcw_reg0_reg, fmcw_reg0_reg_dsm_dither_en_bit);
    }
    else if(ENABLE == DISABLE)
    {
        res = 1;
        CLEAR_BIT(RFC->fmcw_reg0_reg, fmcw_reg0_reg_dsm_dither_en_bit);
    }

    return res;
}

/**
 * @brief   Get the status of the dither for delta module.
 * @return  1 : Enable. 0 : Disable.
 * @note
 */
uint8 getEnableDitherForDSM(void)
{
    return GET_BIT_VALUE(RFC->fmcw_reg0_reg, BIT_4);
}


/**
 * @brief   Set channle number in one chirp scheme.
 * @param   num: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @return  None.
 * @note
 */
void setChirpChannelNum(const chirpChTypedef num)
{
    SET_VALUE(RFC->fmcw_reg6_reg, \
              fmcw_reg6_reg_chirp_channel_num_shift, \
              fmcw_reg6_reg_chirp_channel_num_mask, \
              (uint8)num);
}

/**
 * @brief   Get channle number in one chirp scheme.
 * @param   num: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @return  How many channels have been set.
 * @note
 */
uint8 getChirpChannelNum(void)
{
    return GET_BITS_VALUE(RFC->fmcw_reg6_reg, \
                          fmcw_reg6_reg_chirp_channel_num_shift, \
                          fmcw_reg6_reg_chirp_channel_num_mask);
}


/**
 * @brief   Setting the number of chirps if chirp_single_mode=1, call fmcwEnmode(..., SINGLE_MODE).
 * @param   num: There are a total of 2^16 cycle.
 * @return  None.
 * @note    The fmcwEnmode(..., SINGLE_MODE) function must be called to set the number of chirp cycles.
 */
void setChirpCycleNum(const uint16 num)
{
    SET_VALUE(RFC->fmcw_reg4_reg, \
              fmcw_reg4_reg_chirp_cycle_number_lsb_shift, \
              fmcw_reg4_reg_chirp_cycle_number_lsb_mask, \
              (num & 0xFF));

    SET_VALUE(RFC->fmcw_reg5_reg, \
              fmcw_reg5_reg_chirp_cycle_number_msb_shift, \
              fmcw_reg5_reg_chirp_cycle_number_msb_mask, \
              ((num >> 8) & 0xFF));
}

/**
 * @brief   Get the number of chirps.
 * @return  Number of chirps.
 * @note
 */
uint16 getChirpCycleNum(void)
{
    return GET_BITS_VALUE(RFC->fmcw_reg4_reg,                   \
                          fmcw_reg4_reg_chirp_cycle_number_lsb_shift,     \
                          fmcw_reg4_reg_chirp_cycle_number_lsb_mask) |    \
           (GET_BITS_VALUE(RFC->fmcw_reg5_reg,                 \
                           fmcw_reg5_reg_chirp_cycle_number_msb_shift,     \
                           fmcw_reg5_reg_chirp_cycle_number_msb_mask) << 8);
}


/**
 * @brief   Set the number of chirp channel steps.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @param   numStep: chirp channel steps.
 * @return  None.
 * @note
 */
void setChirpChannelStepNum(const chirpChTypedef chIndex, const uint32 numStep)
{
    SET_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x210)),   \
              fmcw_reg16_reg_ch0_step_num_lsb_shift,               \
              fmcw_reg16_reg_ch0_step_num_lsb_mask,                \
              (numStep));
    SET_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x211)),   \
              fmcw_reg17_reg_ch0_step_num_mid_shift,               \
              fmcw_reg17_reg_ch0_step_num_mid_mask,                \
              (numStep >> 8));

    SET_BIT(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x212)), ((numStep >> 16) & 0x01));
}

/**
 * @brief   Get the number of chirp channel steps.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @return  Channel step.
 * @note
 */
uint32 getChirpChannelStepNum(const chirpChTypedef chIndex)
{
    return GET_BITS_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x210)),   \
                          fmcw_reg16_reg_ch0_step_num_lsb_shift,              \
                          fmcw_reg16_reg_ch0_step_num_lsb_mask) |             \
           (GET_BITS_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x211)), \
                           fmcw_reg17_reg_ch0_step_num_mid_shift,              \
                           fmcw_reg17_reg_ch0_step_num_mid_mask) << 8) |       \
           (GET_BIT_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x212)),  \
                          BIT_0) << 16);
}


/**
 * @brief   Set the number of DT steps for the chirp channel.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @param   dtStep: dt step.
 * @return  ch0_dt_step's MSB. Every step's time = (ch0_dt_step[7:0]+1)*4ns, when reference clock is 250mhz, which means 4ns for one cycle.
 *              dt_step[7:0] MUST >= 8'h1, that means the time of every step MUST be 8ns at least.
 * @note
 */
void setChirpChannelDtStep(const chirpChTypedef chIndex, const uint8 dtStep)
{
    SET_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x212)),   \
              fmcw_reg18_reg_ch0_dt_step_lsb_shift,               \
              fmcw_reg18_reg_ch0_dt_step_lsb_mask,                \
              (dtStep));
    SET_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x213)),   \
              fmcw_reg19_reg_ch0_dt_step_msb_shift,               \
              fmcw_reg19_reg_ch0_dt_step_msb_mask,                \
              (dtStep >> 4));
}

/**
 * @brief   Get the number of DT steps for the chirp channel.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @return  Dt step.
 * @note
 */
uint8 getChirpChannelDtStep(const chirpChTypedef chIndex)
{
    return GET_BITS_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x212)),   \
                          fmcw_reg18_reg_ch0_dt_step_lsb_shift,                           \
                          fmcw_reg18_reg_ch0_dt_step_lsb_mask) |                          \
           (GET_BITS_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x213)), \
                           fmcw_reg19_reg_ch0_dt_step_msb_shift,                           \
                           fmcw_reg19_reg_ch0_dt_step_msb_mask) << 4);
}


/**
 * @brief   Set the number of DF steps for the chirp channel.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @param   dfStep: df step.
 * @return  ch0_df_step's MSB. Every step's frequency increase = df_step[11:0]/2^18*2 (Ghz).
 * @note
 */
void setChirpChannelDfStep(const chirpChTypedef chIndex, const uint16 dfStep)
{
    SET_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x213)),   \
              fmcw_reg19_reg_ch0_df_step_lsb_shift,               \
              fmcw_reg19_reg_ch0_df_step_lsb_mask,                \
              (dfStep));
    SET_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x214)),   \
              fmcw_reg20_reg_ch0_df_step_msb_shift,               \
              fmcw_reg20_reg_ch0_df_step_msb_mask,                \
              (dfStep >> 4));
}

/**
 * @brief   Get the number of DF steps for the chirp channel.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @return  DF step.
 * @note
 */
uint16 getChirpChannelDfStep(const chirpChTypedef chIndex)
{
    return GET_BITS_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x213)),   \
                          fmcw_reg19_reg_ch0_df_step_lsb_shift,                           \
                          fmcw_reg19_reg_ch0_df_step_lsb_mask) |                          \
           (GET_BITS_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x214)), \
                           fmcw_reg20_reg_ch0_df_step_msb_shift,                           \
                           fmcw_reg20_reg_ch0_df_step_msb_mask) << 4);
}


/**
 * @brief   Set the number of DF steps for the chirp channel.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @param   chMode:  0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.
 * @return  None.
 * @note
 */
void setChirpChannelMode(const chirpChTypedef chIndex, const chirpChModeTypedef chMode)
{
    SET_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x212)),    \
              fmcw_reg18_reg_ch0_mode_shift, \
              fmcw_reg18_reg_ch0_mode_mask, \
              (uint8)chMode);
}

/**
 * @brief   Get the number of DF steps for the chirp channel.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @return  0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.
 * @note
 */
uint8 getChirpChannelMode(const chirpChTypedef chIndex)
{
    return GET_BITS_VALUE(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x212)),    \
                          fmcw_reg18_reg_ch0_mode_shift, \
                          fmcw_reg18_reg_ch0_mode_mask);
}


/**
 * @brief   clear dsm's accumulator when this channel start.
 * @param   chIndex: There are a total of 12 channels(CHIRP_CHANNEL_1 ~ CHIRP_CHANNEL_11).
 * @param   clear:  0 : CHIRP_CH_NOT_CLEAR. 1 : CHIRP_CH_CLEAR.
 * @return  None.
 * @note
 */
void chirpChannelClear(const chirpChTypedef chIndex, const chirpChClearTypedef clear)
{
    if(CHIRP_CH_CLEAR == clear)
    {
        SET_BIT(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x212)), \
                fmcw_reg18_reg_ch0_clear_bit);
    }
    else if(CHIRP_CH_NOT_CLEAR == clear)
    {
        CLEAR_BIT(*((uint32 *)(RFC_REG + (uint8)chIndex * 5 + 0x212)), \
                  fmcw_reg18_reg_ch0_clear_bit);
    }
}


/**
 * @brief   Set the type of MIMO.
 * @param   mode: 0 : NO_MIMO. 1 : TD_MIMO. 2 : BP_MIMO.
 * @param   chirpChBits: Chirp channel bits.
 * @param   chirpChNum: Chirp channel numbers.
 * @return  1: Success, 0: Failure..
 * @note
 */
uint8 setMimoType(const mimoModeTypedef mode, const uint32 chirpChBits, const uint32 chirpChNum)
{
    uint8 res = 1;
    switch(mode)
    {
        case NO_MIMO:
        {
            CLEAR_VALUE(RFC->fmcw_mimo_ctrl);
            break;
        }
        case TD_MIMO:
        {
            RFC->fmcw_mimo_tx0_on = chirpChBits;
            RFC->fmcw_mimo_tx1_on = chirpChBits << chirpChNum;
            SET_BIT(RFC->fmcw_mimo_ctrl, fmcw_mimo_ctrl_auto_tx_on_en_bit);
            //chirpChNum*2.
            SET_VALUE(RFC->fmcw_mimo_ctrl, \
                      fmcw_mimo_ctrl_tx_on_period_shift, \
                      fmcw_mimo_ctrl_tx_on_period_mask, \
                      (chirpChNum << 1));
            break;
        }
        case BP_MIMO:
        {
            CLEAR_VALUE(RFC->fmcw_mimo_tx0_psr);
            RFC->fmcw_mimo_tx1_psr = chirpChBits << chirpChNum;
            SET_BIT(RFC->fmcw_mimo_ctrl, fmcw_mimo_ctrl_auto_tx_psr_en_bit);
            //chirpChNum*2.
            SET_VALUE(RFC->fmcw_mimo_ctrl, \
                      (fmcw_mimo_ctrl_tx_psr_period_shift + 1), \
                      ((1 << 9) - 1), \
                      chirpChNum);
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
 * @brief   Set FMCW to single tone mode. Tone frequency is FMCW start frequency.
 * @param   None.
 * @return  None.
 * @note
 */
void fmcwChirpSingleFrequency(void)
{
    /* set a single frequency, Only for test purpose. */
    WRITE_REG(RFC->fmcw_reg20_reg, 0x0);    /* (ch0_df_step &0xFF)>>4;, ch0 df_step */

    WRITE_REG(RFC->fmcw_reg25_reg, 0x0);    /* (ch1_df_step &0xFF)>>4; ch1 df_step */

    WRITE_REG(RFC->fmcw_reg30_reg, 0x0);    /* (ch2_df_step &0xFF)>>4; ch2 df_step */

    WRITE_REG(RFC->fmcw_reg18_reg, 0xfd);    /* ch0's clear [1] = 0. */
    WRITE_REG(RFC->fmcw_reg23_reg, 0xfd);    /* ch1's clear [1] = 0. */
    WRITE_REG(RFC->fmcw_reg28_reg, 0xfd);    /* ch2's clear [1] = 0. */
}

/**
 * @brief   select P27 as widen_bw signal, which is an indicator for CHIRP_T0.
            It will be 1 when transmit CHIRP_T0, and it is synchornized with CHIRP_T0.
            This signal could be used as an indicator synchornized with CHIRP_T0.
 * @param   chirpT0: Rising edge time.
 * @return  None.
 * @note
 */
void widenBwP27(float chirpT0)
{
    /* disable en_buffer1 contorl for if signal output gpio27, 28, 29, 30. */
    RFC->rfc_vga1_reg &= ~(1 << 9);

    /* set P27 as widen_bw signal.
     WRITE_REG(*((uint32 *)(GPIO_REGS + 27 + 12)), 0x2 | 1 << 9 | 1 << 10);//p27 as widen_bw. */

    /* calculate T0's time. */
    uint32 widenBwWidth;
    widenBwWidth = (chirpT0 * 1000) / 8 - 1;
    WRITE_REG(RFC->fmcw_reg8_reg, widenBwWidth & 0xff);
    WRITE_REG(RFC->fmcw_reg9_reg, (widenBwWidth >> 8) & 0xff);

    /* assume the chirp is typical T0/T1/T2 chirp, so T0 would be channel 0. */
    WRITE_REG(RFC->fmcw_reg10_reg, 0x01);
    WRITE_REG(RFC->fmcw_reg11_reg, 0x0);
}

/**
 * @brief   get Fmcw Int State.
 * @param   None.
 * @return  fmcw int state.
 */
uint32 getFmcwIntState(void)
{
    return RFC->fmcw_int_state;
}

/**
 * @brief   FMCW chirp generator interrupt clear.
 * @param   None.
 * @return  fmcw int state.
 */
void clearFmcwInt(uint32 state)
{
    WRITE_REG(RFC->fmcw_int_clear, state);
}
