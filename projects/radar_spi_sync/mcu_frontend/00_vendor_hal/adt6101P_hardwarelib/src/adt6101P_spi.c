/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 高; basis recorded in AUTHORS.json.
 */
/**
 * *****************************************************************************
 * @file    adt6101P_spi.c
 * @original_author  DingZhen(zding@andartechs.com.cn)
 * @version 3.0.0
 * @date    2024-01-22
 * @brief   This file contains all the functions implemented by the SPI peripheral
 *          register function
 *
 *                 Copyright (c) 2024, Andar Technologies Inc.
 *                           www.andartechs.com
 *
 * *****************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "adt6101P_spi.h"

/* Private variables ---------------------------------------------------------*/
static SPI_HANDLER s_spiHandler[3] = {NULL, NULL};


/* Private functions ---------------------------------------------------------*/
/**
 * @brief   get the Serial Clock Rate (SCR) setting from SPICR0 register for the specified SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current SCR setting from SPICR0 register.
 */
uint16 getSpiSCR(SPI_TypeDef *SpiX)
{
    return GET_BITS_VALUE(SpiX->SPICR0, spicr0_scr_shift, spicr0_scr_mask);
}

/**
 * @brief   Set the Serial Clock Rate (SCR) in the SPICR0 register for the specified SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   scr: Value to set the SCR to.
 */
void setSpiSCR(SPI_TypeDef *SpiX, uint16 scr)
{
    SET_VALUE(SpiX->SPICR0, spicr0_scr_shift, spicr0_scr_mask, scr);
}

/**
 * @brief   Configure the Clock Polarity and Phase for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   spiGaterMode: SPI gate mode to set (e.g., SPI_CPOL_High_CPHA_1Edge), see also @ref spiGaterModeTypedef.
 */
void setSpiGaterMode(SPI_TypeDef *SpiX, spiGaterModeTypedef spiGaterMode)
{
    switch(spiGaterMode)
    {
        case(SPI_CPOL_High_CPHA_1Edge):
        {
            SET_BIT(SpiX->SPICR0, spicr0_spo_bit);
            CLEAR_BIT(SpiX->SPICR0, spicr0_sph_bit);
            break;
        }
        case(SPI_CPOL_High_CPHA_2Edge):
        {
            SET_BIT(SpiX->SPICR0, spicr0_spo_bit);
            SET_BIT(SpiX->SPICR0, spicr0_sph_bit);
            break;
        }
        case(SPI_CPOL_Low_CPHA_1Edge):
        {
            CLEAR_BIT(SpiX->SPICR0, spicr0_spo_bit);
            CLEAR_BIT(SpiX->SPICR0, spicr0_sph_bit);
            break;
        }
        case(SPI_CPOL_Low_CPHA_2Edge):
        {
            CLEAR_BIT(SpiX->SPICR0, spicr0_spo_bit);
            SET_BIT(SpiX->SPICR0, spicr0_sph_bit);
            break;
        }
        default:
        {
            break;
        }
    }
}

/**
 * @brief   Get the current Clock Polarity and Phase mode for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current Clock Polarity and Phase mode setting.
 */
spiGaterModeTypedef getSpiGaterMode(SPI_TypeDef *SpiX)
{
    uint8 spoGaterMode;
    uint8 sphGaterMode;

    spoGaterMode =  GET_BIT_VALUE(SpiX->SPICR0, spicr0_spo_shift);
    sphGaterMode =  GET_BIT_VALUE(SpiX->SPICR0, spicr0_sph_shift);

    if(spoGaterMode == SET && sphGaterMode == RESET)
    {
        return SPI_CPOL_High_CPHA_1Edge;
    }
    else if(spoGaterMode == SET && sphGaterMode == SET)
    {
        return SPI_CPOL_High_CPHA_2Edge;
    }
    else if(spoGaterMode == RESET && sphGaterMode == RESET)
    {
        return SPI_CPOL_Low_CPHA_1Edge;
    }
    else if(spoGaterMode == RESET && sphGaterMode == SET)
    {
        return SPI_CPOL_Low_CPHA_2Edge;
    }

    return ERROR_MODE;
}

/**
 * @brief   spi write 1 byte data .
 * @param   SpiX: SPI0/SPI1.
 * @param   txData: tx data.
 * @return  None.
 */
void spiWrite(SPI_TypeDef *SpiX, uint8 txData)
{
    while(GET_BIT_VALUE(SpiX->SPISR, spisr_bsy_shift));

    while(!GET_BIT_VALUE(SpiX->SPISR, spisr_tnf_shift));

    SpiX->SPIDR = txData;

    while(!GET_BIT_VALUE(SpiX->SPISR, spisr_bsy_shift));
}


/**
 * @brief   spi read 1 byte data .
 * @param   SpiX: SPI0/SPI1.
 * @return  None.
 */
uint8 spiRead(SPI_TypeDef *SpiX)
{
    uint8 readData;

    readData = SpiX->SPIDR;

    while(!GET_BIT_VALUE(SpiX->SPISR, spisr_rne_shift));

    return readData;
}


/**
 * @brief   spi batch write data.
 * @param   SpiX: SPI0/SPI1.
 * @param   pbuf: wirte data buf.
 * @param   len: .
 * @return  None.
 */
void spiWriteBurst(SPI_TypeDef *SpiX, uint8 *pbuf, uint16 len)
{
    for(uint16 i = 0; i < len; i ++)
    {
        while(!GET_BIT_VALUE(SpiX->SPISR, spisr_tnf_shift));

        SpiX->SPIDR = pbuf[i];
    }

    while(GET_BIT_VALUE(SpiX->SPISR, spisr_bsy_shift));
}

/**
 * @brief   spi read burst.
 * @param   SpiX: SPI0/SPI1.
 * @param   pbuf: wirte data buf.
 * @param   len: .
 * @note
 */
void spiReadBurst(SPI_TypeDef *SpiX, uint8 *pbuf, uint16 len)
{
    for(uint16 i = 0; i < len; i ++)
    {
        while(!GET_BIT_VALUE(SpiX->SPISR, spisr_tnf_shift));

        SpiX->SPIDR = 0xAA;

        while(!GET_BIT_VALUE(SpiX->SPISR, spisr_rne_shift));

        pbuf[i] = SpiX->SPIDR;
    }
}


/**
 * @brief   SPI0 read write byte.
 * @param   SpiX: SPI0/SPI1.
 * @param   TxData: Need to send data.
 * @return uint8. */
uint8 spiReadWriteByte(SPI_TypeDef *SpiX, uint8 TxData)
{
    while(!isTransmitFifoNotFull(SpiX));

    spiWriteData(SpiX, TxData);

    while(!isReceiveFifoNotEmpty(SpiX));

    return spiReadData(SpiX);
}


/**
 * @brief   Get the Frame Format setting from SPICR0 register for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current Frame Format setting.
 */
uint8 getSpiFrameFormat(SPI_TypeDef *SpiX)
{
    return GET_BITS_VALUE(SpiX->SPICR0, spicr0_frf_shift, spicr0_frf_mask);
}

/**
 * @brief   Set the Frame Format in SPICR0 register for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   format: Frame format to set.
 * @note    Configures the format of data frames for SPI communication.
 */
void setSpiFrameFormat(SPI_TypeDef *SpiX, uint8 format)
{
    SET_VALUE(SpiX->SPICR0, spicr0_frf_shift, spicr0_frf_mask, format);
}

/**
 * @brief   Get the Data Size setting from SPICR0 register for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current Data Size setting.
 */
uint8 getSpiDataSize(SPI_TypeDef *SpiX)
{
    return GET_BITS_VALUE(SpiX->SPICR0, spicr0_dss_shift, spicr0_dss_mask);
}

/**
 * @brief   Set the Data Size in SPICR0 register for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   size: Data size to set, see also @ref spiDataSizeTypedef.
 */
void setSpiDataSize(SPI_TypeDef *SpiX, spiDataSizeTypedef size)
{
    SET_VALUE(SpiX->SPICR0, spicr0_dss_shift, spicr0_dss_mask, size);
}

/**
 * @brief   Set the Loopback mode in the SPICR1 register for the specified SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   lbm: Loopback mode setting (1 to enable).
 */
void setSpiSpiLoopBackMode(SPI_TypeDef *SpiX, uint8 lbm)
{
    if(lbm)
    {
        SET_BIT(SpiX->SPICR1, spicr1_lbm_bit);
    }
    else
    {
        CLEAR_BIT(SpiX->SPICR1, spicr1_lbm_bit);
    }
}

/**
 * @brief   Get the Loopback mode setting from the SPICR1 register for the specified SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current state of the Loopback mode (1 if enabled).
 */
uint8 getSpiSpiLoopBackMode(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPICR1, spicr1_lbm_shift);
}

/**
 * @brief   Enable or disable the Synchronous serial port in the SPICR1 register.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   sse: Synchronous Serial Port Enable setting (1 to enable).
 */
void setSpiSsp(SPI_TypeDef *SpiX, uint8 ssp)
{
    if(ssp)
    {
        SET_BIT(SpiX->SPICR1, spicr1_sse_bit);
    }
    else
    {
        CLEAR_BIT(SpiX->SPICR1, spicr1_sse_bit);
    }
}

/**
 * @brief   Get the current state of the Synchronous serial port from the SPICR1 register.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current state of the SPI operation (1 if enabled).
 */
uint8 getSpiSsp(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPICR1, spicr1_sse_shift);
}

/**
 * @brief   Set the SPI module mode (Master/Slave) in the SPICR1 register.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   mode: SPI mode setting (1 : Slave mode ,0 : Master mode ), see also @ref spiModeTypedef.
 */
void setSpiMode(SPI_TypeDef *SpiX, spiModeTypedef mode)
{
    if(mode)
    {
        SET_BIT(SpiX->SPICR1, spicr1_ms_bit);
    }
    else
    {
        CLEAR_BIT(SpiX->SPICR1, spicr1_ms_bit);
    }
}

/**
 * @brief   Get the current SPI module mode (Master/Slave) from the SPICR1 register.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current SPI mode setting (1 : Slave mode ,0 : Master mode ).
 */
uint8 getSpiSpiMode(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPICR1, spicr1_ms_shift);
}

/**
 * @brief   Set the Slave Output Disable state in the SPICR1 register for the specified SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   sod: Slave Output Disable setting (1 to disable).
 */
void setSpiSod(SPI_TypeDef *SpiX, uint8 sod)
{
    if(sod)
    {
        SET_BIT(SpiX->SPICR1, spicr1_sod_bit);
    }
    else
    {
        CLEAR_BIT(SpiX->SPICR1, spicr1_sod_bit);
    }
}

/**
 * @brief   Get the current Slave Output Disable state from the SPICR1 register for the specified SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current state of the Slave Output Disable setting (1 if disabled).
 */
uint8 getSpiSod(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPICR1, spicr1_sod_shift);
}
/**
 * @brief   Write data to the SPI Data Register (SPIDR).
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   data: Data to be written to the SPI Data Register.
 */
void spiWriteData(SPI_TypeDef *SpiX, uint8 data)
{
    SpiX->SPIDR = data;
}

/**
 * @brief   Read data from the SPI Data Register (SPIDR).
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Data read from the SPI Data Register.
 */
uint8 spiReadData(SPI_TypeDef *SpiX)
{
    return SpiX->SPIDR;
}
/**
 * @brief   Check if the transmit FIFO of the SPI is empty.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Status of the transmit FIFO (1 is empty, 0 is not empty).
 */
uint8 isTransmitFifoEmpty(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPISR, spisr_tfe_shift);
}

/**
 * @brief   Check if the transmit FIFO of the SPI is not full.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Status of the transmit FIFO (1 is not full, 0 is full).
 */
uint8 isTransmitFifoNotFull(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPISR, spisr_tnf_shift);
}

/**
 * @brief   Check if the receive FIFO of the SPI is not empty.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Status of the receive FIFO (1 is not empty, 0 is empty).
 */
uint8 isReceiveFifoNotEmpty(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPISR, spisr_rne_shift);
}

/**
 * @brief   Check if the receive FIFO of the SPI is full.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Status of the receive FIFO (1 is full, 0 is not full).
 * @note    Used to determine if the receive FIFO buffer is full.
 */
uint8 isReceiveFifoFull(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPISR, spisr_rff_shift);
}

/**
 * @brief   Check if the SPI is currently busy in communication.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  SPI busy status (1 if busy, 0 if idle).
 */
uint8 isSpiBusy(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPISR, spisr_bsy_shift);
}

/**
 * @brief   Set the clock prescaler for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   prescaler: The prescaler value to set (must be even, between 2 and 254 inclusive).
 */
void setSpiClockPrescaler(SPI_TypeDef *SpiX, uint16 prescaler)
{
    if(prescaler < 2 || prescaler > 254 || (prescaler % 2) != 0)
    {
        return;
    }
    SpiX->SPICPSR = (SpiX->SPICPSR & ~0xFF) | (prescaler & 0xFF);
}

/**
 * @brief   Get the current clock prescaler setting of the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current value of the clock prescaler.
 */
uint16 getSpiClockPrescaler(SPI_TypeDef *SpiX)
{
    return SpiX->SPICPSR & 0xFF;
}

/**
 * @brief   set the Spi interrupt status.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return
 */
void enableSpiInterrupt(SPI_TypeDef *SpiX, uint8 tmpreg)
{
    MODIFY_REG(SpiX->SPIIMSC, (SPI_INTSTATUS_ROR | SPI_INTSTATUS_RT |  \
                               SPI_INTSTATUS_RX | SPI_INTSTATUS_TX), tmpreg);
}

/**
 * @brief   clear the Spi interrupt status..
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return
 */
void disableSpiInterrupt(SPI_TypeDef *SpiX)
{
    CLEAR_VALUE(SpiX->SPIIMSC);
}

/**
 * @brief   Get the Raw interrupt status.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Status of the Raw interrupt status.
 */
uint16 getSpiRawInterruptState(SPI_TypeDef *SpiX)
{
    return SpiX->SPIRIS;
}

/**
 * @brief   Get the Masked interrupt status.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Status of the Masked interrupt status.
 */
uint16 getSpiMaskedInterruptState(SPI_TypeDef *SpiX)
{
    return SpiX->SPIMIS;
}


/**
 * @brief   Clear the Receive FIFO Overrun and Receive Timeout Interrupt.
 * @param   SpiX: SPi number: SPI0,SPI.
 */
void clearSpiInterrupt(SPI_TypeDef *SpiX)
{
    SET_BIT(SpiX->SPIICR, SPI_INTSTATUS_CLEAR);
}


/**
 * @brief   Enable or disable the Receive DMA for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   enable: Set to a 1 value to enable Receive DMA, or 0 to disable.
 */
void setSpiReceiveDmaEnable(SPI_TypeDef *SpiX, uint8 enable)
{
    if(enable)
    {
        SET_BIT(SpiX->SPIDMACR, spidmacr_rxdmae_bit);
    }
    else
    {
        CLEAR_BIT(SpiX->SPIDMACR, spidmacr_rxdmae_bit);
    }
}

/**
 * @brief   Get the current status of the Receive DMA enable setting for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current status of the Receive DMA enable setting (1 if enabled).
 */
uint8 getSpiReceiveDmaEnable(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPIDMACR, spidmacr_rxdmae_shift);
}

/**
 * @brief   Enable or disable the Transmit DMA for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   enable: Set to a 1 value to enable Transmit DMA, or 0 to disable.
 */
void setSpiTransmitDmaEnable(SPI_TypeDef *SpiX, uint8 enable)
{
    if(enable)
    {
        SET_BIT(SpiX->SPIDMACR, spidmacr_txdmae_bit);
    }
    else
    {
        CLEAR_BIT(SpiX->SPIDMACR, spidmacr_txdmae_bit);
    }
}

/**
 * @brief   Get the current status of the Transmit DMA enable setting for the SPI module.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @return  Current status of the Transmit DMA enable setting (1 if enabled).
 */
uint8 getSpiTransmitDmaEnable(SPI_TypeDef *SpiX)
{
    return GET_BIT_VALUE(SpiX->SPIDMACR, spidmacr_txdmae_shift);
}

/**
 * @brief   Set the spi interrupt handler function.
 * @param   SpiX: SPi number: SPI0,SPI.
 * @param   handler: the address of the spi interrupt callback function.
 */
void setSpiIntHandler(SPI_TypeDef *SpiX, const SPI_HANDLER handler)
{

    if(NULL != handler)
    {
        if(SPI0 == SpiX)
        {
            s_spiHandler[0] = handler;
        }
        else if(SPI1 == SpiX)
        {
            s_spiHandler[1] = handler;
        }
    }
}

/* Handler functions ---------------------------------------------------------*/
/**
 * @brief   SPI0 interrupt handler.
 */
void spi0Handler(void)
{
    uint8 state;

    state = getSpiMaskedInterruptState(SPI0);
    clearSpiInterrupt(SPI0);

    s_spiHandler[0](state);
}


/**
 * @brief   SPI1 interrupt handler.
 */
void spi1Handler(void)
{
    uint8 state;

    state = getSpiMaskedInterruptState(SPI0);
    clearSpiInterrupt(SPI1);

    s_spiHandler[1](state);
}
