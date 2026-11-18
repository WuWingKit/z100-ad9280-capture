/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    gpio_bsp.c
 * @original_author  ZhengBo(bzheng@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-18
 * @brief   This file references the GPIO HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "gpio_bsp.h"
#include "gpio_ctype_map.h"


/* Exported functions --------------------------------------------------------*/
/**
 * @brief   GPIO Init.
 *
 * Example:
 * @code
 * gpioInitStrTypedef  gpioInitStruct;
 * gpioInitStruct.Mode  = GPIO_MODE_OUTPUT;
 * gpioInitStruct.Pull  = GPIO_PULL_UP;
 * gpioInitStruct.Pin   = GPIO_PIN_2 | GPIO_PIN_9;
 * gpioInit(&gpioInitStruct);
 * @endcode
 * @param   GPIO_Init: gpio init.
 * @return  None.
 */
void gpioInit(gpioInitStrTypedef *gpioInitStr)
{
    uint32 position = 0x00u;
    uint32 ioposition;
    uint32 iocurrent;

    while(((gpioInitStr->Pin) >> position) != 0x00u)
    {
        /* Get the IO position */
        ioposition = (0x01uL << position);

        /* Get the current IO position */
        iocurrent = (uint32)(gpioInitStr->Pin) & ioposition;

        if(iocurrent == ioposition)
        {
            /*set gpio mode */
            setGpioConfigMode((gpioConfigPinTypedef)position, gpioInitStr->Mode);

            /*set gpio mode */
            setGpioConfigPull((gpioConfigPinTypedef)position, gpioInitStr->Pull);

            /*set gpio pid */
            setGpioConfigPid((gpioConfigPinTypedef)position, PID_GPIO);
        }

        position++;
    }
}

/**
 * @brief   gpio Read Pin.
 *
 * @code
 * gpioInitStrTypedef  gpio29InitStruct;
 * gpio29InitStruct.Mode  = GPIO_MODE_INPUT;
 * gpio29InitStruct.Pin   = GPIO_PIN_29;
 * gpioInit(&gpio29InitStruct);
 * gpioReadPin(GPIO_PIN_29);
 * @endcode
 * @param   gpioPin: gpio Pin define.
 * @return  1: set  0: reset.
 */
gpioPinStateTypedef gpioReadPin(uint32 gpioPin)
{
    gpioPinStateTypedef bitStatus;

    if((GPIO->gpio_in & gpioPin) != (uint32)GPIO_PIN_RESET)
    {
        bitStatus = GPIO_PIN_SET;
    }
    else
    {
        bitStatus = GPIO_PIN_RESET;
    }

    return bitStatus;
}

/**
 * @brief   GPIO set output high or low level.
 *
 * @code
 * gpioInitStrTypedef  gpio29InitStruct;
 * gpio29InitStruct.Mode  = GPIO_MODE_OUTPUT;
 * gpio29InitStruct.Pull  = GPIO_PULL_UP;
 * gpio29InitStruct.Pin   = GPIO_PIN_29;
 * gpioInit(&gpio29InitStruct);
 * gpioWritePin(GPIO_PIN_29, GPIO_PIN_SET);
 * @endcode
 * @param   gpioPin: gpio Pin define.
 * @param   pinState: 1: set  0: reset.
 * @return  None.
 */
void gpioWritePin(uint32 gpioPin, gpioPinStateTypedef pinState)
{
    if(pinState != GPIO_PIN_RESET)
    {
        GPIO->gpio_out_set |= gpioPin;
    }
    else
    {
        GPIO->gpio_out_clear |= gpioPin;
    }
}

/**
 * @brief   gpio De Init
 * @param   gpioPin: gpio Pin define.
 * @return  None.
 */
void gpioDeInit(uint32 gpioPin)
{
    uint32 position = 0x00u;
    uint32 ioposition;
    uint32 iocurrent;

    while(((gpioPin) >> position) != 0x00u)
    {
        /* Get the IO position */
        ioposition = (0x01uL << position);

        /* Get the current IO position */
        iocurrent = (uint32)(gpioPin) & ioposition;

        if(iocurrent == ioposition)
        {
            *((uint32 *)(GPIO_CONFIG_REGS + position)) = 0x10;

            setGpioInterruptEnable(gpioPin, DISABLE); //GPIOX interrupt disable

            setGpioWakeEnable(gpioPin, DISABLE);
        }

        position++;
    }
}
