/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_system.c
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file contains all the functions implemented by the SYSTEM
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_system.h"
#include "CMSDK_CM3.h"
#include "adt6101P_type_define.h"
#include "uart_ctype_map.h"
#include "gpio_ctype_map.h"
#include "pmu_ctype_map.h"
#include "rfc_ctype_map.h"
#include <stdio.h>

/* Private functions ---------------------------------------------------------*/
#ifndef  __NO_SYSTEM_INIT
void SystemInit()
{
    //Init.
    for(uint32 i = 0; i < 10000; i++) {}
}
#endif


/**
 * @brief   switching interrupt .
 *      NMI: Watchdog
 *      0  : Uart0 tx/rx                9  : spi1
 *      1  : Uart1 tx/rx                10 : RTC
 *      2  : Uart3 tx/rx                11 : IIC0
 *      3  : Timer0                     12 : IIC1
 *      4  : Timer1                     13 : FFT
 *      5  : Dualtimer2                 14 : reserve
 *      6  : Uart0/1/2 overflow         15 : gpio
 *      7  : pmu interrupt              16 : dma_int0
 *      8  : spi0                       17 : dma_int1       18 : TRNG
 * @param   interrutNum: interrupt number, see also @ref interruptNumTypedef.
 * @param   enableOrDisenable: on or off total interrupt 1:on 0:off, see also @ref interruptStateTypedef.
 * @return  None.
 */
void setIntHandlerEn(interruptNumTypedef interruptNum, interruptStateTypedef enableOrDisenable)
{
    if(DISABLEINT == enableOrDisenable)
    {
        WRITE_REG(soc_NVIC[soc_CLRENA], (1 << (uint8)interruptNum));
    }
    else if(ENABLEINT == enableOrDisenable)
    {
        WRITE_REG(soc_NVIC[soc_SETENA], (1 << (uint8)interruptNum));
    }
}


/**
 * @brief   Get Priority Grouping
 *          Reads the priority grouping field from the NVIC Interrupt Controller.
 * @param   None.
 * @return  Priority grouping field (soc->AIRCR [10:8] PRIGROUP field).
 */
uint32 getNvicPriorityGrouping(void)
{
    return (((*(uint32 *)soc_AIRCR) & SCB_AIRCR_PRIGROUP_Msk) >> SCB_AIRCR_PRIGROUP_Pos);
}

/**
 * @brief   Encode Priority
 *          Encodes the priority for an interrupt with the given priority group,
 *          preemptive priority value, and subpriority value.
 *          In case of a conflict between priority grouping and available
 *          priority bits (__NVIC_PRIO_BITS), the smallest possible priority group is set.
 *
 * @param   PriorityGroup    Used priority group.
 * @param   PreemptPriority  Preemptive priority value (starting from 0).
 * @param   SubPriority      Subpriority value (starting from 0).
 * @return          Encoded priority. Value can be used in the function \ref setPriority().
 */
uint32 encodeNvicPriority(uint32 PriorityGroup, uint32 PreemptPriority, uint32 SubPriority)
{
    uint32 PriorityGroupTmp = (PriorityGroup & (uint32)0x07UL);
    uint32 PreemptPriorityBits;
    uint32 SubPriorityBits;

    PreemptPriorityBits = ((7UL - PriorityGroupTmp) > (uint32)(__NVIC_PRIO_BITS)) ? (uint32)(__NVIC_PRIO_BITS) : (uint32)(
                              7UL - PriorityGroupTmp);
    SubPriorityBits     = ((PriorityGroupTmp + (uint32)(__NVIC_PRIO_BITS)) < (uint32)7UL) ? (uint32)0UL : (uint32)((
                              PriorityGroupTmp - 7UL) + (uint32)(__NVIC_PRIO_BITS));

    return (
               ((PreemptPriority & (uint32)((1UL << (PreemptPriorityBits)) - 1UL)) << SubPriorityBits) |
               ((SubPriority     & (uint32)((1UL << (SubPriorityBits)) - 1UL)))
           );
}

/**
 * @brief   Set Interrupt Priority
 *          Sets the priority of a device specific interrupt or a processor exception.
 *          The interrupt number can be positive to specify a device specific interrupt,
 *          or negative to specify a processor exception.
 *
 * @param   interruptNum    Interrupt number.
 * @param   priority        Priority to set.
 * @return  None.
 * @note        The priority cannot be set for every processor exception.
 */
void setPriority(int32 interruptNum, uint32 priority)
{
    if(interruptNum >= 0)
    {
        NVIC->IP[interruptNum] = (uint8)((priority << (8U - __NVIC_PRIO_BITS)) & (uint32)0xFFUL);
    }
}
/**
 * @brief   Set Nvic Interrupt Priority
 * @param   interruptNum     Interrupt number.
 * @param   PreemptPriority  Group Priority
 * @param   SubPriority      SubPriority
 * @return  None.
 */
void setNvicPriority(int32 interruptNum, uint32 PreemptPriority, uint32 SubPriority)
{
    uint32 prioritygroup = 0x00U;

    prioritygroup = getNvicPriorityGrouping();

    setPriority(interruptNum, encodeNvicPriority(prioritygroup, PreemptPriority, SubPriority));
}

/**
 * @brief   Set Priority Grouping
 *          Sets the priority grouping field using the required unlock sequence.
 *          The parameter PriorityGroup is assigned to the field SCB->AIRCR [10:8] PRIGROUP field.
 *          Only values from 0..7 are used.
 *          In case of a conflict between priority grouping and available
 *          priority bits (__NVIC_PRIO_BITS), the smallest possible priority group is set.
 *
 * @param   PriorityGroup  Priority grouping field.
 * @return  None.
 */
void setNvicPriorityGroup(uint32 PriorityGroup)
{
    uint32 reg_value;
    uint32 PriorityGroupTmp = (PriorityGroup & (uint32)0x07UL);

    reg_value = SCB->AIRCR;
    reg_value &= ~((uint32_t)(SCB_AIRCR_VECTKEY_Msk | SCB_AIRCR_PRIGROUP_Msk));

    reg_value = (reg_value                                   |
                 ((uint32_t)0x5FAUL << SCB_AIRCR_VECTKEY_Pos) |
                 (PriorityGroupTmp << SCB_AIRCR_PRIGROUP_Pos));                /* Insert write key and priority group */
    SCB->AIRCR =  reg_value;
}


/**
 * @brief   Used to indicate that the vector table is in the
 *          code area or in the RAM area.
 * @param   NVIC_VectTab: The defalut of ADT6101P is 0x00000000.
 * @param   Offset: vector table offset ,The defalut of ADT6101P is 0x8000.
 * @return  None.
 */
void MY_NVIC_SetVectorTable(uint32 NVIC_VectTab, uint32 Offset)
{
    //set NVIC's vector table offset register
    SCB->VTOR = NVIC_VectTab | (Offset & (uint32)0x1FFFFF80);
    __enable_irq();

}


/**
 * @brief   system_reset.
 * @param   None.
 * @return  None.
 */
void system_reset(void)
{
    int32 regVal = 0;
    // read 0xE000ED0C, and then change key to 0x05fa, and write bit [2] = 1.
    regVal = soc_AIRCR[0x0];
    regVal = (0x0000ffff & regVal) | 0x05fa0004;
    soc_AIRCR[0x0] = regVal;
}



/**
 * @brief   Set the high clock soruce.
 * @param   sel: select clk_high source, see also @ref clkHighSrcTypedef.
 * @return  None.
 */
void setHighClkSrc(const clkHighSrcTypedef sel)
{
    SET_VALUE(PMU->pmu_clk_high_reg, \
              pmu_clk_high_reg_ctrl_high_clk_sel_shift, \
              pmu_clk_high_reg_ctrl_high_clk_sel_mask, \
              (uint8)sel);
}

/**
 * @brief   Get the high clock soruce.
 * @return  clk_high source.
 */
uint8 getHighClkSrc(void)
{
    return GET_BITS_VALUE(PMU->pmu_clk_high_reg, \
                          pmu_clk_high_reg_ctrl_high_clk_sel_shift, \
                          pmu_clk_high_reg_ctrl_high_clk_sel_mask);
}

/**
 * @brief   Set the high clock soruce.
 * @param   sel: select clk_high source, see also @ref clkHighSrcTypedef.
 * @return  None.
 */
void setPclkDiv(const pclkDivSrcTypedef sel)
{
    SET_VALUE(PMU->pmu_clk_high_reg, \
              pmu_clk_high_reg_ctrl_apb_clk_sel_shift, \
              pmu_clk_high_reg_ctrl_apb_clk_sel_mask, \
              (uint8)sel);
}


/**
 * @brief   get range res.
 * @param   fm: .
 * @param   t0: .
 * @param   downSample: .
 * @param   fftPoint: .
 * @return  float.
 */
float getRangeRes(uint32 fm, uint32 t0, uint16 downSample, uint16 fftPoint)
{
    float rangeResolution;
    rangeResolution = 300000000.0 * (t0 / 1000000.0) * (125.0 / 15.0 / (downSample + 1) * 1000000.0) / 2.0 /
                      (fm * 1000000.0) / fftPoint;
    return rangeResolution;
}
