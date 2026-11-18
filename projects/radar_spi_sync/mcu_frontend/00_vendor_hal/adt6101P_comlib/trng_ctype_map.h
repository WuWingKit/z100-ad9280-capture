/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************
 * @file    trng_ctype_map.h
 * @original_author  WuHao(hwu@andartechs.com.cn)
 * @version 1.0.0
 * @date    2020-11-24
 * @brief   trng configuration registers address definition
 *
 *                 Copyright (c) 2020, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __TRNG_CTYPE_MAP_H
#define __TRNG_CTYPE_MAP_H
#include "adt6101P_type_define.h"

typedef struct
{
         uint32 reserve0[0x40];
    __IO uint32 RNG_IMR;
    __IO uint32 RNG_ISR;
    __IO uint32 RNG_ICR;
    __IO uint32 TRNG_CONFIG;
    __IO uint32 TRNG_VALUE;
    __IO uint32 EHRDATA0;
    __IO uint32 EHRDATA1;
    __IO uint32 EHRDATA2;
    __IO uint32 EHRDATA3;
    __IO uint32 EHRDATA4;
    __IO uint32 EHRDATA5;
    __IO uint32 RND_SOURCE_ENABLE;
    __IO uint32 SAMPLE_CNT1;
    __IO uint32 AUTOCORR_STATISTIC;
    __IO uint32 TRNG_DEBUG_CONTROL;
         uint32 reserve1[0x01];
    __IO uint32 TRNG_SW_RESET;
         uint32 reserve2[0x1d];
    __IO uint32 TRNG_BUSY;
    __IO uint32 RST_BIST_COUNTER;
         uint32 reserve3[0x08];
    __IO uint32 RNG_BIST_CNTR0;
    __IO uint32 RNG_BIST_CNTR1;
    __IO uint32 RNG_BIST_CNTR2;

} TrngTypeDef;

#endif
