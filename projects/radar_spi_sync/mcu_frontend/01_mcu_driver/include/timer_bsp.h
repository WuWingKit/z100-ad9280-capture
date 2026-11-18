/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    timer_bsp.h
 * @original_author  TaoCaoJia(cjtao@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file provides a TIMER universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef TIMER_BSP_H
#define TIMER_BSP_H
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_timer.h"
#include "sys_bsp.h"



/** @addtogroup BSP
 *  @{
 */

/** @addtogroup TIMER_BSP
 *  @{
 */


/* Exported types ------------------------------------------------------------*/
/** @addtogroup TIMER_Exported_Macros
  * @{
  */

/* Private define ------------------------------------------------------------*/
#define CLEAR_TIMER_VALUE  0

typedef struct
{
    TimerTypeDef *Timerx;             /* Timerx: Timer0 / Timer1*/

    timerCtrlModeTypeDef mode ;       /* mode: DEFUALT_MODE / INPUT_ENABLE_MODE / INPUT_CLOCK_MODE. */

    uint32 reload;                    /* reload: frequency of crystal oscillator. */

    TIMER_HANDLER    intHandler;      /* timer interrupt handler. */
} timerInitStrTypedef;

typedef struct
{
    dualTimerTypedef Timerx;             /* Timerx: DUALTIMER1 / DUALTIMER2        */

    uint32 reload;                       /* reload: frequency of crystal oscillator. */

    clkdivTypedef clkdiv;                /*clkdiv: CLKDIV1 / CLKDIV16 / CLKDIV256 / CLKDIV0. */

    DUALTIMER_HANDLER intHandler;        /* dualtimer interrupt handler. */

    dualTimerCntModeTypedef cntmode;     /* dualtimer counter mode. */

    dualTimerModeTypedef mode;           /* dualtimer Mode. */

} dualtimerInitStrTypedef;
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup TIMER_Exported_Functions
  * @{
  */
void timerInit(timerInitStrTypedef *Timerx);

void dualtimerInit(dualtimerInitStrTypedef *Timerx);

void timerOff(TimerTypeDef *Timerx);

void dualtimerOff(dualTimerTypedef Timerx);

uint32 getReload(float value);

void delayUs(uint32 delayus);

/**
  * @}
  */
/**
  * @}
  */
/**
  * @}
  */

#endif /* TIMER_BSP_H */
