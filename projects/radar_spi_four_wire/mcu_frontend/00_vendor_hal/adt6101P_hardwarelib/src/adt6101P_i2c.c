/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_i2c.c
 * @original_author  WangFan(fwang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-1-02
 * @brief   This file contains all the functions implemented by the I2C
 *          peripheral register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "i2c_ctype_map.h"
#include "adt6101P_i2c.h"

/* Static    -----------------------------------------------------------------*/
static I2C_HANDLER s_i2cHandler[2] = {NULL, NULL};

/* Static    -----------------------------------------------------------------*/
/**
 * @brief   I2C0 interrupt handler.
 */
void i2c0Handler(void)
{
    uint8 resTemp = 0;
    uint16 state = 0;
    uint8 tmpState = 0;
    i2cMaskTypedef maskState = RX_TO_MASK;
    i2cClrTypedef  clrState = RX_TO_CLR;

    for(uint8 i = 0; i < 12; i++)
    {
        state++;
        maskState++;
        clrState++;
        tmpState = getI2cInterruptMask(I2C0, maskState);
        if(0 != tmpState)
        {
            setI2cClr(I2C0, clrState, SET);
            resTemp = getI2cRfifoData(I2C0);
            s_i2cHandler[0](state, resTemp);
        }
    }
}

/**
 * @brief   I2C1 interrupt handler.
 */
void i2c1Handler(void)
{
    uint8 resTemp = 0;
    uint16 state = 0;
    uint8 tmpState = 0;
    i2cMaskTypedef maskState = RX_TO_MASK;
    i2cClrTypedef  clrState = RX_TO_CLR;

    for(uint8 i = 0; i < 12; i++)
    {
        state++;
        maskState++;
        clrState++;
        tmpState = getI2cInterruptMask(I2C1, maskState);
        if(0 != tmpState)
        {
            setI2cClr(I2C1, clrState, SET);
            resTemp = getI2cRfifoData(I2C1);
            s_i2cHandler[1](state, resTemp);
        }
    }
}

/**
 * @brief   Set the i2c interrupt handler function.
 * @param   i2cX: Uart number:I2C0, I2C1.
 * @param   handler: the address of the uart interrupt callback function.
 * @return  None.
 */
void setI2cHandler(I2C_TypeDef *i2cX, const I2C_HANDLER handler)
{
    if(NULL != handler)
    {
        if(I2C0 == i2cX)
        {
            s_i2cHandler[0] = handler;
        }
        else if(I2C1 == i2cX)
        {
            s_i2cHandler[1] = handler;
        }
    }
}

/* Functions -----------------------------------------------------------------*/
/**
 * @brief   set tfifo_wf_data.
 * @param   i2cX: I2C0 or I2C1.
 * @param   data: i2c transmit fifo write data.
 * @return  None.
 * @note    Write Only
 */
void setI2cTfifoData(I2C_TypeDef *i2cX, const uint8 data)
{
    SET_VALUE(i2cX->i2cir_tfifo_wr_data, i2cir_tfifo_wr_data_shift, i2cir_tfifo_wr_data_mask, data);
}

/**
 * @brief   get rfifo_wf_data.
 * @param   i2cX: I2C0 or I2C1.
 * @return  i2c receive fifo read data.
 * @note    Read Only
 */
inline uint8 getI2cRfifoData(I2C_TypeDef *i2cX)
{
    return (uint8)GET_BITS_VALUE(i2cX->i2cir_rfifo_rd_data, i2cir_rfifo_rd_data_shift, i2cir_rfifo_rd_data_mask);
}

/**
 * @brief   get I2c status.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint32: [11:0] is valid; [31:12] reserved.
 * @note    Read Only
 *
 * [11]: slave address match when as slave mode
 * [10]: slave bus error, 1:slave received misplaced start or misplaced stop
 * [9] : master transfer completed flag or arbitration is lost
 * [8] : receive FIFO over read when receive FIFO empty
 * [7] : receive FIFO not empty
 * [6] : receive FIFO full
 * [5] : receive FIFO over threshold
 * [4] : transmit FIFO over write when transmit FIFO full
 * [3] : transmit FIFO over read when transmit FIFO empty
 * [2] : transmit FIFO empty
 * [1] : transmit FIFO under threshold
 * [0] : receive FIFO time out
 */
uint8 getI2cStatus(I2C_TypeDef *i2cX, i2cStatusTypedef statu)
{
    return GET_BIT_VALUE(i2cX->i2cir_int_status, statu);
}


/**
 * @brief   set i2c interrupt mask.
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: the uint32 of interrupt mask, see also @ref i2cMaskTypedef.
 *          [11:0] is valid; [31:12] reserved
 * [11]: field_i2cs_reg_addr_match_mask
 * [10]: field_i2cs_reg_ir_phy_err_mask
 * [9] : field_i2cm_reg_irq_mask
 * [8] : field_i2c_reg_rfifo_udfl_mask
 * [7] : field_i2c_reg_rfifo_empty_n_mask
 * [6] : field_i2c_reg_rfifo_full_mask
 * [5] : field_i2c_reg_rfifo_ovth_mask
 * [4] : field_i2c_reg_tfifo_ovfl_mask
 * [3] : field_i2c_reg_tfifo_udfl_mask
 * [2] : field_i2c_reg_tfifo_empty_mask
 * [1] : field_i2c_reg_tfifo_unth_mask
 * [0] : field_i2c_reg_rx_to_mask
 * @param   able: SET:set 1; DISABLE: set 0, see also @ref i2cMaskTypedef.
 * @return  None.
 * @note    mask 1: this interrupt can be read by host
            mask 0: this interrupt is 0
 */
void setI2cInterruptMask(I2C_TypeDef *i2cX, i2cMaskTypedef mask, bitStateTypedef able)
{
    if(able == SET)
    {
        SET_BIT(i2cX->i2cir_int_mask, mask);
    }
    else
    {
        CLEAR_BIT(i2cX->i2cir_int_mask, mask);
    }
}


/**
 * @brief   get i2c interrupt mask.
 * @param   i2cX: I2C0 or I2C1.
 * @param   mask: see also @ref i2cMaskTypedef.
 * @return  uint32: [11:0] is valid; [31:12] reserved.
 * @note mask 1: this interrupt can be read by host
         mask 0: this interrupt is 0
 *
 * [11]: field_i2cs_reg_addr_match_mask
 * [10]: field_i2cs_reg_ir_phy_err_mask
 * [9] : field_i2cm_reg_irq_mask
 * [8] : field_i2c_reg_rfifo_udfl_mask
 * [7] : field_i2c_reg_rfifo_empty_n_mask
 * [6] : field_i2c_reg_rfifo_full_mask
 * [5] : field_i2c_reg_rfifo_ovth_mask
 * [4] : field_i2c_reg_tfifo_ovfl_mask
 * [3] : field_i2c_reg_tfifo_udfl_mask
 * [2] : field_i2c_reg_tfifo_empty_mask
 * [1] : field_i2c_reg_tfifo_unth_mask
 * [0] : field_i2c_reg_rx_to_mask
 */
inline uint8 getI2cInterruptMask(I2C_TypeDef *i2cX, i2cMaskTypedef mask)
{
    return GET_BIT_VALUE(i2cX->i2cir_int_mask, mask);
}

/**
 * @brief   clear i2c interrupt.
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: the uint32 of interrupt mask, see also @ref i2cClrTypedef.
 * @param   able: see also @ref bitStateTypedef.
            [11:0] is valid; [31:12] reserved
 * @return  None.
 * @note    Write Only
 *
 * [11]: field_i2cs_reg_addr_match_clr
 * [10]: field_i2cs_reg_ir_phy_err_clr
 * [9] : field_i2cm_reg_irq_clr
 * [8] : field_i2c_reg_rfifo_udfl_clr
 * [7] : field_i2c_reg_rfifo_empty_n_clr
 * [6] : field_i2c_reg_rfifo_full_clr
 * [5] : field_i2c_reg_rfifo_ovth_clr
 * [4] : field_i2c_reg_tfifo_ovfl_clr
 * [3] : field_i2c_reg_tfifo_udfl_clr
 * [2] : field_i2c_reg_tfifo_empty_clr
 * [1] : field_i2c_reg_tfifo_unth_clr
 * [0] : field_i2c_reg_rx_to_clr
 */
inline void setI2cClr(I2C_TypeDef *i2cX, i2cClrTypedef value, bitStateTypedef able)
{
    if(able == SET)
    {
        SET_BIT(i2cX->i2cir_int_clr, value);
    }
    else
    {
        CLEAR_BIT(i2cX->i2cir_int_clr, value);
    }
}

/**
 * @brief   set i2c configuration for master mode.
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: the uint32 of interrupt mask, see also @ref i2cClrTypedef.
 * @param   able: see also @ref bitStateTypedef.
 * @return  None.
 * Normally, 0xe8 is for read operation and 0xd8 is for write operation.
 * When in master mode, this command will load by hardware and clear by hardware
 * [7]: start command                       [6]: stop command
 * [5]: read command                        [4]: write command
 * [3]: ack command                         [2]: restart command
 * 1'h1: need a restart operation.
 * 1'h0: don't need a restart operation.
 * [1:0]:reserved
 *
 * there is no STOP command between this two operation.
 */
void setI2cmCr(I2C_TypeDef *i2cX, i2cmClrTypedef value, bitStateTypedef able)
{
    if(able == SET)
    {
        SET_BIT(i2cX->i2c_i2cm_cr, value);
    }
    else
    {
        CLEAR_BIT(i2cX->i2c_i2cm_cr, value);
    }
}

/**
 * @brief   get i2c configuration for master mode.
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: see also @ref i2cmClrTypedef.
 * @return  uint32:[7:0] is valid; [31:8] reserved
 *          please see more information in function: setI2cCm
 */
uint32 getI2cmCr(I2C_TypeDef *i2cX, i2cmClrTypedef value)
{
    return GET_BIT_VALUE(i2cX->i2c_i2cm_cr, value);
}

/**
 * @brief   set i2cs_nack.
 * @param   i2cX: I2C0 or I2C1.
 * @param   nack: SOFTWARE_NACK:when as slave mode, software send nack after receiving data.
 *                HAEDWARE_NACK:hardware send nack automaticlly.
 * @return  None.
 * @note    default: nack is HAEDWARE_NACK
 */
void setI2csNack(I2C_TypeDef *i2cX, i2cNackTypedef nack)
{
    switch(nack)
    {
        case HAEDWARE_NACK:
        {
            CLEAR_BIT(i2cX->i2c_cfg, i2c_cfg_i2cs_nack_bit);
            break;
        }
        case SOFTWARE_NACK:
        {
            SET_BIT(i2cX->i2c_cfg, i2c_cfg_i2cs_nack_bit);
            break;
        }
        default:
            break;
    }
}

/**
 * @brief   get i2cs_nack.
 * @param   i2cX: I2C0 or I2C1.
 * @return  SOFTWARE_NACK:when as slave mode, software send nack after receiving data.
 *          HAEDWARE_NACK:hardware send nack automaticlly.
 */
i2cNackTypedef getI2csNack(I2C_TypeDef *i2cX)
{
    i2cNackTypedef mode;
    if(GET_BIT_VALUE(i2cX->i2c_cfg, i2c_cfg_i2cs_nack_shift))
    {
        mode = SOFTWARE_NACK;
    }
    else
    {
        mode = HAEDWARE_NACK;
    }
    return mode;
}

/**
 * @brief   set i2cs_10bitaddr.
 * @param   i2cX: I2C0 or I2C1.
 * @param   SLAVE_10BIT: setting slave as 10bit addressing mode, see also @ref i2cSlaveBitTypedef.
            SLAVE_7BIT: setting slave as 7 bit addressing mode.
 * @return  None.
 */
void setI2cs10BitAddr(I2C_TypeDef *i2cX, i2cSlaveBitTypedef bit)
{
    switch(bit)
    {
        case SLAVE_7BIT:
        {
            CLEAR_BIT(i2cX->i2c_cfg, i2c_cfg_i2cs_10bitaddr_bit);
            break;
        }
        case SLAVE_10BIT:
        {
            SET_BIT(i2cX->i2c_cfg, i2c_cfg_i2cs_10bitaddr_bit);
            break;
        }
        default:
            break;
    }
}

/**
 * @brief   get i2cs_10bitaddr.
 * @param   i2cX: I2C0 or I2C1.
 * @return  SLAVE_10BIT: setting slave as 10bit addressing mode.
            SLAVE_7BIT: setting slave as 7 bit addressing mode.
 */
i2cSlaveBitTypedef getI2cs10BitAddr(I2C_TypeDef *i2cX)
{
    i2cSlaveBitTypedef mode;
    if(GET_BIT_VALUE(i2cX->i2c_cfg, i2c_cfg_i2cs_10bitaddr_shift))
    {
        mode = SLAVE_10BIT;
    }
    else
    {
        mode = SLAVE_7BIT;
    }
    return mode;
}

/**
 * @brief   set i2c mode.
 * @param   i2cX: I2C0 or I2C1.
 * @param   mode: see also @ref i2cModeTypedef.
 *                  SLAVE_MODE : i2c as slave.
                    MASTER_MODE: i2c as master.
 * @return  None.
 */
void setI2cMode(I2C_TypeDef *i2cX, i2cModeTypedef mode)
{
    switch(mode)
    {
        case SLAVE_MODE:
        {
            CLEAR_BIT(i2cX->i2c_cfg, i2c_cfg_master_bit);
            break;
        }
        case MASTER_MODE:
        {
            SET_BIT(i2cX->i2c_cfg, i2c_cfg_master_bit);
            break;
        }
        default:
            break;
    }
}

/**
 * @brief   get i2c mode.
 * @param   i2cX: I2C0 or I2C1.
 * @return  SLAVE_MODE : i2c as slave.
            MASTER_MODE: i2c as master.
 */
i2cModeTypedef getI2cMode(I2C_TypeDef *i2cX)
{
    i2cModeTypedef mode;
    if(GET_BIT_VALUE(i2cX->i2c_cfg, i2c_cfg_master_shift))
    {
        mode = MASTER_MODE;
    }
    else
    {
        mode = SLAVE_MODE;
    }
    return mode;
}

/**
 * @brief   set i2c rx time out threshold.
 *          Unit is SCL period when as master.
 * @param   i2cX: I2C0 or I2C1.
 * @param   thld: i2c rx time out threshold.
 * @return  1:success; 0:fail.
 */
uint8 setI2cRxThld(I2C_TypeDef *i2cX, const uint8 thld)
{
    if(thld < 256)
    {
        SET_VALUE(i2cX->i2c_cfg, i2c_cfg_to_thld_shift, i2c_cfg_to_thld_mask, thld);
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   get i2c rx time out threshold.
 *          Unit is SCL period when as master.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint32: i2c rx time out threshold.
 */
uint8 getI2cRxThld(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_cfg, i2c_cfg_to_thld_shift, i2c_cfg_to_thld_mask);
}

/**
 * @brief   set i2c rfifo threshold.
 * @param   i2cX: I2C0 or I2C1.
 * @param   uint32: i2c rfifo threshold.
 * @return  1:set successfully; 0: set failed
 * @note    if rfifo_cnt >= rfifo_thld, then rfifo_ovth interrupt
 *          will be generated. and geneate DMA request.
 */
uint8 setI2cRfifoThld(I2C_TypeDef *i2cX, const uint8 thld)
{
    if(thld < 16)
    {
        SET_VALUE(i2cX->i2c_cfg, i2c_cfg_rfifo_thld_shift, i2c_cfg_rfifo_thld_mask, thld);
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   get i2c rfifo threshold.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint32: i2c rfifo threshold.
 * @note    if rfifo_cnt >= rfifo_thld, then rfifo_ovth interrupt
 *          will be generated. and geneate DMA request.
 */
uint8 getI2cRfifoThld(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_cfg, i2c_cfg_rfifo_thld_shift, i2c_cfg_rfifo_thld_mask);
}

/**
 * @brief   set i2c tfifo threshold.
 * @param   i2cX: I2C0 or I2C1.
 * @param   uint32: i2c tfifo threshold.
 * @return  1:set successfully; 0: set failed
 * @note    if tfifo_cnt <= tfifo_thld, then tfifo_unth interrupt
 *          will be generated. and geneate DMA request.
 */
uint8 setI2cTfifoThld(I2C_TypeDef *i2cX, const uint8 thld)
{
    if(thld < 16)
    {
        SET_VALUE(i2cX->i2c_cfg, i2c_cfg_tfifo_thld_shift, i2c_cfg_tfifo_thld_mask, thld);
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   get i2c tfifo threshold.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint32: i2c tfifo threshold.
 * @note    if tfifo_cnt <= tfifo_thld, then tfifo_unth interrupt
 *          will be generated. and geneate DMA request.
 */
uint8 getI2cTfifoThld(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_cfg, i2c_cfg_tfifo_thld_shift, i2c_cfg_tfifo_thld_mask);
}

/**
 * @brief   set i2cm clock prescale.
 * @param   i2cX: I2C0 or I2C1.
 * @param   uint32: i2c set value.
 * @return  1:set successfully; 0: set failed
 * @note    clock prescale register when as i2c master,
 *          and it is also used as rfifo timer out unit
 *          the value = Freq_pclk / (5 * Freq_SCL) - 1.
 *          Freq_pclk is PCLK's frequency and Freq_SCL is SCL's frequency.
 */
uint8 setI2cmPrer(I2C_TypeDef *i2cX, const uint16 value)
{
    if(value > 0)
    {
        SET_VALUE(i2cX->i2c_i2cm_cfg, i2c_i2cm_cfg_i2cm_prer_shift, i2c_i2cm_cfg_i2cm_prer_mask, value);
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief   get i2cm clock prescale.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint32: i2c set value.
 * @note    clock prescale register when as i2c master,
 *          and it is also used as rfifo timer out unit
 *          the value = Freq_pclk / (5 * Freq_SCL) - 1.
 *          Freq_pclk is PCLK's frequency and Freq_SCL is SCL's frequency.
 *          for 100Khz SCL, the value = 50Mhz/(5*100Khz)-1 = 99.
 */
uint16 getI2cmPrer(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_i2cm_cfg, i2c_i2cm_cfg_i2cm_prer_shift, i2c_i2cm_cfg_i2cm_prer_mask);
}



/**
 * @brief   set i2c as master ctrl.
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: a more detailed introduction is as follows.
 * @return  [7:0]: [7:5] is valid; [4:0] reserved
 *          [7]: i2c master core SET
 *          [6]: 1:i2c master 10bit addressing mode. 0:i2c master 7bit addressing mode
 *          [5]: i2c master transfer operation.
 *              1'h1 : restart operation is not the same as start operation. for example,
 *                     a write operation followed by a read operation. such as EEPROM's read memory.
 *              1'h0 : restart operation is the same as start operation.
 *          [4:0]:reserved
 */
void setI2cmCtr(I2C_TypeDef *i2cX, const uint8 value)
{
    SET_VALUE(i2cX->i2c_i2cm_cfg, i2c_i2cm_cfg_i2cm_ctr_shift, i2c_i2cm_cfg_i2cm_ctr_mask, value);
}

/**
 * @brief   get i2c as master ctrl.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint8: a more detailed introduction is as follows.
 *          [7:0]: [7:5] is valid; [4:0] reserved
 *          [7]: i2c master core SET
 *          [6]: 1:i2c master 10bit addressing mode. 0:i2c master 7bit addressing mode
 *          [5]: i2c master transfer operation.
 *              1'h1 : restart operation is not the same as start operation. for example,
 *                     a write operation followed by a read operation. such as EEPROM's read memory.
 *              1'h0 : restart operation is the same as start operation.
 *          [4:0]:reserved
 */
uint8 getI2cmCtr(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_i2cm_cfg, i2c_i2cm_cfg_i2cm_ctr_shift, i2c_i2cm_cfg_i2cm_ctr_mask);
}

/**
 * @brief   set target slave address when as master mode.
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: target slave address.
 */
void setI2cmTar(I2C_TypeDef *i2cX, const uint16 value)
{
    i2cX->i2c_i2cm_tar = value;
}

/**
 * @brief   get target slave address when as master mode.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint16: target slave address.
 */
uint16 getI2cmTar(I2C_TypeDef *i2cX)
{
    return i2cX->i2c_i2cm_tar;
}

/**
 * @brief   set own slave address when as slave mode.
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: own slave address when as slave mode
 * @return  None.
 */
void setI2csSar(I2C_TypeDef *i2cX, const uint16 value)
{
    i2cX->i2c_i2cs_sar = value;
}

/**
 * @brief   get own slave address when as slave mode.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint16: own slave address when as slave mode.
 */
uint16 getI2csSar(I2C_TypeDef *i2cX)
{
    return i2cX->i2c_i2cs_sar;
}

/**
 * @brief   get i2cm byte rest.
 *              the rest of bytes needed to be transmitted as i2c master mode,
 *              and the total number is defined by i2cm_transfer_len
 * @param   i2cX: I2C0 or I2C1.
 * @note    Read Only.
 */
uint8 getI2cByteFifoRest(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_byte_fifo_cnt, i2c_byte_fifo_cnt_i2cm_byte_rest_shift,
                          i2c_byte_fifo_cnt_i2cm_byte_rest_mask);
}

/**
 * @brief   get receive fifo count indicator
 * @param   i2cX: I2C0 or I2C1.
 * @note    Read Only.
 */
uint8 getI2cRfifoCnt(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_byte_fifo_cnt, i2c_byte_fifo_cnt_rfifo_cnt_shift, i2c_byte_fifo_cnt_rfifo_cnt_mask);
}

/**
 * @brief   get transmit fifo count indicator
 * @param   i2cX: I2C0 or I2C1.
 * @note    Read Only.
 */
uint8 getI2cTfifoCnt(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_byte_fifo_cnt, i2c_byte_fifo_cnt_tfifo_cnt_shift, i2c_byte_fifo_cnt_tfifo_cnt_mask);
}

/**
 * @brief   set transfer len
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: a more detailed introduction is as follows.
 * @note    uint16: [15:8]: i2c master restart transfer length
 *                  [ 7:0]: i2c master start transfer length, it's effective in both write and read operation.
 */
void setI2cmTransferLen(I2C_TypeDef *i2cX, const uint16 value)
{
    i2cX->i2c_i2cm_transfer_len = value;
}

/**
 * @brief   get transfer len
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint16: [15:8]: i2c master restart transfer length.
 *                  [ 7:0]: i2c master start transfer length, it's effective in both write and read operation.
 */
uint16 getI2cmTransferLen(I2C_TypeDef *i2cX)
{
    return i2cX->i2c_i2cm_transfer_len;
}

/**
 * @brief   set read tfifo when tfifo empty, get tfifo_idle_data.
 * @param   i2cX: I2C0 or I2C1.
 * @param   value: a more detailed introduction is as follows.
 */
void setI2cTfifoIdle(I2C_TypeDef *i2cX, const uint8 value)
{
    i2cX->i2c_tfifo_idle_data = value;
}

/**
 * @brief   get read tfifo when tfifo empty, get tfifo_idle_data.
 * @param   i2cX: I2C0 or I2C1.
 * @return  uint8: [ 7:0]: read tfifo when tfifo empty, get tfifo_idle_data.
 */
uint8 getI2cTfifoIdle(I2C_TypeDef *i2cX)
{
    return i2cX->i2c_tfifo_idle_data;
}

/**
 * @brief   get I2c as slave read operation status.
 * @param   i2cX: I2C0 or I2C1.
 * @note    Read Only!
 */
uint8 getI2csRdOpStatus(I2C_TypeDef *i2cX)
{
    return GET_BIT_VALUE(i2cX->i2c_status, i2c_status_i2cs_rd_op_shift);
}

/**
 * @brief   get I2c as slave write operation status.
 * @param   i2cX: I2C0 or I2C1.
 * @note    Read Only!
 */
uint8 getI2csWrOpStatus(I2C_TypeDef *i2cX)
{
    return GET_BIT_VALUE(i2cX->i2c_status, i2c_status_i2cs_wr_op_shift);
}

/**
 * @brief   get I2c as slave FSM busy status.
 * @param   i2cX: I2C0 or I2C1.
 * @note    Read Only!
 */
uint8 getI2csBusyStatus(I2C_TypeDef *i2cX)
{
    return GET_BIT_VALUE(i2cX->i2c_status, i2c_status_i2cs_busy_shift);
}

/**
 * @brief   get I2c as master some status.
 * @param   i2cX: I2C0 or I2C1.
 * @note    uint8:  [4] i2c BUSY signal between start and stop when in i2c master mode. 0: not busy. 1: busy.
 *                  [3] received ACK signal when in i2c master mode. 0: received ACK. 1: received NACK.
 *                  [2] arbitration lost bit status
 *                  [1] transfer in progress
 *                  [0] arbitration lost or transfer finished status.
 *          Read Only!
 */
uint8 getI2cmStatus(I2C_TypeDef *i2cX)
{
    return GET_BITS_VALUE(i2cX->i2c_status, i2c_status_i2cm_sr_shift, i2c_status_i2cm_sr_mask);
}

/**
 * @brief   set I2c fifo clear.
 * @param   i2cX: I2C0 or I2C1.
 * @note    Write Only!
 */
void setI2cFifoClr(I2C_TypeDef *i2cX)
{
    SET_BIT(i2cX->i2c_fifo_clr, 1);
}

