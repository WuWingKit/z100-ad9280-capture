/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_timer.h
 * @original_author  TaoCaoJia(cjtao@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-19
 * @brief   Header file containing functions prototypes of Timer HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_TIMER_H
#define __ADT6101P_TIMER_H
/* Includes ------------------------------------------------------------------*/
#include "timer_ctype_map.h"

/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup Timer
 *  @{
 */

/** @addtogroup TIMER_Private_Macros
 *  @{
 */
/* Private   types ------------------------------------------------------------*/
typedef void (*TIMER_HANDLER)(void);
typedef void (*DUALTIMER_HANDLER)(void);

/**
  * @brief  Configuration CLKDIV enumeration
  */
typedef enum
{
    CLKDIV1   = 0x00, /*!<  0 stages of prescale, clock is divided by 1, default*/
    CLKDIV16  = 0x01, /*!<  4 stages of prescale, clock is divided by 16*/
    CLKDIV256 = 0x02, /*!<  8 stages of prescale, clock is divided by 256*/
    CLKDIV0   = 0x03  /*!<  Undefined, do not use*/
} clkdivTypedef;

/**
  * @brief  Configuration dualTimer enumeration
  */
typedef enum
{
    DUALTIMER1 = 0x0,
    DUALTIMER2 = 0x1
} dualTimerTypedef;

typedef enum
{
    WRAPPING = 0x0, /*!<  Wrapping mode, default*/
    ONE_SHOT = 0x1  /*!<  One-shot mode*/
} dualTimerCntModeTypedef;

typedef enum
{
    COUNTER16BIT = 0x0, /*!< 16-bit counter, default*/
    COUNTER32BIT = 0x1  /*!< 32-bit counter*/
} dualTimerSizeTypedef;

typedef enum
{
    FREE_RUNING = 0x0, /*!<  Timer is in free-running mode, default.*/
    PERIODIC    = 0x1  /*!<  Timer is in periodic mode*/
} dualTimerModeTypedef;

typedef enum
{
    TIMER_ENABLE     = 0x0, /*!< Enable*/
    INPUT_ENABLE     = 0x1, /*!< Select external input as enable.*/
    INPUT_CLOCK      = 0x2, /*!< Select external input as clock*/
    ENABLE_INTERRUPT = 0x3  /*!< Timer interrupt enable*/
} timerCtrlTypeDef;

typedef enum
{
    DEFUALT_MODE      = 0x0, /*!< Enable*/
    INPUT_ENABLE_MODE = 0x1, /*!< Select external input as enable.*/
    INPUT_CLOCK_MODE  = 0x2, /*!< Select external input as clock*/
} timerCtrlModeTypeDef;

typedef enum
{
    COUNTER_MODE         = 0x0, /*!< Selects one-shot or wrapping counter mode:*/
    TIMER_SIZE           = 0x1, /*!< Selects 16-bit or 32- bit counter operation*/
    TIMER_PRE            = 0x2, /*!< Prescale bits*/
    DUALTIMER_ENABLE_INT = 0x5, /*!< Interrupt Enable bit*/
    TIMER_MODE           = 0x6, /*!< Mode bit*/
    DUALTIMER_ENABLE     = 0x7  /*!< Enable bit*/
} dualTimerCtrlTypeDef;

/**
  * @}
  */
/* Private functions ---------------------------------------------------------*/
/** @addtogroup TIMER_Private_Functions
  * @{
  */
static void setTimerDefaultMode(TimerTypeDef *Timerx);

static void setTimerExInMode(TimerTypeDef *Timerx);

static void setTimerExinClkEn(TimerTypeDef *Timerx);

void setTimerIntEn(TimerTypeDef *Timerx, const stateTypedef enable);

void clearTimerCtrl(TimerTypeDef *Timerx);

uint8 getTimerCtrl(TimerTypeDef *Timerx, const timerCtrlTypeDef ctrl);

void setTimerCtrlMode(TimerTypeDef *Timerx, const timerCtrlModeTypeDef ctrlMode);

uint32 getTimerValue(TimerTypeDef *Timerx);

void setTimerValue(TimerTypeDef *Timerx, const uint32 value);

void setTimerReload(TimerTypeDef *Timerx, const uint32 reload);

uint32 getTimerReload(TimerTypeDef *Timerx);

void setTimerIntFlag(TimerTypeDef *Timerx, const stateTypedef enable);

uint8 getTimerIntFlag(TimerTypeDef *Timerx);

void setDualtimerReload(const dualTimerTypedef Timerx, const uint32 reload);

uint32 getDualtimerReload(const dualTimerTypedef Timerx);

uint32 getDualtimerValue(const dualTimerTypedef Timerx);

void setDualtimerCounterMode(const dualTimerTypedef Timerx, const dualTimerCntModeTypedef mode);

void setDualtimerSize(const dualTimerTypedef Timerx, const dualTimerSizeTypedef size);

void setDualtimerPre(const dualTimerTypedef Timerx, const clkdivTypedef clkdiv);

void setDualtimerInt(const dualTimerTypedef Timerx, const stateTypedef enable);

void setDualtimerMode(const dualTimerTypedef Timerx, const dualTimerModeTypedef mode);

void setDualtimerEnable(const dualTimerTypedef Timerx, const stateTypedef enable);

uint8 getDualtimerCtrl(const dualTimerTypedef Timerx, const dualTimerCtrlTypeDef ctrl);

void clearDualtimerIntReg(const dualTimerTypedef Timerx, const uint32 value);

uint8 getRawDualtimerInt(const dualTimerTypedef Timerx);

uint8 getDualtimerInt(const dualTimerTypedef Timerx);

uint32 getDualtimerBGload(const dualTimerTypedef Timerx);

void clearDualTimerCtrl(const dualTimerTypedef Timerx);

void timer0Handler(void);

void timer1Handler(void);

void setTimerIntHandler(const TimerTypeDef *Timerx, const TIMER_HANDLER handler);

void dualtimer0Handler(void);

void setDualTimerIntHandler(const dualTimerTypedef Timerx, const DUALTIMER_HANDLER handler);

/**
  * @}
  */
/**
  * @}
  */
/**
  * @}
  */

#endif
