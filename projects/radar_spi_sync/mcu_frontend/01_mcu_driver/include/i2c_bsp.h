/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    i2c_bsp.h
 * @original_author  WangFan(fwang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-1-03
 * @brief   This file references the PMU HAL library and combines it into a universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
#ifndef __I2C_BSP_H
#define __I2C_BSP_H

/* Includes ------------------------------------------------------------------*/
#include "adt6101P_type_define.h"
#include "i2c_ctype_map.h"
#include "adt6101P_i2c.h"
#include "sys_bsp.h"

/** @addtogroup BSP
 *  @{
 */

/** @addtogroup I2C_BSP
 *  @{
 */

/* Exported types ------------------------------------------------------------*/
/** @addtogroup I2C_Exported_Macros
  * @{
  */
typedef enum
{
    ADDR_BIT_10,
    ADDR_BIT_7
} i2cBitAddrTypedef;

typedef struct
{
    i2cModeTypedef      i2cMode; // as a master(MASTER_MODE) or slave(SLAVE_MODE)
    i2cBitAddrTypedef   i2cBitAddr; // 7-bit：ADDR_BIT_7 or 10-bit：ADDR_BIT_10.
    uint32              i2cClockSpeed; // Specifies the clock frequency.
    uint16              i2cOwnAddr; // Specifies the device own address.
    uint16              i2cTarAddr; // Specifies the target slave address.
} i2cInitTypedef;
/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/
/** @addtogroup I2C_Exported_Functions
  * @{
  */

void i2cInit(I2C_TypeDef *i2cX, i2cInitTypedef *i2cInitStruct);
uint8 i2cRecvByte(I2C_TypeDef *i2cX);
void i2cSendByte(I2C_TypeDef *i2cX, uint8 dataToWrite);

/**
  * @}
  */
/**
  * @}
  */
/**
  * @}
  */
#endif // !__I2C_BSP_H
