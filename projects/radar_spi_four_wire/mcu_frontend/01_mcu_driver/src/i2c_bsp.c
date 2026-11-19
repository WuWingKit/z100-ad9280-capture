/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    i2c_bsp.c
 * @original_author  WangFan(fwang@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-1-03
 * @brief   This file provides a I2C universal peripheral function interface, making it convenient
 *          for users to directly call it
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "gpio_bsp.h"
#include "adt6101P_system.h"
#include "i2c_bsp.h"


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   I2C0 and I2C1 as master init.
 *
 * @code
 * i2cInitTypedef i2cStr;
 * i2cStr.i2cMode = MASTER_MODE;
 * i2cStr.i2cBitAddr = ADDR_BIT_7;
 * i2cStr.i2cClockSpeed = 200000; //set i2c clock 200K
 * i2cStr.i2cOwnAddr = 0x3A;
 * i2cStr.i2cTarAddr = 0x11;
 * i2cInit(I2C0, &i2cStr);
 * @endcode
 * @param   i2cX: i2c include I2C0 I2C1.
 * @param   i2cInitStruct: pointer to a I2C_InitTypeDef structure that, see also @ref i2cInitTypedef.
 *   contains the configuration information for the specified I2C peripheral.
 * @return  None.
 */
void i2cInit(I2C_TypeDef *i2cX, i2cInitTypedef *i2cInitStruct)
{
    /* set gpio cfg */
    gpioInitStrTypedef  gpioInitStruct;
    gpioInitStruct.Pull = GPIO_PULL_UP;
    if(i2cX == I2C0)
    {
        gpioInitStruct.Pin  =  GPIO_PIN_7 | GPIO_PIN_8;

        /* gpio initialization:The default pid is gpio,
           the pid must be reconfigured.*/
        gpioInit(&gpioInitStruct);
        setGpioConfigPid(GPIO_CFG_P7, PID_IIC0_SCL);
        setGpioConfigPid(GPIO_CFG_P8, PID_IIC0_SDA);
    }
    else
    {
        gpioInitStruct.Pin  =  GPIO_PIN_17 | GPIO_PIN_18;
        gpioInit(&gpioInitStruct);
        setGpioConfigPid(GPIO_CFG_P17, PID_IIC1_SCL);
        setGpioConfigPid(GPIO_CFG_P18, PID_IIC1_SDA);
    }

    /* init i2c struct */
    if(i2cInitStruct->i2cMode == MASTER_MODE)
    {
        /* master mode */
        setI2cMode(i2cX, MASTER_MODE);

        /* select i2c 10bit addressing mode or 7bit */
        if(i2cInitStruct->i2cBitAddr == ADDR_BIT_7)
        {
            setI2cmCtr(i2cX, 0x80);
        }
        else
        {
            setI2cmCtr(i2cX, 0xC0);
        }
    }
    else
    {
        /* slave mode */
        setI2cMode(i2cX, SLAVE_MODE);
    }

    /* set i2c rate */
    int speed = 0;
    
    sysHighClkTypedef sysPclk = getPclk();
    // value= (50M/(5*f))-1, f = 50M / (5(value+1))
    speed = sysPclk * 1000000 / (5 * (i2cInitStruct->i2cClockSpeed - 1));
    
    setI2cmPrer(i2cX, speed);

    /* set target slave address */
    setI2cmTar(i2cX, i2cInitStruct->i2cTarAddr);

    /* set own address */
    setI2csSar(i2cX, i2cInitStruct->i2cOwnAddr);

    /* init i2c interrupt mask */
    // setI2cInterruptMask(i2cX, MASTER_IRQ_MASK, SET);
    
    if(i2cX == I2C0)
    {
        setIntHandlerEn(INT_I2C0, ENABLEINT);
    }
    else if(i2cX == I2C1)
    {
        setIntHandlerEn(INT_I2C1, ENABLEINT);
    }
    
}

/**
 * @brief   I2C0 and I2C1 receive 1byte data.
 * @param   i2cX: i2c include I2C0 I2C1.
 * @return  None.
 */
uint8 i2cRecvByte(I2C_TypeDef *i2cX)
{
    uint8 receive = 0;

    /* configuration for master mode */
    setI2cmCr(i2cX, (i2cmClrTypedef)0xe8, SET);

    /* get data from i2c */
    receive = getI2cRfifoData(i2cX);

    return receive;
}

/**
 * @brief   I2C0 and I2C1 send 1byte data.
 * @param   i2cX: i2c include I2C0 I2C1.
 * @return  None.
 */
void i2cSendByte(I2C_TypeDef *i2cX, uint8 dataToWrite)
{
    //start , write , ack and stop command
    setI2cTfifoData(i2cX, dataToWrite);

    setI2cmCr(i2cX, (i2cmClrTypedef)0xd8, SET);
}

