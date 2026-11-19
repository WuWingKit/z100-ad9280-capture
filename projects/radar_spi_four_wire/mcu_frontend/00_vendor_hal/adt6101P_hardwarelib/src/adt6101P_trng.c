/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_trng.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file contains all the functions implemented by the TRNG
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_trng.h"
#include "trng_ctype_map.h"


/* Private variables ---------------------------------------------------------*/
static volatile uint32 s_trngFlag = 0;


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   trng init.
 * @param   trngBuff: .
 * @return  None.
 */
void trngInit(uint32 *trngBuff)
{
    if((TRNG->AUTOCORR_STATISTIC) > 0xffff)
    {
        TRNG->TRNG_SW_RESET = 1;
    }

    setTrngRngImr(TRNG_IMR_AUTOCORR_ERR_INT_MASK | \
                  TRNG_IMR_CRNGT_ERR_INT_MASK |      \
                  TRNG_IMR_VN_ERR_INT_MASK);
    
    /* 
        Sets the number of rng_clk cycles between two consecutive ring oscillator samples. 
        Under a 100M clock, the default setting is 0xFFFF, with a test time of 500ms to 500ms.
        When set to 0x1600, the test time is 40 to 50ms.
        The set value is decreasing, and the possible resulting numbers may not be randm.
    */
    TRNG->SAMPLE_CNT1 = 0x1600;
    
    /*RW
    [1:0] select the number of inverters(out of four possible selections) in the ring
    oscillator(the entropy source):
        00: select the shortest inverter chain length
        01: select the short inverter chain length
        10: select the long inverter chain length
        11: select the longest inverter chain length
    */
    setTrngConfig(TRNG_RND_SRC_SEL_LONG);

    /*RW
    The RND_SOURCE_ENABLE register controls whether the entropy source,ring oscillator
    1 : enable
    0 : disable
    */
    TRNG->RND_SOURCE_ENABLE = 1;


    while(s_trngFlag == 0);

    getTrngData(&trngBuff[0]);
    s_trngFlag = 0;
}

/**
 * @brief   get Trng Flag.
 * @param   None.
 * @return  uint8. Trng Flag value
 */
uint32 getTrngFlag(void)
{
    return s_trngFlag;
}

/**
 * @brief   set Trng Flag.
 * @param   uint8. Trng Flag value
 * @return  None.
 */
void settrngFlag(uint32 value)
{
    s_trngFlag = value;
}

/**
 * @brief   set Trng Rng Imr.
 * @param   tmpreg: value of the register.
 * @return  None.
 */
void setTrngRngImr(uint32 tmpreg)
{
    WRITE_REG(TRNG->RNG_IMR, tmpreg);
}

/**
 * @brief   get Trng Rng Imr.
 * @param   None.
 * @return  Trng Rng Imr.
 */
uint32 getTrngRngImr(void)
{
    return TRNG->RNG_IMR;
}

/**
 * @brief   get Trng Rng Isr.
 * @param   None.
 * @return  Trng Rng Isr.
 */
uint32 getTrngRngIsr(void)
{
    return TRNG->RNG_ISR;
}

/**
 * @brief   set Trng Rng Icr.
 * @param   tmpreg: value of the register.
 * @return  None.
 */
void setTrngRngIcr(uint32 tmpreg)
{
    WRITE_REG(TRNG->RNG_ICR, tmpreg);
}

/**
 * @brief   set Trng Config.
 * @param   trngCfgTypedef trngCfg.
 * @return  None.
 */
void setTrngConfig(trngCfgTypedef trngCfg)
{
    WRITE_REG(TRNG->TRNG_CONFIG, trngCfg);
}

/**
 * @brief   get Trng Config.
 * @param   None.
 * @return  Trng Rng Imr.
 */
uint32 getTrngConfig(void)
{
    return TRNG->TRNG_CONFIG;
}

/**
 * @brief   get Trng Value.
 * @param   None.
 * @return  Trng Value.
 */
uint32 getTrngValue(void)
{
    return TRNG->TRNG_VALUE;
}

/**
 * @brief   get Trng Data.
 * @param   trngBuff.
 * @return  None.
 */
void getTrngData(uint32 *trngBuff)
{
    trngBuff[0] = TRNG->EHRDATA0;
    trngBuff[1] = TRNG->EHRDATA1;
    trngBuff[2] = TRNG->EHRDATA2;
    trngBuff[3] = TRNG->EHRDATA3;
    trngBuff[4] = TRNG->EHRDATA4;
    trngBuff[5] = TRNG->EHRDATA5;
}
