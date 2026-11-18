/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************
 * @file    rtc_ctype_map.h
 * @original_author  WuHao(hwu@andartechs.com.cn)
 * @version 1.0.0
 * @date    2020-11-24
 * @brief   rtc configuration registers address definition
 *
 *                 Copyright (c) 2020, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __RTC_CTYPE_MAP_H
#define __RTC_CTYPE_MAP_H
#include "adt6101P_type_define.h"
typedef struct
{
    __IO uint32 RTCDR;
    __IO uint32 RTCMR;
    __IO uint32 RTCLR;
    __IO uint32 RTCCR;
    __IO uint32 RTCIMSC;
    __IO uint32 RTCRIS;
    __IO uint32 RTCMIS;
    __IO uint32 RTCICR;
} RTC_TypeDef;


#endif
