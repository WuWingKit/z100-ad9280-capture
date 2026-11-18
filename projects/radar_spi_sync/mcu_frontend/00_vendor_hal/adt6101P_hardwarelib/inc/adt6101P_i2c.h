/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_i2c.h
 * @original_author  WangFan(fwang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-1-03
 * @brief   Header file containing functions prototypes of I2C HAL library
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __ADT6101P_I2C_H
#define __ADT6101P_I2C_H

/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "i2c_ctype_map.h"
#include <stdio.h>

/** @addtogroup ADT6101P_HardWareLib
 *  @{
 */

/** @addtogroup I2C
 *  @{
 */

/** @addtogroup I2C_Private_Macros
  * @{
  */


/* Typedef  ------------------------------------------------------------------*/
typedef void (*I2C_HANDLER)(const uint16 mode, const uint8 data);


/**
  * @brief  Configuration i2c status enumeration
  */
typedef enum
{
    REC_FIFO_TIME_OUT   = 1, // receive FIFO time out
    TX_FIFO_UNDER_THLD  = 2, // transmit FIFO under threshold
    TX_FIFO_EMPTY       = 3, // transmit FIFO empty
    TX_FIFO_OVER_READ   = 4, // transmit FIFO over read when transmit FIFO empty
    TX_FIFO_OVER_FULL   = 5, // transmit FIFO over write when transmit FIFO full
    RX_FIFO_OVER_THLD   = 6, // receive FIFO over threshold
    RX_FIFO_FULL        = 7, // receive FIFO full
    RX_FIFO_NOT_EMPTY   = 8, // receive FIFO not empty
    RX_FIFO_OVER_EMPTY  = 9, // receive FIFO over read when receive FIFO empty
    MASTER_TR_COMPLETER = 10, // master transfer completed flag or arbitration is lost
    SLAVE_BUS_ERROR     = 11, // slave bus error, 1:slave received misplaced start or misplaced stop
    SLAVE_ADDRESS_MATCH = 12  //slave address match when as slave mode
} i2cStatusTypedef;

/**
  * @brief  Configuration i2c mask enumeration
  *         mask 1: this interrupt can be read by host
  *         mask 0: this interrupt is 0
  */
typedef enum
{
    RX_TO_MASK            = 1,
    TFIFO_UNTH_MASK       = 2,
    TFIFO_EMPTY_MASK      = 3,
    TFIFO_UDFL_MASK       = 4,
    TFIFO_OVFL_MASK       = 5,
    RFIFO_OVTH_MASK       = 6,
    RFIFO_FULL_MASK       = 7,
    RFIFO_EMPTY_MASK      = 8,
    RFIFO_UDFL_MASK       = 9,
    MASTER_IRQ_MASK       = 10,
    SLAVE_IR_PHY_ERR_MASK = 11,
    SLAVE_ADDR_MATCH_MASK = 12
} i2cMaskTypedef;

/**
  * @brief  Configuration i2c control enumeration
  */
typedef enum
{
    RX_TO_CLR            = 1,
    TFIFO_UNTH_CLR       = 2,
    TFIFO_EMPTY_CLR      = 3,
    TFIFO_UDFL_CLR       = 4,
    TFIFO_OVFL_CLR       = 5,
    RFIFO_OVTH_CLR       = 6,
    RFIFO_FULL_CLR       = 7,
    RFIFO_EMPTY_CLR      = 8,
    RFIFO_UDFL_CLR       = 9,
    MASTER_IRQ_CLR       = 10,
    SLAVE_IR_PHY_ERR_CLR = 11,
    SLAVE_ADDR_MATCH_CLR = 12
} i2cClrTypedef;

/**
  * @brief  Configuration i2c master control enumeration
  */
typedef enum
{
    I2C_RESTART = 1,
    I2C_ACK     = 2,
    I2C_WRITE   = 3,
    I2C_READ    = 4,
    I2C_STOP    = 5,
    I2C_START   = 6
} i2cmClrTypedef;

/**
  * @brief  Configuration i2cNack enumeration
  */
typedef enum
{
    HAEDWARE_NACK = 0,
    SOFTWARE_NACK = 1
} i2cNackTypedef;

/**
  * @brief  Configuration i2c slave bit addr enumeration
  */
typedef enum
{
    SLAVE_7BIT  = 0,
    SLAVE_10BIT = 1
} i2cSlaveBitTypedef;

/**
  * @brief  Configuration i2c slave or master enumeration
  */
typedef enum
{
    SLAVE_MODE  = 0,
    MASTER_MODE = 1
} i2cModeTypedef;
/**
  * @}
  */


/* Functions -----------------------------------------------------------------*/
/** @addtogroup I2C_private_Functions
  * @{
  */
void setI2cHandler(I2C_TypeDef *i2cX, const I2C_HANDLER handler);
void setI2cTfifoData(I2C_TypeDef *i2cX, const uint8 data);
uint8 getI2cRfifoData(I2C_TypeDef *i2cX);
uint8 getI2cStatus(I2C_TypeDef *i2cX, i2cStatusTypedef statu);
void setI2cInterruptMask(I2C_TypeDef *i2cX, i2cMaskTypedef mask, bitStateTypedef able);
uint8 getI2cInterruptMask(I2C_TypeDef *i2cX, i2cMaskTypedef mask);
void setI2cClr(I2C_TypeDef *i2cX, i2cClrTypedef value, bitStateTypedef able);
void setI2cmCr(I2C_TypeDef *i2cX, i2cmClrTypedef value, bitStateTypedef able);
uint32 getI2cmCr(I2C_TypeDef *i2cX, i2cmClrTypedef value);
void setI2csNack(I2C_TypeDef *i2cX, i2cNackTypedef nack);
i2cNackTypedef getI2csNack(I2C_TypeDef *i2cX);
void setI2cs10BitAddr(I2C_TypeDef *i2cX, i2cSlaveBitTypedef bit);
i2cSlaveBitTypedef getI2cs10BitAddr(I2C_TypeDef *i2cX);
void setI2cMode(I2C_TypeDef *i2cX, i2cModeTypedef mode);
i2cModeTypedef getI2cMode(I2C_TypeDef *i2cX);
uint8 setI2cRxThld(I2C_TypeDef *i2cX, const uint8 thld);
uint8 getI2cRxThld(I2C_TypeDef *i2cX);
uint8 setI2cRfifoThld(I2C_TypeDef *i2cX, const uint8 thld);
uint8 getI2cRfifoThld(I2C_TypeDef *i2cX);
uint8 setI2cTfifoThld(I2C_TypeDef *i2cX, const uint8 thld);
uint8 getI2cTfifoThld(I2C_TypeDef *i2cX);
uint8 setI2cmPrer(I2C_TypeDef *i2cX, const uint16 value);
uint16 getI2cmPrer(I2C_TypeDef *i2cX);
void setI2cmCtr(I2C_TypeDef *i2cX, const uint8 value);
uint8 getI2cmCtr(I2C_TypeDef *i2cX);
void setI2cmTar(I2C_TypeDef *i2cX, const uint16 value);
uint16 getI2cmTar(I2C_TypeDef *i2cX);
void setI2csSar(I2C_TypeDef *i2cX, const uint16 value);
uint16 getI2csSar(I2C_TypeDef *i2cX);
uint8 getI2cByteFifoRest(I2C_TypeDef *i2cX);
uint8 getI2cRfifoCnt(I2C_TypeDef *i2cX);
uint8 getI2cTfifoCnt(I2C_TypeDef *i2cX);
void setI2cmTransferLen(I2C_TypeDef *i2cX, const uint16 value);
uint16 getI2cmTransferLen(I2C_TypeDef *i2cX);
void setI2cTfifoIdle(I2C_TypeDef *i2cX, const uint8 value);
uint8 getI2cTfifoIdle(I2C_TypeDef *i2cX);
uint8 getI2csRdOpStatus(I2C_TypeDef *i2cX);
uint8 getI2csWrOpStatus(I2C_TypeDef *i2cX);
uint8 getI2csBusyStatus(I2C_TypeDef *i2cX);
uint8 getI2cmStatus(I2C_TypeDef *i2cX);
void setI2cFifoClr(I2C_TypeDef *i2cX);

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
