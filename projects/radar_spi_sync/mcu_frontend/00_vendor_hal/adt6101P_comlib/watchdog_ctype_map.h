/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************
 * @file    watchdog_ctype_map.h
 * @original_author  WuHao(hwu@andartechs.com.cn)
 * @version 1.0.0
 * @date    2020-11-24
 * @brief   uart configuration registers address definition
 *
 *                 Copyright (c) 2020, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __WATCHDOG_CTYPE_MAP_H
#define __WATCHDOG_CTYPE_MAP_H
#include "adt6101P_type_define.h"
typedef struct
{
    __IO uint32 WDOGLOAD;       /* 0x000 */
    __IO uint32 WDOGVALUE;
    __IO uint32 WDOGCONTROL;
    __IO uint32 WDOGINTCLR;
    __IO uint32 WDOGRIS;
    __IO uint32 WDOGMIS;
         uint32 RESERVED0[0x2fa];
    __IO uint32 WDOGLOCK;       /* 0xc00 */
         uint32 RESERVED1[0xbf];
    __IO uint32 WDOGITCR;       /* 0xf00 */
    __IO uint32 WDOGITOP;
} WATCHDOG_TypeDef;


#endif
