/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************
 * @file    i2c_ctype_map.h
 * @original_author  WuHao(hwu@andartechs.com.cn)
 * @version 1.0.0
 * @date    2020-11-24
 * @brief   i2c configuration registers address definition
 *
 *                 Copyright (c) 2020, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************
 */
#ifndef __I2C_CTYPE_MAP_H
#define __I2C_CTYPE_MAP_H
#include "adt6101P_type_define.h"
//#define i2cir_tfifo_wr_data 0x000
#define i2cir_tfifo_wr_data_shift 0
#define i2cir_tfifo_wr_data_mask ((1<<8)-1)

//#define i2cir_rfifo_rd_data 0x001
#define i2cir_rfifo_rd_data_shift 0
#define i2cir_rfifo_rd_data_mask ((1<<8)-1)

//#define i2cir_int_status 0x002
#define i2cir_status_receive_FIFO_time_out_bit (1<<0)
#define i2cir_status_transmit_FIFO_under_thld_bit (1<<1)
#define i2cir_status_transmit_FIFO_empty_bit (1<<2)
#define i2cir_status_transmit_FIFO_over_read_bit (1<<3)
#define i2cir_status_transmit_FIFO_over_write_bit (1<<4)
#define i2cir_status_receive_FIFO_over_thld_bit (1<<5)
#define i2cir_status_receive_FIFO_full_bit (1<<6)
#define i2cir_status_receive_FIFO_not_empty_bit (1<<7)
#define i2cir_status_receive_FIFO_over_read_bit (1<<8)
#define i2cir_status_master_transfer_completed_bit (1<<9)
#define i2cir_status_slave_bus_error_bit (1<<10)
#define i2cir_status_slave_address_match_bit (1<<11)
//#define i2cir_int_mask 0x003

//#define i2cir_int_clr 0x004

//#define i2c_i2cm_cr 0x005

//#define i2c_cfg 0x006
#define i2c_cfg_i2cs_nack_bit (1<<21)
#define i2c_cfg_i2cs_nack_shift 21
#define i2c_cfg_i2cs_10bitaddr_bit (1<<20)
#define i2c_cfg_i2cs_10bitaddr_shift 20
#define i2c_cfg_en_bit (1<<19)
#define i2c_cfg_master_bit (1<<18)
#define i2c_cfg_master_shift 18
#define i2c_cfg_reserved_bit_shift 16
#define i2c_cfg_reserved_bit_mask ((1<<2)-1)
#define i2c_cfg_to_thld_shift 8
#define i2c_cfg_to_thld_mask ((1<<8)-1)
#define i2c_cfg_rfifo_thld_shift 4
#define i2c_cfg_rfifo_thld_mask ((1<<4)-1)
#define i2c_cfg_tfifo_thld_shift 0
#define i2c_cfg_tfifo_thld_mask ((1<<4)-1)

//#define i2c_i2cm_cfg 0x007
#define i2c_i2cm_cfg_i2cm_prer_shift 8
#define i2c_i2cm_cfg_i2cm_prer_mask ((1<<16)-1)
#define i2c_i2cm_cfg_i2cm_ctr_shift 0
#define i2c_i2cm_cfg_i2cm_ctr_mask ((1<<8)-1)

//#define i2c_i2cm_tar 0x008

//#define i2c_i2cs_sar 0x009

//#define i2c_byte_fifo_cnt 0x00a
#define i2c_byte_fifo_cnt_i2cm_byte_rest_shift 8
#define i2c_byte_fifo_cnt_i2cm_byte_rest_mask ((1<<8)-1)
#define i2c_byte_fifo_cnt_rfifo_cnt_shift 4
#define i2c_byte_fifo_cnt_rfifo_cnt_mask ((1<<4)-1)
#define i2c_byte_fifo_cnt_tfifo_cnt_shift 0
#define i2c_byte_fifo_cnt_tfifo_cnt_mask ((1<<4)-1)

//#define i2c_i2cm_transfer_len 0x00b

//#define i2c_tfifo_idle_data 0x00c

//#define i2c_status 0x00d
#define i2c_status_i2cs_rd_op_bit (1<<7)
#define i2c_status_i2cs_rd_op_shift 7
#define i2c_status_i2cs_wr_op_bit (1<<6)
#define i2c_status_i2cs_wr_op_shift 6
#define i2c_status_i2cs_busy_bit (1<<5)
#define i2c_status_i2cs_busy_shift 5
#define i2c_status_i2cm_sr_shift 0
#define i2c_status_i2cm_sr_mask ((1<<5)-1)

//#define i2c_fifo_clr 0x00e

// I2C
typedef struct
{
    __IO uint32 i2cir_tfifo_wr_data ;
    __IO uint32 i2cir_rfifo_rd_data ;
    __IO uint32 i2cir_int_status ;
    __IO uint32 i2cir_int_mask ;
    __IO uint32 i2cir_int_clr ;
    __IO uint32 i2c_i2cm_cr ;
    __IO uint32 i2c_cfg ;
    __IO uint32 i2c_i2cm_cfg ;
    __IO uint32 i2c_i2cm_tar ;
    __IO uint32 i2c_i2cs_sar ;
    __IO uint32 i2c_byte_fifo_cnt ;
    __IO uint32 i2c_i2cm_transfer_len ;
    __IO uint32 i2c_tfifo_idle_data ;
    __IO uint32 i2c_status ;
    __IO uint32 i2c_fifo_clr ;
} I2C_TypeDef;


#endif
