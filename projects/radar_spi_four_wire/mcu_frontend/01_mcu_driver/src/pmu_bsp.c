/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    pmu_bsp.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file references the PMU HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "pmu_bsp.h"
#include "adt6101P.h"
#include "adt6101P_tx.h"
#include "adt6101P_rx.h"
#include "adt6101P_system.h"
#include "gpio_bsp.h"
#include "timer_bsp.h"


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   adt6101P power setting.
 * @param   onOroff: power on plli pll, plli vco, plln pll, plln vco, bb1 and bb2,
 *          bias1 and so on, see also @ref powerStateTypedef.
 * @return  None.
 */
void adt6101PPowerSw(const powerStateTypedef onOroff)
{
    g_sysHighClk = SYS_HIGH_CLK_50M;

    if(POWER_ON == onOroff)
    {
        baseband18LDO(ENABLE, BB_LDO_1P8, DISABLE, BB_LDO_1P8);
        delayUs(50);
        baseband18LDO(ENABLE, BB_LDO_1P8, ENABLE, BB_LDO_1P8);
        delayUs(50);
        pllLDOEnable(ENABLE, ENABLE);
        delayUs(50);
        pllVcoConfig(ENABLE, PLLN_VCO_LDO_1P8);
        delayUs(50);
        pllPllConfig(ENABLE, PLLN_PLL_LDO_2P5, ENABLE);
        delayUs(50);
        dacEnBias1Pd(ENABLE, POWER_ON);
        delayUs(50);
        
        /* must output 1, ant then set as output. so there is no glitch of P29's high level output. */
        gpioInitStrTypedef  gpio22InitStruct;
        gpio22InitStruct.Mode  = GPIO_MODE_OUTPUT;
        gpio22InitStruct.Pull  = GPIO_PULL_UP;
        gpio22InitStruct.Pin   = GPIO_PIN_22;
        gpioInit(&gpio22InitStruct);
        gpioWritePin(GPIO_PIN_22, GPIO_PIN_SET); 

    }
    else if(POWER_DOWN == onOroff)
    {
        pllLDOEnable(DISABLE, ENABLE);

        pllVcoConfig(DISABLE, PLLN_VCO_LDO_1P8);
        pllPllConfig(ENABLE, PLLN_PLL_LDO_2P5, ENABLE);
        baseband18LDO(DISABLE, BB_LDO_1P8, DISABLE, BB_LDO_1P8);
        dacEnBias1Pd(DISABLE, POWER_DOWN);
    }
}

/**
 * @brief   set adt6101P gpio is hz.
 * @param   None.
 * @return  None.
 */
void adt6101PGpioHz(void)
{
    gpioInitStrTypedef  gpioInitStruct;
    gpioInitStruct.Mode  = GPIO_MODE_HIGH_INPENDECE;
    gpioInitStruct.Pull  = GPIO_NO_PULL;
    gpioInitStruct.Pin   = GPIO_PIN_All;
    gpioInit(&gpioInitStruct);
}

