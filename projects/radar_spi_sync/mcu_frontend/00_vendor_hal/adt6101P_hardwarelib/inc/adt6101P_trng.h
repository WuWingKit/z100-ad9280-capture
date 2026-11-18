/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_trng.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   Header file containing functions prototypes of TRNG HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_TRNG__H
#define __ADT6101P_TRNG__H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"


/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup TRNG
 *  @{
 */


/** @addtogroup TRNG_Private_Macros
  * @{
  */
/* Private define ------------------------------------------------------------*/
/**
  * @brief  TRNG RNG_IMR select
  */
#define TRNG_IMR_EHR_VALID_INT_MASK       ((uint8)0x01)
#define TRNG_IMR_AUTOCORR_ERR_INT_MASK    ((uint8)0x02)
#define TRNG_IMR_CRNGT_ERR_INT_MASK       ((uint8)0x04)
#define TRNG_IMR_VN_ERR_INT_MASK          ((uint8)0x08)

/**
  * @brief  TRNG RNG_ISR select
  */
#define TRNG_ISR_EHR_VALID                ((uint8)0x01)
#define TRNG_ISR_AUTOCORR_ERR             ((uint8)0x02)
#define TRNG_ISR_CRNGT_ERR                ((uint8)0x04)
#define TRNG_ISR_VN_ERR                   ((uint8)0x08)

/**
  * @brief  TRNG RNG_ICR select
  */
#define TRNG_ICR_EHR_VALID                ((uint8)0x01)
#define TRNG_ICR_AUTOCORR_ERR             ((uint8)0x02)
#define TRNG_ICR_CRNGT_ERR                ((uint8)0x04)
#define TRNG_ICR_VN_ERR                   ((uint8)0x08)

/**
  * @brief  TRNG RNG_VALID select
  */
#define TRNG_EHR_VALID                    ((uint8)0x01)

/**
  * @brief  Configuration Trng enumeration
  */
typedef enum
{
    TRNG_RND_SRC_SEL_SHORTEST     = 0x0,
    TRNG_RND_SRC_SEL_SHORT        = 0x1,
    TRNG_RND_SRC_SEL_LONG         = 0x2,
    TRNG_RND_SRC_SEL_LONGEST      = 0x3
} trngCfgTypedef;
/**
  * @}
  */


/* Private functions ---------------------------------------------------------*/
/** @addtogroup TRNG_Private_Functions
  * @{
  */
void trngInit(uint32 *trngBuff);

uint32 getTrngFlag(void);
void settrngFlag(uint32 value);

void setTrngRngImr(uint32 tmpreg);
uint32 getTrngRngImr(void);

uint32 getTrngRngIsr(void);
void setTrngRngIcr(uint32 tmpreg);

void setTrngConfig(trngCfgTypedef trngCfg);
uint32 getTrngConfig(void);

uint32 getTrngValue(void);
void getTrngData(uint32 *trngBuff);

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
