/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************
 * @file    spi_ctype_map.h
 * @original_author  WuHao(hwu@andartechs.com.cn)
 * @version 1.0.0
 * @date    2020-11-24
 * @brief   spi configuration registers address definition
 *
 *                 Copyright (c) 2020, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __SPI_CTYPE_MAP_H
#define __SPI_CTYPE_MAP_H
#include "adt6101P_type_define.h"

#define spicr0_scr_shift       8
#define spicr0_scr_mask        0xFF00
#define spicr0_sph_shift       7
#define spicr0_sph_bit         (1 << 7)
#define spicr0_sph_mask        0x01
#define spicr0_spo_shift       6
#define spicr0_spo_bit         (1 << 6)
#define spicr0_spo_mask        0x01
#define spicr0_frf_shift       4
#define spicr0_frf_mask        0x30
#define spicr0_dss_shift       0
#define spicr0_dss_mask        ((1<<4)-1)

#define spicr1_sod_shift       3
#define spicr1_sod_bit         (1 << 3)
#define spicr1_ms_shift        2
#define spicr1_ms_bit          (1 << 2)
#define spicr1_sse_shift       1
#define spicr1_sse_bit         (1 << 1)
#define spicr1_lbm_shift       0
#define spicr1_lbm_bit         (1 << 0)

#define spisr_bsy_shift        4
#define spisr_bsy_bit          (1 << 4)
#define spisr_rff_shift        3
#define spisr_rff_bit          (1 << 3)
#define spisr_rne_shift        2
#define spisr_rne_bit          (1 << 2)
#define spisr_tnf_shift        1
#define spisr_tnf_bit          (1 << 1)
#define spisr_tfe_shift        0
#define spisr_tfe_bit          (1 << 0)

#define spiimsc_txim_shift     3
#define spiimsc_txim_bit       (1 << 3)
#define spiimsc_rxim_shift     2
#define spiimsc_rxim_bit       (1 << 2)
#define spiimsc_rtim_shift     1
#define spiimsc_rtim_bit       (1 << 1)
#define spiimsc_rorim_shift    0
#define spiimsc_rorim_bit      (1 << 0)

#define spiris_txris_shift     3
#define spiris_txris_bit       (1 << 3)
#define spiris_rxris_shift     2
#define spiris_rxris_bit       (1 << 2)
#define spiris_rtris_shift     1
#define spiris_rtris_bit       (1 << 1)
#define spiris_rorris_shift    0
#define spiris_rorris_bit      (1 << 0)

#define spimis_txmis_shift     3
#define spimis_txmis_bit       (1 << 3)
#define spimis_rxmis_shift     2
#define spimis_rxmis_bit       (1 << 2)
#define spimis_rtmis_shift     1
#define spimis_rtmis_bit       (1 << 1)
#define spimis_rormis_shift    0
#define spimis_rormis_bit      (1 << 0)

#define spiicr_rtic_shift      1
#define spiicr_rtic_bit        (1 << 1)
#define spiicr_roric_shift     0
#define spiicr_roric_bit       (1 << 0)

#define spidmacr_txdmae_shift  1
#define spidmacr_txdmae_bit    (1 << 1)
#define spidmacr_rxdmae_shift  0
#define spidmacr_rxdmae_bit    (1 << 0)

#define spicpsr_cpsdvsr_shift  0
#define spicpsr_cpsdvsr_mask   0xFF



typedef struct
{
    __IO uint32	SPICR0;
    __IO uint32	SPICR1;
    __IO uint32	SPIDR;
    __IO uint32 SPISR;
    __IO uint32	SPICPSR;
    __IO uint32	SPIIMSC;
    __IO uint32	SPIRIS;
    __IO uint32	SPIMIS;
    __IO uint32	SPIICR;
    __IO uint32	SPIDMACR;
} SPI_TypeDef;

#endif
