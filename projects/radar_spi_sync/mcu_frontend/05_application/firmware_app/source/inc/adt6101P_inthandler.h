/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************
 * @file    adt6101P_inthandler.h
 * @original_author  WuHao(hwu@andartechs.com.cn)
 * @version 1.0.0
 * @date    2020-11-09
 * @brief   1. you need to turn on the total interrupt, for example, you want to use
            the uart0Hander ,the code you need to initialize is :
            soc_NVIC[soc_SETENA] = 0x01;
            2. you need to change the interrupt entry name in the startip_CMSDK_CM3.s
            to uart0Handler
 *
 *                 Copyright (c) 2020, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __INTHANDER__H
#define __INTHANDER__H
#include "adt6101P_type_define.h"


/*------------------------------ function ------------------------------------*/
void uart0InterruptHandler(const uint16 mode);

void uart1InterruptHandler(const uint16 mode);

void uart2InterruptHandler(const uint16 mode);


void timer0InterruptHandler(void);

void timer1InterruptHandler(void);


void dualtimer1InterruptHandler(void);

void dualtimer2InterruptHandler(void);


void i2c0InterruptHandler(const uint16 mode, const uint8 data);

void i2c1InterruptHandler(const uint16 mode, const uint8 data);


void spi0InterruptHandler(const uint8 mode);

void spi1InterruptHandler(const uint8 mode);

#endif
