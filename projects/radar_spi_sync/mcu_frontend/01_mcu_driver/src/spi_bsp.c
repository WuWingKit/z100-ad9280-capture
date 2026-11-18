/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    spi_bsp.c
 * @original_author  DingZhen(zding@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file references the SPI HAL library and combines it into a
 *          universal interface function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "spi_bsp.h"
#include "gpio_bsp.h"
#include "adt6101P_system.h"

/* Exported functions --------------------------------------------------------*/
/**
 * @brief   spiInit.
 *
 * @code
 * s_spi0Init.SpiDataSize = SPI_DATASIZE_8BIT;
 * s_spi0Init.SpiPrescaler = SPI_Prescaler_16;
 * s_spi0Init.SpiGaterMode = SPI_CPOL_Low_CPHA_1Edge;
 * s_spi0Init.SpiMode = SPI_Master;
 * s_spi0Init.SpiNSS = SPI_NSS_Soft;
 * s_spi0Init.intEn = SPI_INTSTATUS_ROR | SPI_INTSTATUS_RT;
 * s_spi0Init.intHandler = spi0InterruptHandler;
 * spiInit(SPI0, &s_spi0Init);
 * @endcode
 * @param   SpiX: SPI0/SPI1.
 * @param   SpiInitStrTypedef: SPI setting structure, see also @ref SpiInitStrTypedef.
 * @return  None.
 */
void spiInit(SPI_TypeDef *SpiX, SpiInitStrTypedef *spiInitStr)
{
    /* Set the SPI Gpio Init */
    spiGpioInit(SpiX, spiInitStr);
    
    /* Set the SPI CS Init */
    /* Transmit 8~16 bit at a time using  SPI_NSS_Hard */
    /* Use SPI_NSS_Soft to transfer more than 16 bit at a time */
    spiCsInit(SpiX, spiInitStr);

    /* Set the SPI communication mode (CPOL and CPHA) */
    setSpiGaterMode(SpiX, spiInitStr->SpiGaterMode);

    /* Set the SPI data size */
    setSpiDataSize(SpiX, spiInitStr->SpiDataSize);
    /* Set the SPI mode (Master or Slave) */
    setSpiMode(SpiX, spiInitStr->SpiMode);

    /* Set the SPI clock prescaler */
    setSpiClockPrescaler(SpiX, spiInitStr->SpiPrescaler);

    /* Enable SPI */
    setSpiSsp(SpiX, 1);

    /* Enable SPI Interrupt */
    enableSpiInterrupt(SpiX, spiInitStr->intEn);

    setSpiIntHandler(SpiX, spiInitStr->intHandler);

    if(SpiX == SPI0)
    {
        setIntHandlerEn(INT_SPI0, ENABLEINT);
    }
    else if(SpiX == SPI1)
    {
        setIntHandlerEn(INT_SPI1, ENABLEINT);
    }
}

/**
 * @brief   spiInit.
 * @param   SpiX: SPI0/SPI1.
 * @param   SpiInitStrTypedef: SPI setting structure, see also @ref SpiInitStrTypedef.
 * @return  None.
 */
void spiGpioInit(SPI_TypeDef *SpiX, SpiInitStrTypedef *spiInitStr)
{

    gpioInitStrTypedef  spiGpioInitStruct;

    spiGpioInitStruct.Mode = GPIO_MODE_HIGH_INPENDECE;
    spiGpioInitStruct.Pull = GPIO_NO_PULL;

    if(SpiX == SPI0)
    {
        spiGpioInitStruct.Pin  = GPIO_PIN_3 | GPIO_PIN_5 | GPIO_PIN_6;
        gpioInit(&spiGpioInitStruct);

        spiGpioInitStruct.Pin = GPIO_PIN_3;
        setGpioConfigPid(GPIO_CFG_P3, (gpioPidTypedef)(PID_SPI0_CLK_MASTER + spiInitStr->SpiMode));

        spiGpioInitStruct.Pin = GPIO_PIN_5;
        setGpioConfigPid(GPIO_CFG_P5, (gpioPidTypedef)(PID_SPI0_MISO_MASTER + spiInitStr->SpiMode));

        spiGpioInitStruct.Pin = GPIO_PIN_6;
        setGpioConfigPid(GPIO_CFG_P6, (gpioPidTypedef)(PID_SPI0_MOSI_MASTER + spiInitStr->SpiMode));
    }
    else if(SpiX == SPI1)
    {
        spiGpioInitStruct.Pin  = GPIO_PIN_13 | GPIO_PIN_15 | GPIO_PIN_16;
        gpioInit(&spiGpioInitStruct);

        spiGpioInitStruct.Pin = GPIO_PIN_13;
        setGpioConfigPid(GPIO_CFG_P13, (gpioPidTypedef)(PID_SPI1_CLK_MASTER + spiInitStr->SpiMode));

        spiGpioInitStruct.Pin = GPIO_PIN_15;
        setGpioConfigPid(GPIO_CFG_P15, (gpioPidTypedef)(PID_SPI1_MISO_MASTER + spiInitStr->SpiMode));

        spiGpioInitStruct.Pin = GPIO_PIN_16;
        setGpioConfigPid(GPIO_CFG_P16, (gpioPidTypedef)(PID_SPI1_MOSI_MASTER + spiInitStr->SpiMode));
    }

}

/**
 * @brief   spiCsInit
 * @param   SpiX: SPI0/SPI1.
 * @return  None.
 */
void spiCsInit(SPI_TypeDef *SpiX, SpiInitStrTypedef *spiInitStr)
{
    gpioInitStrTypedef  gpioInitStruct;


    if(SpiX == SPI0)
    {
        if(spiInitStr->SpiNSS == SPI_NSS_Soft)
        {
            gpioInitStruct.Mode  = GPIO_MODE_OUTPUT;
            gpioInitStruct.Pull  = GPIO_PULL_UP;
            gpioInitStruct.Pin   = GPIO_PIN_4;
            gpioInit(&gpioInitStruct);
        }
        else
        {
            gpioInitStruct.Mode  = GPIO_MODE_HIGH_INPENDECE;
            gpioInitStruct.Pull  = GPIO_NO_PULL;
            gpioInitStruct.Pin   = GPIO_PIN_4;
            gpioInit(&gpioInitStruct);
            setGpioConfigPid(GPIO_CFG_P4, (gpioPidTypedef)(PID_SPI0_CS_MASTER + spiInitStr->SpiMode)); 
        }
    }
    else if(SpiX == SPI1)
    {
        if(spiInitStr->SpiNSS == SPI_NSS_Soft)
        {
            gpioInitStruct.Mode  = GPIO_MODE_OUTPUT;
            gpioInitStruct.Pull  = GPIO_PULL_UP;
            gpioInitStruct.Pin   = GPIO_PIN_14;
            gpioInit(&gpioInitStruct);
        }
        else
        {
            gpioInitStruct.Mode  = GPIO_MODE_HIGH_INPENDECE;
            gpioInitStruct.Pull  = GPIO_NO_PULL;
            gpioInitStruct.Pin   = GPIO_PIN_14;
            gpioInit(&gpioInitStruct);
            setGpioConfigPid(GPIO_CFG_P14, (gpioPidTypedef)(PID_SPI1_CS_MASTER + spiInitStr->SpiMode)); 
        }

    }
}

/**
 * @brief   Spi Cs Select。
 * @param   SpiX: SPI0/SPI1.
 * @return  None.
 */
void spiCsSelect(SPI_TypeDef *SpiX)
{
    if(SpiX == SPI0)
    {
        gpioWritePin(GPIO_PIN_4, GPIO_PIN_SET);
    }
    else if(SpiX == SPI1)
    {
        gpioWritePin(GPIO_PIN_14, GPIO_PIN_SET);
    }
}

/**
 * @brief   Spi Cs not Select。
 * @param   SpiX: SPI0/SPI1.
 * @return  None.
 */
void spiCsDeselect(SPI_TypeDef *SpiX)
{
    if(SpiX == SPI0)
    {
        gpioWritePin(GPIO_PIN_4, GPIO_PIN_RESET);
    }
    else if(SpiX == SPI1)
    {
        gpioWritePin(GPIO_PIN_14, GPIO_PIN_RESET);
    }
}


/**
 * @brief   Waiting for the SPI transmission status to become idle status.
 * @param   SpiX: SPI0/SPI1.
 * @return  None.
 */
void waitBsy(SPI_TypeDef *SpiX)
{
    while(isSpiBusy(SpiX));
}


/**
 * @brief   flashPd.
 * @param   SpiX: SPI0/SPI1.
 * @return  None.
 */
void flashPd(SPI_TypeDef *SpiX)
{
    spiReadWriteByte(SpiX, 0xB9);

    waitBsy(SpiX);
}


/**
 * @brief   flash release pd.
 * @param   SpiX: SPI0/SPI1.
 * @return  None.
 */
void flashReleasePd(SPI_TypeDef *SpiX)
{
    spiReadWriteByte(SpiX, 0xAB);

    waitBsy(SpiX);
}
