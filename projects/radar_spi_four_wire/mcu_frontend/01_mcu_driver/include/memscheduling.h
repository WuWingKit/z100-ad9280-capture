/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    memscheduling.h
 * @original_author  HuangZhiheng(zhhuang@andartechs.com.cn)
 * @version 1.0.0
 * @date    2024-01-22
 * @brief   This file provides a DSP universal function interface, making it convenient
 *          for users to directly call it.
 *          memory                   address                size
 *          CODE RAM          0x0000_8000-0x0000_ffff       32KB     coding memory
 *          DATA RAM          0x2000_0000-0x2000_7fff       32KB     stack and heap
 *          DATA RAM          0x2000_8000-0x2001_7fff       64KB     adc0 ,fft data
 *          DATA RAM          0x2001_8000-0x2002_7fff       64KB     adc1 ,fft data
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __MEMSCHDULING_H
#define __MEMSCHDULING_H


/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "adt6101P.h"


/** @addtogroup SERVICE
 *  @{
 */

/** @addtogroup MEMORY_SCHEDULE
 *  @{
 */
/* Exported types ------------------------------------------------------------*/
/** @addtogroup DOP_SWAP_Exported_Macros
  * @{
  */
/**
 * @brief  Doppler swap structure
 */
typedef struct
{
    uint16 rangeMax;
    uint16 velocityMax;
} dopSwapStrTypedef;

/**
 * @brief 命名规范化别名：多普勒维度交换参数
 */
typedef dopSwapStrTypedef memDopplerSwapConfig_t;

/**
 * @brief 命名规范化别名：DMA 搬运参数
 */
typedef struct
{
    uint32 srcAddr;
    uint32 dstAddr;
    uint16 srcStepBytes;
    uint16 dstStepBytes;
    uint16 transferCount;
} memTransferConfig_t;


/* Exported functions --------------------------------------------------------*/
/** @addtogroup MEMORY_SCHEDULE_Exported_Functions
  * @{
  */
void memScheduling(
    uint32 srcAddre,
    uint32 dstAddre,
    uint16 srcStep,
    uint16 dstStep,
    uint16 transLen);

void dopplerSwap(int32 dopFftLen, int32 rngMax, uint32 absMergeArray[][CHIRP_NUM]);

void dopplerSwapMimo(dopSwapStrTypedef *dopSwapStr, uint32 absMergeArray[][CHIRP_NUM2]);

/**
 * @brief 规范命名接口：执行一次 DMA 内存搬运
 *
 * 使用示例：
 * memTransferConfig_t cfg = {(uint32)&src[0], (uint32)&dst[0], 4, 4, len};
 * memScheduleTransfer(&cfg);
 */
static inline void memScheduleTransfer(const memTransferConfig_t *cfg)
{
    memScheduling(cfg->srcAddr,
                  cfg->dstAddr,
                  cfg->srcStepBytes,
                  cfg->dstStepBytes,
                  cfg->transferCount);
}

/**
 * @brief 规范命名接口：执行多普勒交换（SISO）
 */
static inline void memDopplerSwap(int32 dopFftLen, int32 rngMax, uint32 absMergeArray[][CHIRP_NUM])
{
    dopplerSwap(dopFftLen, rngMax, absMergeArray);
}

/**
 * @brief 规范命名接口：执行多普勒交换（MIMO）
 */
static inline void memDopplerSwapMimo(memDopplerSwapConfig_t *cfg, uint32 absMergeArray[][CHIRP_NUM2])
{
    dopplerSwapMimo(cfg, absMergeArray);
}

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


