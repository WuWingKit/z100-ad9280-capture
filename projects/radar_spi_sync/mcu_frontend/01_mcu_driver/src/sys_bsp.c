/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    sys_bsp.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file references SYSTEM HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "sys_bsp.h"
#include "tx_bsp.h"
#include "gpio_bsp.h"
#include "rtc_bsp.h"
#include "timer_bsp.h"
#include "adt6101P_tx.h"
#include "adt6101P_rx.h"
#include "adt6101P.h"
#include "adt6101P_gpio.h"
#include "adt6101P_pmu.h"
#include "lowpower_bsp.h"

#include "gpio_ctype_map.h"
#include "rfc_ctype_map.h"

/* Private variables ---------------------------------------------------------*/
volatile sysHighClkTypedef g_sysHighClk = SYS_HIGH_CLK_50M;

/* Exported functions --------------------------------------------------------*/
/**
 * @brief   initial high clock, sleep config, rtc and calibration, reset phase shifter.
 * @param   None.
 * @return  None.
 */
void adt6101PDigiInit(void)
{
#if CHIRP_LOWPOWER
    // enable the channel's interrupt for FMCW chirp.
    RFC->fmcw_int_en = 0x6; // 0x1 for T0_start interrupt, 0x2 for T1_start. 0x4 for T2_start. 
#else
    RFC->fmcw_int_en = 0x2;
#endif

// if not in cascade appliaction, use ds_start to start fmcw and adc sampling.
// if in cascade appliaction, and as a master, use sync_out to start fmcw and adc sampling.
// if in cascade appliaction, and as a slave,  wait external sync_in to start fmcw and adc sampling.
#ifdef CASCADE_MASTER
    GPIO->gpio_p02_config = 0x33;   //as sync_in.
    GPIO->gpio_p09_config = 0x3;   //as sync_out.
    RFC->fmcw_fmcw_sync = fmcw_fmcw_sync_adc_start_from_syncin_bit | \
                           fmcw_fmcw_sync_en_ckadc_from_syncin_bit | \
                            fmcw_fmcw_sync_negedge_sel_bit;
#endif
    
#ifdef CASCADE_SLAVE
    GPIO->gpio_p02_config = 0x33;   //as sync_in.
    RFC->fmcw_fmcw_sync = fmcw_fmcw_sync_adc_start_from_syncin_bit | \
                            fmcw_fmcw_sync_en_ckadc_from_syncin_bit;
#endif

    /* MUST enable all of clock gating cells. Otherwise, the sleep will fail. */
    for(uint8 i = (uint8)CLK_GATING_PMU_APB; i < (uint8)CLK_GATING_ROM; ++i)
    {
        setExtClkEn(DISABLE, (extClkTypedef)i);
    }

    rtcInit(ENABLE, CLK_32k_RCSOR);
    rc32kCalibration();
    // when VDD_RF is on, reset phase shifter.
    RFC->rfc_nload_reg =  rfc_nload_reg_ps_rstn_inv_bit;
    
    /* manu_ctrl_en = 1, only sleep mode switching. */
    setManualCtrlToLdoAndClk(ENABLE);
}


/**
 * @brief   initial tx and rx and psr.
 * @param   pDataStr: analog RF and IF structure, see also @ref AnalogDataStrTypedef.
 * @param   chirpNumber: Configure the number of chirps for FMCW.
 * @return  None.
 */
void adt6101PRfInit(const AnalogDataStrTypedef *pDataStr, const uint32 chirpNumber)
{
    uint8 buffer20gEn = pDataStr->rx0En  | \
                        pDataStr->rx1En | \
                        pDataStr->tx0En | \
                        pDataStr->tx1En;
    uint8 doublerEn = pDataStr->rx0En    | \
                      pDataStr->rx1En | \
                      pDataStr->tx0En | \
                      pDataStr->tx1En;
    plliDacSetting();
    pllnConfig(ENABLE);

    pllLock(FSTART, FM, chirpNumber);
    fmcwConfig(FSTART, FM, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);

    RFC->rf_spare_reg = 0x01;

    /* set fmcw pll to doubler */
    fmcwModeSel(MODE_CASCADED);

    /* enable 20G and doubler */
    rfbuff20G((stateTypedef)buffer20gEn, (stateTypedef)doublerEn);
    
    /* config mimo mode
       in case TD_MIMO enabled, the MIMO field in rfc_rx_lna_reg shoule be enabled. */
    txMIMO(pDataStr->txMimoEn);
    
    /* enable buffer1to4 */
    rfBuff60GEnable(DISABLE, DISABLE, DISABLE, DISABLE, DISABLE);
    /* enable bias for 20G(fmcw buffer), doubler and buffer1to4 */
    bias20GTo60G(DISABLE, DISABLE, DISABLE, 8, 8, DISABLE, DISABLE, DISABLE);
}


/**
 * @brief   ADT6101P analog initialization.
 * @param   pDataStr: analog RF and IF structure, see also @ref AnalogDataStrTypedef.
 * @return  None.
 */
void adt6101PAnalog(const AnalogDataStrTypedef *pDataStr)
{
    int32 biasRx0 = 0;
    int32 biasRx1 = 0;
    
    int32 biasRxLna0 = 0;
	int32 biasRxLna1 = 0;

    /* enable pa1 and pa0 */
    txPaEnable(pDataStr->tx0En, pDataStr->tx1En);
    txPaBias(pDataStr->PaGain0, pDataStr->PaGain1);

    rxLnaEnable(pDataStr->rx0En, pDataStr->rx1En);
    mixerEnable(DISABLE, DISABLE);
    //lnaRegularBias(pDataStr->rx0En, pDataStr->rx1En);
    lnaDacBias(CH0_LNA_BIAS1, biasRxLna0);
    lnaDacBias(CH0_LNA_BIAS2, biasRxLna0);
    lnaDacBias(CH0_LNA_BIAS3, biasRxLna0);
    lnaDacBias(CH1_LNA_BIAS1, biasRxLna1);
    lnaDacBias(CH1_LNA_BIAS2, biasRxLna1);
    lnaDacBias(CH1_LNA_BIAS3, biasRxLna1);

    /* bias2_en: enable signal for ztc bias for BB */
    baseBandBiasSetting(ENABLE);

    /* turn on tia pga */
    rfTiaEnable(DISABLE, DISABLE);
    rfTiaGainSetting(TIA_GAIN_0DB, TIA_GAIN_0DB, TIA_GAIN_0DB, TIA_GAIN_0DB);
    rfTiaDacBias(biasRx0, biasRx1);
    
    /* pga config */
    rfVgaEnable(pDataStr->rx0En, pDataStr->rx1En);
    rfVgaGainSetting(pDataStr->pgaGain0, pDataStr->pgaGain0, pDataStr->pgaGain1, pDataStr->pgaGain1);
    rfVgaDacBias(biasRx0, biasRx1);
    rfPgaGainCalEnable(DISABLE, DISABLE, DISABLE, DISABLE);
    rfPgaGainCalSetting(PGA_CAL_P1, PGA_CAL_P1, PGA_CAL_P1, PGA_CAL_P1);
    
    /* filter config */
    tiaHpfConfig(pDataStr->tiaHpfEn, pDataStr->tiaHpfEn, pDataStr->tiaHpf, pDataStr->tiaHpf);
    rcHpfConfig(pDataStr->rcHpfEn, pDataStr->rcHpfEn, pDataStr->rcHpf, pDataStr->rcHpf);
    pgaLpfConfig(pDataStr->pgaLpf, pDataStr->pgaLpf);
    pgaHpfConfig(pDataStr->pgaHpfEn, pDataStr->pgaHpfEn, pDataStr->pgaHpf, pDataStr->pgaHpf);

    #if ANALOG_IF_OUTPUT
    /* enable vga en_buffer */
    RFC->rfc_vga0_reg |= 1 << 9;
    /*en_buffer0 contorl if signal output gpio23, 24, 25, 26.
        en_buffer1 contorl if signal output gpio27, 28, 29, 30.
        when we test low power consumption, we need to connect p29 with vddrf enable.
        Normally, p29 is high resistance, vga1 en_buffer will lower the p29 level. */
    RFC->rfc_vga1_reg |= 1 << 9;
    #endif

    /* gpio mux to output bist if signal */
    GPIO->gpio_p24_config = 0x16;
    GPIO->gpio_p25_config = 0x16;
}
/**
 * @brief   adjust the calibration parameters to make rc32KHz close to 32.768KHz.
 *          rf_rc32k_reg_reg is a (sign+abs) format value to control rc32k's frequency.
 *             for example, 0x3f is the smallest frequency.  0x1f is the largest frequency.
 *             0x20 is close to 0x0, but smaller than 0x0.
 * @param   None.
 * @return  None.
 */
void rc32kCalibration(void)
{
    int i;
    uint32 regVal;
    uint32 index;

    setExtClkEn(DISABLE, CLK_GATING_CNR_HIGH);
    setSoftwareReset(ENABLE, CTRL_RST_CALI);

    /* rc32k calibration. */
    /* using the smallest coef. */
    setRc32kCailFilter(0x0);
    delayUs(100);   /* delay 1 cycle making this coef active. */
    index = 0;      /* index is from 0 ~ 63. the larger index means the higher frequency. */

    for(i = 5; i >= 0; i = i - 1)
    {
        /* try setting different bits to 1, from bit5 to bit0. */
        index |= 1 << i;
        /* exchange index to register. */
        if(index >= 0x20)
        {
            regVal = index - 0x20;
        }
        else
        {
            regVal = (~index) & 0xff ;
        }
        setRc32kCail(regVal);
        delayUs(100);

        /* read period of clock 32k. */
        regVal = getPmuStatus() & 0xffff;

        /* if period is less than our target, then clear this bit. */
        if(regVal < 0x5f5e)
        {
            index &= ~(1 << i);
        }
    }

    /* set as regular coef. */
    setRc32kCailFilter(0x5);
}

/**
 * @brief   select high_clk as 100MHz.
 * @param   None.
 * @return  None.
 */
void switch100MHclk(void)
{
    //select APB clock, based on hclk.
    //select CAN clock, based on hclk.
    #if FRAME_LOWPOWER
        setPclkDiv(HCLK_DIV2); 
        setCanClkDiv(CAN_CLK_HIGH_DIV_2);    
    #endif     
    setHighClkSrc(CLK_PLLI_CKDIG);
     
    for(uint8 i = 0; i < 10; i = i + 1);

    g_sysHighClk = SYS_HIGH_CLK_100M;
}

/**
 * @brief   select high_clk as 50MHz.
 * @param   None.
 * @return  None.
 */
void switch50MHclk(void)
{
    //select APB clock, based on hclk.
    //select CAN clock, based on hclk.
    #if FRAME_LOWPOWER
        setPclkDiv(HCLK_DIV1);
        setCanClkDiv(CAN_CLK_HIGH_DIV_1);
    #endif     
    setHighClkSrc(CLK_PLLI_50M);
    
    
    for(uint8 i = 0; i < 10; i = i + 1);

    g_sysHighClk = SYS_HIGH_CLK_50M;
}

/**
 * @brief   get Sys High Clk.
 * @param   None.
 * @return  sysHighClkTypedef.
 */
sysHighClkTypedef getSysHighClk(void)
{
    return g_sysHighClk;
}

/**
 * @brief   get pclk.
 * @param   None.
 * @return  sysHighClkTypedef.
 */
sysHighClkTypedef getPclk(void)
{
    volatile sysHighClkTypedef sysclk = SYS_HIGH_CLK_50M;
    
    /**Solution: switch the main clock, and the external set clock will also be switched, 
          leading to failure of communication 
          See jira for details:HFCN-24*/
        
    #if FRAME_LOWPOWER
        sysclk = SYS_HIGH_CLK_50M;
    #else
        sysclk = g_sysHighClk;
    #endif     

    
    return sysclk;
}

/**
 * @brief   initalization p2~p17.
 * @param   None.
 * @return  None.
 */
void ledInit(void)
{
    gpioInitStrTypedef  gpioInitStruct;
    gpioInitStruct.Mode  = GPIO_MODE_OUTPUT;
    gpioInitStruct.Pull  = GPIO_PULL_UP;
    gpioInitStruct.Pin   = GPIO_PIN_2  | GPIO_PIN_3  | GPIO_PIN_4  | \
                           GPIO_PIN_5  | GPIO_PIN_6  | GPIO_PIN_7  | \
                           GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10 | \
                           GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | \
                           GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_16 | \
                           GPIO_PIN_17;
    gpioInit(&gpioInitStruct);
}


/**
 * @brief   turn on/off all p2~p17.
 * @param   None.
 * @return  None.
 */
void ledAllon(void)
{
    GPIO->gpio_out_set = 0x03FFFC;
}
void ledAlloff(void)
{
    GPIO->gpio_out_clear = 0x03FFFC;
}


/**
 * @brief   led on/off.
 * @param   Pin_number: gpio number.
 * @return  None.
 */
void ledOn(uint32 actionType)
{
    GPIO->gpio_out_set = actionType & 0x03FFFC;
}
void ledOff(unsigned int Pin_number)
{
    GPIO->gpio_out_clear = Pin_number & 0x3fffff;
}
