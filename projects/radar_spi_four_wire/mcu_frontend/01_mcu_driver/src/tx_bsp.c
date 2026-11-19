/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    tx_bsp.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file references the TX library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P.h"
#include "tx_bsp.h"
#include "adt6101P_tx.h"
#include "adt6101P_system.h"
#include "timer_bsp.h"
#include "adc_bsp.h"

#include "adt6101P_adc.h"
#include "rfc_ctype_map.h"
#include <arm_math.h>
#include <stdbool.h>

/* Exported typedef ----------------------------------------------------------*/
typedef struct
{
    chirpChTypedef      chIndex;
    uint32              numStep;
    uint32              dtStep;
    uint32              dfStep;
    chirpChModeTypedef  chMode;
    chirpChClearTypedef chClear;
} chirpChannelStrTypedef;


/* Exported variables --------------------------------------------------------*/
volatile int g_gpadcOffset = 0;


/* Exported functions --------------------------------------------------------*/
void setChirpChannel(const chirpChannelStrTypedef *chirpChStr);
/**
 * @brief   adjust plln lock.
 * @param   fstart: Starting frequency point.
 * @param   fm: Frequency bandwidth.
 * @return  None.
 * @note
 */
void pllLock(float fstart, uint32 fm, uint32 chirpNumber)
{
    //refer to the detailed pllLOck flowchart from the following document
    //Chapter4.5.5 ADT3102_Technical_Reference_Manual
    uint32 pllnLockStatus;
    uint32 regPllnRegb;
    uint8 vcoCap      = 0;
    int32 fstartFlag  = 0;
    int32 fendtFlag   = 0;
    int16 vcoCont     = 0;
    int16 vcoContSave = 0;
    int8  vcoSwStart  = -1;
    int8  vcoSwEnd    = -1;
    bool  found       = false;
    bool  damage      = false;

    /* set vco_sw = 0 */
    RFC->rf_plln_regb_reg &= ~(0xf << 8);
    regPllnRegb = RFC->rf_plln_regb_reg;

    do{
        vcoSwStart = -1;
        vcoSwEnd   = -1;
        fstartFlag = 0;
        fendtFlag  = 0;

        for(vcoCap = 0; vcoCap < 16; vcoCap++)
        {
            RFC->rf_plln_regb_reg &= ~(0xf << 8);
            regPllnRegb = RFC->rf_plln_regb_reg;
            RFC->rf_plln_regb_reg = regPllnRegb | (vcoCap << 8);

            /* check fstart lock status */
            fmcwConfig(fstart, 0, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);
            fmcwEnmode(REGISTER, CONTINUE_MODE);
            delayUs(100);

            /* read PLL lock status. */
            pllnLockStatus =  RFC->rf_plln_rege_reg & (1 << 2);

            /* PLL is locked. */
            if(pllnLockStatus == 4)
            {
                fstartFlag = 1;
            }
            /* PLL is unlocked. */
            else
            {
                fstartFlag = 0;
            }

            /* check fend lock status */
            if((fm != 0) && (fstartFlag == 1))
            {
                fmcwConfig(fstart + ((float)fm / 1000.0f), 0, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);
                fmcwEnmode(REGISTER, CONTINUE_MODE);
                delayUs(100);

                /* read PLL lock status. */
                pllnLockStatus = RFC->rf_plln_rege_reg & (1 << 2);

                /* PLL is locked. */
                if(pllnLockStatus == 4)
                {
                    fendtFlag = 1;
                }
                /* PLL is unlocked. */
                else
                {
                    fendtFlag = 0;
                }
            }
            else
            {
                fendtFlag = 1;
            }

            /* at the end of each cycle, check prompt vcoSw is found */
            if((fstartFlag == 1) && (fendtFlag == 1))
            {
                found = true;
                if(vcoSwStart < 0)
                {
                    vcoSwStart = vcoCap;
                    vcoSwEnd   = vcoCap;
                }
                else
                {
                    vcoSwEnd   = vcoCap;
                    vcoCont++;
                }
            }
            if(vcoCont > 0 && found)
            {
                if(vcoContSave == vcoCont)
                {
                    break;
                }
                vcoContSave = vcoCont;
            }
        }

        if(found)
        {
            uint8 vcoSw;
            float vtuneValueM;
            vcoSw = ((int)(vcoSwStart + vcoSwEnd + 1) / 2);

            g_gpadcOffset = gpadcCalibration();

            RFC->rf_plln_regb_reg = regPllnRegb | (vcoSw << 8);
            fmcwConfig(fstart + ((float)fm / 1000.0f) / 2.0f, 0, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);
            fmcwEnmode(REGISTER, CONTINUE_MODE);
            delayUs(100);
            vtuneValueM = vtuneRead();
            CLEAR_VALUE(RFC->rf_gpadc_reg_reg);

            if(vtuneValueM < 120 && (vcoSw + 1) <= vcoSwEnd)
            {
                float vtuneValueH;

                RFC->rf_plln_regb_reg = regPllnRegb | (vcoSw + 1) << 8;
                fmcwConfig(fstart + ((float)fm / 1000.0f) / 2.0f, 0, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);
                fmcwEnmode(REGISTER, CONTINUE_MODE);
                delayUs(100);
                vtuneValueH = vtuneRead();
                CLEAR_VALUE(RFC->rf_gpadc_reg_reg);

                if(fabs(120 - vtuneValueH) >= fabs(120 - vtuneValueM))
                {
                    RFC->rf_plln_regb_reg = regPllnRegb | (vcoSw << 8);
                }
                else
                {
                    RFC->rf_plln_regb_reg = regPllnRegb | ((vcoSw + 1) << 8);
                }
            }
            else if(vtuneValueM > 120 && (vcoSw - 1) >= vcoSwStart)
            {
                float vtuneValueL;

                RFC->rf_plln_regb_reg = regPllnRegb | ((vcoSw - 1) << 8);
                fmcwConfig(fstart + ((float)fm / 1000.0f) / 2.0f, 0, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);
                fmcwEnmode(REGISTER, CONTINUE_MODE);
                delayUs(100);
                vtuneValueL = vtuneRead();
                CLEAR_VALUE(RFC->rf_gpadc_reg_reg);

                if(fabs(120 - vtuneValueL) >= fabs(120 - vtuneValueM))
                {
                    RFC->rf_plln_regb_reg = regPllnRegb | vcoSw << 8;
                }
                else
                {
                    RFC->rf_plln_regb_reg = regPllnRegb | (vcoSw - 1) << 8;
                }
            }
            else
            {
                RFC->rf_plln_regb_reg = regPllnRegb | (vcoSw << 8);
            }
        }
        else
        {
            damage = true;
            //printf("PLL calibration failed\n");
        }

    }while(!found && !damage);

}

/**
 * @brief   adjust plln lock with returned status and fstart.
 * @param   freqStart: pointer to fstart.
 * @param   fm: Frequency bandwidth..
 * @param   chirpNumber:  chirpNumber.
 * @return  the plln lock status
 * @note
 */
uint8 pllLockM(float *freqStart, uint32 fm, uint32 chirpNumber)
{
    //refer to the detailed pllLOck flowchart from the following document
    //Chapter4.5.5 ADT3102_Technical_Reference_Manual
    uint32 pllnLockStatus;
    uint32 regPllnRegb;
    uint8 vcoCap      = 0;
    int32 fstartFlag  = 0;
    int32 fendtFlag   = 0;
    int16 vcoCont     = 0;
    int16 vcoContSave = 0;
    int8  vcoSwStart  = -1;
    int8  vcoSwEnd    = -1;
    float vtuneValid  = 0;
    float vtuneMin    = 100;
    uint8 count       = 0;
    bool  found       = false;
    bool  damage      = false;
    float scan_step   = 0.0;
    float fstart      = *freqStart;
    uint8 pllStatus   = 0;


    RFC->rf_plln_regb_reg &= ~(0xf<<8);
    regPllnRegb = RFC->rf_plln_regb_reg;

    do{
        vcoSwStart = -1;
        vcoSwEnd   = -1;
        fstartFlag = 0;
        fendtFlag  = 0;
        vtuneMin   = 100;

        for(vcoCap = 0; vcoCap < 16; vcoCap++){
            RFC->rf_plln_regb_reg &= ~(0xf<<8);
            regPllnRegb = RFC->rf_plln_regb_reg;
            RFC->rf_plln_regb_reg = regPllnRegb | (vcoCap<<8);

            /* check fstart lock status */
            fmcwConfig(fstart + scan_step, 0, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);
            fmcwEnmode(REGISTER, CONTINUE_MODE);
            delayUs(100);

            pllnLockStatus = (RFC->rf_plln_rege_reg)&(1<<2);

            if(pllnLockStatus == 4){
                fstartFlag = 1;
            }
            else{
                fstartFlag = 0;
            }

            /* check fend lock status */
            if ((fm != 0) && (fstartFlag == 1)){
                fmcwConfig(fstart + scan_step + ((float)fm / 1000.0f), 0, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);
                fmcwEnmode(REGISTER, CONTINUE_MODE);
                delayUs(100);

                pllnLockStatus = (RFC->rf_plln_rege_reg)&(1<<2);

                if(pllnLockStatus == 4){
                    fendtFlag = 1;
                }
                else{
                    fendtFlag = 0;
                }
            }else{
                fendtFlag = 1;
            }

            /* at the end of each cycle, check prompt vcoSw is found */
            if ((fstartFlag == 1) && (fendtFlag == 1)){
                found = true;
                if(vcoSwStart < 0){
                    vcoSwStart = vcoCap;
                    vcoSwEnd   = vcoCap;
                }
                else
                {
                    vcoSwEnd   = vcoCap;
                    vcoCont++;
                }
            }
            if(vcoCont > 0 && found)
                if(vcoContSave == vcoCont) break;
                vcoContSave = vcoCont; 
        }

        if(found)
        {
            // if there are two valid vco_sw, choose the larger one.  XX for pcb803.
            RFC->rf_plln_regb_reg   = regPllnRegb | (((int)(vcoSwStart + vcoSwEnd + 1)/2)<<8);
            pllStatus = 1;
            //printf("PLL calibration okay. vcoSW=%d vcoSwStart = %d vcoSwEnd = %d \r\n", (int)(vcoSwStart + vcoSwEnd + 1)/2,vcoSwStart,vcoSwEnd);
        }
        else
        {
            fstartFlag = 0;
            count++;
            // check vtune for every vcoCap
            for(vcoCap = 0; vcoCap < 16; vcoCap++)
            {
                RFC->rf_plln_regb_reg &= ~(0xf<<8);
                regPllnRegb = RFC->rf_plln_regb_reg;
                RFC->rf_plln_regb_reg = regPllnRegb | (vcoCap<<8);

                fmcwConfig(fstart + scan_step, 0, CHIRP_T0, CHIRP_T1, CHIRP_T2, chirpNumber);
                fmcwEnmode(REGISTER, CONTINUE_MODE);
                delayUs(100);

                pllnLockStatus = (RFC->rf_plln_rege_reg)&(1<<2);

                if(pllnLockStatus == 4){
                    fstartFlag = 1;
                    vtuneValid = vtuneRead();
                    if(vtuneValid < vtuneMin){
                        vtuneMin = vtuneValid;
                    }
                }
            }

            //decrease fstart by scan_step if vtune is beyond expected
            //and try to lock again
            if (fstartFlag == 1)
            {
                if (vtuneMin >= 0.35)
                {
                    scan_step = scan_step - 0.1;
                    *freqStart = fstart + scan_step;
                    count = 0;
                }
            }

            //quit when lock failed
            if(count > 5)
            {
                damage = true;
                pllStatus = 0;
                //printf("PLL calibration failed\n");
            }
        }
    }while(!found && !damage);
    return pllStatus;
}


/**
 * @brief   The following is the parameter configuaration of vco output(sweep wave).
 *   |                /|\                    /|\        |            /|\
 *   |               / | \                  / | \       |           / | \
 *   |              /  |  \                /  |  \      |          /  |  \
 *   |             /   |   \              /   |   \    FM         /   |   \
 *   |            /    |    \            /    |    \    |        /    |    \
 *   |           /     |     \          /     |     \   |       /     |     \
 *   |          /      |      \        /      |      \  |      /      |      \
 *   |[fstart] /       |       \      /       |       \ |     /       |       \
_*___|________/________|________\____/________|________\|____/________|________\
 *   |        [  t0    ][  t1   ][t2][  t0    ][   t1   ][t2]
 *   |        [    Chirp1 cycle     ][    Chirp2 cycle  ]
 * @param   fstart:     GHz  start frequence.
 * @param   fm:         MHz  bandwidth.
 * @param   num_step:   us   chir p0 rise time.
 * @param   t0:         us   chirp1 rise time.
 * @param   t1:         us   chirp1 rise time.
 * @param   t2:         us   chirp1 rise time.
 * @param   chirpNumber: FMCW chirp number TX (Chirp1 cycle,Chirp2 cycle......).
 * @return  None.
 * @note    1,t0+t1+t2 = n*(120ns), "n" is an integer number.
 *          2,single frequency mode setting: FM=0;
 *          3,t0<1.04ms, t1<1.04ms, t0+t1+t2<7.86ms.
 *          The "1.04ms" means 2^17*8ns, and the "7.86ms" means 2^16*120ns from data_sample's view.
 *          4,if chirpNumber=0, then set as continue mode.
 *          5,Assumed the chirp's slope rate > 950Khz/1us.
 *          The "950Khz/1us" means 2Ghz/2^18 per 8ns, and equals to 2Ghz/2^18*125 per 1us.
 */
void fmcwConfig(float fstart, float fm, float t0, float t1, float t2, uint32 chirpNumber)
{
    /******************************************* FMCW channel setting *******/
    // chirp setting.
    float T0step = 20; // T0 and T1's delta_t step, for 100Mhz reference clock, it's (1+1)*10 = 20ns.
    float T2step = (t2 > 10000) ? 2500 : ((t2 > 2620) ? 80 : 20); // T2's delta_t step, currently it's 20ns. If T2 > 10ms, change step to 250*10ns., if 10ms >= T2 > 2.6ms, change step to 8*10ns.
   
    uint32 AdcPeriod ; // ADC sample period. unit:ns. 
    
    if(ADC_CLOCK_7P14M == ADC_SAMPLE_RATE)
    {
        AdcPeriod = getIntervalNum(ADC_SAMPLE_RATE, t0, t1, t2) * 140 ; //unit: ns.
    }
    
    // firstly, calculate t1 and t2, and then calculate and compensate t0 to align ADC sample period.  
    float ch1StepNum = round(t1 * 1000 / T0step);
    float ch2StepNum = round(t2 * 1000 / T2step);  
    
    uint32 ch0StepNum = (AdcPeriod - ch1StepNum * T0step - ch2StepNum * T2step) / T0step;
    
    float ch0Df = round(fm * 262144.0f / 1200.0f / ch0StepNum); // pow(2,18)/1200; ch0 delta frequency
    float ch1Df = round(ch0Df * ch0StepNum / ch1StepNum) ;
 
    //float FM_actual = ch0Df * ch0StepNum * 1200 / pow(2,18);   // unit: Mhz. 
    //float t0_actual = ch0StepNum * T0step / 1000;              // unit: us.
    //float t1_actual = ch1StepNum * T0step / 1000;              // unit: us.
    //float T2_actual = ch4StepNum * T2step / 1000;              // unit: us.

    // Chirp number
    RFC->fmcw_reg4_reg = chirpNumber & 0xff;
    RFC->fmcw_reg5_reg = (chirpNumber >> 8) & 0xff;
   
    // FMCW Chirp start frequency.
    uint32 freq_start;    
    // Fix the bug cause by the accuracy of 'float' type.
    // For example, when setting float fstart = 64.8, the actual fstart = 64.8000031, 
    // then it will case freq_start=0x580001, instead of our expected 0x580000. 
    uint32 fstart_100khz = round(fstart * 10000);
    freq_start = (uint32)(round(6553.6 * (fstart_100khz - 38.4 * 10000) / 300));//(uint32)(round(pow(2,18)*((Fstart_actual-38.4)/1.2)));
    RFC->fmcw_reg12_reg = freq_start & 0xff;//FMCW f_start
    RFC->fmcw_reg13_reg = (freq_start >> 8) & 0xff;//FMCW f_start
    RFC->fmcw_reg14_reg = (freq_start >> 16) & 0x7F;//FMCW f_start

    uint32 num_step;
    uint32 dt_step;
    uint32 clear;
    uint32 df_step;
    uint8 channelNumber;   
    uint8 chAdd ;
    uint8 chMode, chClear;
    /******************************************* FMCW channel setting *******/
    // only for "UP mode" channel and fstart is 1.2Ghz's interger multiple, clear is 1.  For all other cases, clear is 0. 
    // add  "round(fstart*1000),1200)" to fix the bug when fstart=64.8 or fstart=63.6. xx.      
    // only when fstart is 1.2Ghz's interger multiple, and fm=0, then disable dither, for best phase noise performance.   otherwise, there would be a noise at chirp frequency. 
    RFC->fmcw_reg0_reg |= fmcw_reg0_reg_dsm_dither_en_bit;
	  if (fmod(fstart_100khz, 12000) >= 1e-1) {
        clear = 0;
    } else {
        clear = 1;
        if(fm == 0) {
          RFC->fmcw_reg0_reg &= ~fmcw_reg0_reg_dsm_dither_en_bit;
        }
    }
    // configure channel_0 to channle_2.        
    for(int i = 0; i <= 2; i++)
    {
        chAdd = 0;
        dt_step = 1;        //assume all channel's dt=1, that means 1*T0step. 
        switch(i) 
        {
            case 0:
                chAdd = 1;
                channelNumber = 0;
                num_step = (uint32)(ch0StepNum - 1);  
                df_step = ch0Df; 
                chClear = clear; 
                chMode = 0x0 ; //0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.                    
                break;
            case 1:
                if(t1 != 0) {
                    chAdd = 1;
                    channelNumber++;
                    num_step = (uint32)(ch1StepNum-1);  
                    df_step = ch1Df; 
                    chClear = 0;      
                    chMode = 0x1 ; //0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.               
                }
                break;  
            case 2:  
                if(t2 != 0) {
                    chAdd = 1;
                    channelNumber++;
                    num_step = (uint32)(ch2StepNum-1);  
                    df_step = 0; 
                    chClear = clear;      
                    dt_step = T2step / 10 - 1; // for the case when T2 > 2.6ms                
                    chMode = 0x3 ; //0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.               
                }                            
                break;                              
            default : 
                break;
        }
        if(chAdd)
        {
            *((uint32*)(RFC_REG + channelNumber*5 + 0x210)) = num_step & 0xff;         // num_step's lsb.  fmcw_reg16_reg is 0x210
            *((uint32*)(RFC_REG + channelNumber*5 + 0x211)) = (num_step >> 8) & 0xff;    // num_step's mid. 
            *((uint32*)(RFC_REG + channelNumber*5 + 0x212)) = ((dt_step & 0xf) << 4) | (chMode << 2) | (chClear << 1) | ((num_step >> 16) & 0x1) ;  // num_step's msb.
            *((uint32*)(RFC_REG + channelNumber*5 + 0x213)) = ((df_step & 0xF) << 4) | ((dt_step & 0xF0) >> 4);
            *((uint32*)(RFC_REG + channelNumber*5 + 0x214)) = (df_step & 0xFF0) >> 4;                                   
        }                   
    }
    
    RFC->fmcw_reg6_reg = channelNumber;

//Commented by xx. ch0/ch1/ch2's clear have been set correctly by chClear as above code.     
    // if fm=0, then set as single frequency.
//    if(fm ==0)
//    {
//        RFC->fmcw_reg18_reg &= 0xfd ; //ch0's clear [1] = 0. 
//        RFC->fmcw_reg23_reg &= 0xfd ; //ch1's clear [1] = 0. 
//        RFC->fmcw_reg28_reg &= 0xfd ; //ch2's clear [1] = 0.        
//    }
    
    // if chirpNumber != 0, then set as single mode, otherwise, it's continue mode.
    if(chirpNumber !=0)
    {
        // ready for start FMCW chirp from DATA_SAMPLE_MODULE.
        fmcwEnmode(DATA_SAMPLE_MODULE, SINGLE_MODE);
    }
    else 
    {
        // ready for start FMCW chirp from REGISTER, and continue mode.
        fmcwEnmode(REGISTER, CONTINUE_MODE);
    }
 
#if 0
#if TD_MIMO_EN 
    // Please make sure g_tx0En=g_tx1En=1.  g_PaGain0 and g_PaGain0 not be zero. 
    // TD_MIMO can be confirmed by outputting two chirps through the selected debug channel.  
    // example: send range FFT data through a user-defined debug output when enabled.
// Please add enough delayUs between frames when debug output is enabled.
    uint32 chirp_channel_num;
    uint32 chirp_channel_bits;
    chirp_channel_num = RFC->fmcw_reg6_reg + 1;        // chirp_channel_num  
    chirp_channel_bits = (1 << chirp_channel_num) - 1; //for example,  2 channels: 0b11.  3 channels:0b111. 
    RFC->fmcw_mimo_tx0_on = chirp_channel_bits;
    RFC->fmcw_mimo_tx1_on = chirp_channel_bits << chirp_channel_num;
    RFC->fmcw_mimo_ctrl   = (chirp_channel_num << 1) | fmcw_mimo_ctrl_auto_tx_on_en_bit;   //chirp_channel_num*2.
#else
  #if BP_MIMO_EN
    // Please make sure g_tx0En=g_tx1En=1.  g_PaGain0 and g_PaGain0 not be zero. 
    // to be tested. 
    uint32 chirp_channel_num;
    uint32 chirp_channel_bits;
    chirp_channel_num = RFC->fmcw_reg6_reg + 1;        // chirp_channel_num  
    chirp_channel_bits = (1 << chirp_channel_num) - 1; //for example,  2 channels: 0b11.  3 channels:0b111. 
    RFC->fmcw_mimo_tx0_psr = 0x0;
    RFC->fmcw_mimo_tx1_psr = chirp_channel_bits << chirp_channel_num;
    RFC->fmcw_mimo_ctrl   = (chirp_channel_num << (fmcw_mimo_ctrl_tx_psr_period_shift+1)) | fmcw_mimo_ctrl_auto_tx_psr_en_bit;   //chirp_channel_num*2.
  #else    
    RFC->fmcw_mimo_ctrl   = 0;
  #endif   //#ifdef BP_MIMO
#endif  //#ifdef TD_MIMO 
#endif
}


/**
 * @brief   The following is the parameter configuaration of vco output(sweep wave).
 *                          fmSaw0                                              fmSaw0                                                                               _
 *   ?                        /|\                                                 /|\
 *   |                      /  | \            fmSaw1             |              /  | \            fmSaw1
 *   |                    /    |  \            /|\               |            /    |  \            /|\
 *   |                  /      |   \          / | \              |          /      |   \          / | \
 *   |                /        |    \        /  |  \             |        /        |    \        /  |  \
 *   |              /          |     \      /   |   \            |      /          |     \      /   |   \
 *   |            /            |      \    /    |    \           |    /            |      \    /    |    \
 *   |[fstart]  /              |       \  /     |     \          |  /              |       \  /     |     \
_*___|________/________________|________\/______|______\_________|/________________|________\/______|______\_________
 *   |        [    t0Saw0      ][t1Saw0 ][t0Saw1][t1Saw1][  t2  ]|
 *   |        [    Chirp1 cycle                                 ][    Chirp1 cycle                                 ]
 *   |                                                           |
 * @param   fstart:     GHz  start frequence.
 * @param   fm:         MHz  bandwidth.
 * @param   num_step:   us   chir p0 rise time.
 * @param   t0Saw0:     us   saw0 rise time.
 * @param   t1Saw0:     us   saw0 fall time.
 * @param   t0Saw0:     us   saw1 rise time.
 * @param   t1Saw1:     us   saw1 fall time.
 * @param   t2:         us   reset time.
 * @param   chirpNumber: FMCW chirp number TX (Chirp1 cycle,Chirp2 cycle......).
 * @return  None.
 * @note    1. (t0Saw0, t1Saw0, t0Saw1, t1Saw1, t2)*1000  MUST be step's integer multiple.
            2. chirp's slope more than 1mhz/1us.
            3. t0Saw0, t1Saw0, t0Saw0, t1Saw0, t2, each of them should < 1ms.  so we can assume all channel's dt=1, that means 8ns.
            why there is "1ms"?  Because our maximum step number is 2^17-1, and every step is assumed as 8ns, so (2^17-1)*8ns = 1ms approximately.
            4. Assume t0Saw0 > 0.
            5. if t0Saw1=0, then t1Saw1 will be ignored.
            6. t0Saw0+t1Saw0+t0Saw1+t1Saw1+t2 = n*(120ns), to align adc sampling.
            7. single frequency mode setting: FM=0;
 */
void fmcwConfig_dualSaw(float fstart, float fmSaw0, float t0Saw0, float t1Saw0, float fmSaw1, float t0Saw1,
                        float t1Saw1, float t2, uint32 chirpNumber)
{

    uint32 c7 = 140;  // ADC sample period. 
    float c8 = 20;     // fmcw reference clock. 
    //float c10 = 20;
    
    float chTemp = fmodf((t0Saw0+t1Saw0+t0Saw1+t1Saw1+t2)*1000,c7);
 
    float ch0StepNum = (t0Saw0*1000 - chTemp)/c8;  //ch0 step number, aligned to adc.
    float ch0Df = round(fmSaw0*131.072/ch0StepNum); // //pow(2,18)/2000;        //ch0 delta frequency  
    float ch1StepNum = round(t1Saw0*1000/c8); 
    float ch1Df = round(ch0Df * ch0StepNum / ch1StepNum) ; 
    float ch2StepNum = round(t0Saw1*1000/c8);   
    float ch2Df = round(fmSaw1*131.072/ch2StepNum); // //pow(2,18)/2000;            
    float ch3StepNum = round(t1Saw1*1000/c8);   
    float ch3Df = round(ch2Df * ch2StepNum / ch3StepNum) ;     
    float ch4StepNum = round(t2*1000/c8);       
        
    float Fstart_actual = fstart;
    //float FM_actual = ch0Df * ch0StepNum * 2000 / pow(2,18);   // unit: Mhz. 
    //float t0Saw0_actual = ch0StepNum * c8 / 1000;              // unit: us.
    //float t1Saw0_actual = ch1StepNum * c8 / 1000;              // unit: us.
    //float T2_actual     = ch4StepNum * c8 / 1000;              // unit: us.

    uint32 freq_start;
    uint32 num_step;
    uint32 dt_step;
    uint32 clear;
    uint32 df_step;
    uint8 channelNumber;   
    uint8 chAdd ;
    uint8 chMode, chClear;
    
    // Chirp number
    RFC->fmcw_reg4_reg = chirpNumber&0xff;
    RFC->fmcw_reg5_reg = (chirpNumber>>8)&0xff;
    
    // Chirp start frequency. 
    freq_start = (uint32)(round(131072*(Fstart_actual-64)));//(uint32)(round(pow(2,18)*((Fstart_actual-64)/2)));
    RFC->fmcw_reg12_reg = freq_start&0xff;//FMCW f_start
    RFC->fmcw_reg13_reg = (freq_start>>8)&0xff;//FMCW f_start
    RFC->fmcw_reg14_reg = (freq_start>>16)&0x7F;//FMCW f_start

    /******************************************* FMCW channel setting *******/
    // only for "UP mode" channel and fstart is 2Ghz's interger multiple, clear is 1.  For all other cases, clear is 0. 
    if (fmod(fstart,2)!=0) {
        clear = 0;
    } else {
        clear = 1;
    }
    // configure channel_0 to channle_5.        
    for(int i=0;i<=5;i++)
    {
        chAdd = 0;
        dt_step = 1;        //assume all channel's dt=1, that means 8ns. 
        switch(i) 
        {
            case 0:
                chAdd = 1;
                channelNumber = 0;
                num_step = (uint32)(ch0StepNum-1);  
                df_step = ch0Df; 
                chClear = clear; 
                chMode = 0x0 ; //0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.                    
                break;
            case 1:
                if(t1Saw0 != 0) {
                    chAdd = 1;
                    channelNumber++;
                    num_step = (uint32)(ch1StepNum-1);  
                    df_step = ch1Df; 
                    chClear = 0;      
                    chMode = 0x1 ; //0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.               
                }
                break;  
            case 2:  
                if(t0Saw1 != 0) {
                    chAdd = 1;
                    channelNumber++;
                    num_step = (uint32)(ch2StepNum-1);  
                    df_step = ch2Df; 
                    chClear = clear;      
                    chMode = 0x0 ; //0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.               
                }                            
                break;
            case 3:    //if t0Saw1=0, then t1Saw1 will be ignored. 
                if((t0Saw1 != 0) && (t1Saw1 != 0)) {
                    chAdd = 1;
                    channelNumber++;
                    num_step = (uint32)(ch3StepNum-1);  
                    df_step = ch3Df; 
                    chClear = 0;      
                    chMode = 0x1 ; //0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.               
                }                            
                break;  
            case 4:  
                if(t2 != 0) {
                    chAdd = 1;
                    channelNumber++;
                    num_step = (uint32)(ch4StepNum-1);  
                    df_step = 0; 
                    chClear = clear;      
                    chMode = 0x3 ; //0 : UP Mode. 1 : DOWN Mode. 2 : HOLD Mode. 3 : RESET Mode.               
                }                            
                break;                              
            default : 
                break;
        }
        if(chAdd)
        {
            *((uint32*)(RFC_REG + channelNumber*5 + 0x210)) = num_step&0xff;         // num_step's lsb.  fmcw_reg16_reg is 0x210
            *((uint32*)(RFC_REG + channelNumber*5 + 0x211)) = (num_step>>8)&0xff;    // num_step's mid. 
            *((uint32*)(RFC_REG + channelNumber*5 + 0x212)) = ((dt_step&0xf)<<4) | (chMode << 2) | (chClear<<1) | ((num_step>>16)&0x1) ;  // num_step's msb.
            *((uint32*)(RFC_REG + channelNumber*5 + 0x213)) = ((df_step &0xF)<<4)|((dt_step&0xF0)>>4);
            *((uint32*)(RFC_REG + channelNumber*5 + 0x214)) = (df_step &0xFF0)>>4;                                   
        }                   
    }
    
    RFC->fmcw_reg6_reg = channelNumber;
   
    if (fmSaw0 !=0 )
    {
        fmcwEnmode(DATA_SAMPLE_MODULE, SINGLE_MODE);
    }
#if 0
#if TD_MIMO_EN 
    // Please make sure g_tx0En=g_tx1En=1.  g_PaGain0 and g_PaGain0 not be zero. 
    // TD_MIMO can be confirmed by outputting two chirps through the selected debug channel.  
    // example: send range FFT data through a user-defined debug output when enabled.
// Please add enough delayUs between frames when debug output is enabled.
    uint32 chirp_channel_num;
    uint32 chirp_channel_bits;
    chirp_channel_num = RFC->fmcw_reg6_reg + 1;        // chirp_channel_num  
    chirp_channel_bits = (1 << chirp_channel_num) - 1; //for example,  2 channels: 0b11.  3 channels:0b111. 
    RFC->fmcw_mimo_tx0_on = chirp_channel_bits;
    RFC->fmcw_mimo_tx1_on = chirp_channel_bits << chirp_channel_num;
    RFC->fmcw_mimo_ctrl   = (chirp_channel_num << 1) | fmcw_mimo_ctrl_auto_tx_on_en_bit;   //chirp_channel_num*2.
#else
  #if BP_MIMO_EN
    // Please make sure g_tx0En=g_tx1En=1.  g_PaGain0 and g_PaGain0 not be zero. 
    // to be tested. 
    uint32 chirp_channel_num;
    uint32 chirp_channel_bits;
    chirp_channel_num = RFC->fmcw_reg6_reg + 1;        // chirp_channel_num  
    chirp_channel_bits = (1 << chirp_channel_num) - 1; //for example,  2 channels: 0b11.  3 channels:0b111. 
    RFC->fmcw_mimo_tx0_psr = 0x0;
    RFC->fmcw_mimo_tx1_psr = chirp_channel_bits << chirp_channel_num;
    RFC->fmcw_mimo_ctrl   = (chirp_channel_num << (fmcw_mimo_ctrl_tx_psr_period_shift+1)) | fmcw_mimo_ctrl_auto_tx_psr_en_bit;   //chirp_channel_num*2.
  #else    
    RFC->fmcw_mimo_ctrl   = 0;
  #endif
#endif
#endif
}


/**
 * @brief   Set the chirp channel for FMCW.
 * @param   chirpChStr: chirp channel structure, see also @ref chirpChannelStrTypedef.
 * @return  None.
 * @note
 */
void setChirpChannel(const chirpChannelStrTypedef *chirpChStr)
{
    setChirpChannelStepNum(chirpChStr->chIndex, chirpChStr->numStep);
    setChirpChannelDtStep(chirpChStr->chIndex, chirpChStr->dtStep);
    setChirpChannelDfStep(chirpChStr->chIndex, chirpChStr->dfStep);
    setChirpChannelMode(chirpChStr->chIndex, chirpChStr->chMode);
    chirpChannelClear(chirpChStr->chIndex, chirpChStr->chClear);
}


/**
 * @brief   vtune read.
 *          -> start frequence single lock
            -> end frequence single lock
 * @param   None.
 * @return  float.
 * @note
 */
float vtuneRead(void)
{
    volatile float vtuneValue;
    volatile uint8 gpadcValue;
    RFC->rf_plln_regc_reg |= 0x02300000;
    gpadcEnable(GPADC_IN18_2);
    //After testing, we need to read gpadc data many times.
    delayUs(60);
    
    gpadcValue = RFC->rf_gpadc_data_reg & 0xFF;
    delayUs(60);
    gpadcValue = RFC->rf_gpadc_data_reg & 0xFF;
    delayUs(60);
    gpadcValue = RFC->rf_gpadc_data_reg & 0xFF;
    
    //vtuneValue = (float)(((gpadcValue - g_gpadcOffset) * 1.0 / 256.0) * VDD_GPADC);
    vtuneValue = (float)(gpadcValue - g_gpadcOffset);
    RFC->rf_plln_regc_reg &= ~(0xfff00000);
    return vtuneValue;
}
