/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#include "app_spi_stream.h"

#include "project_config.h"
#include "adt6101P_type_define.h"
#include "adt6101P_dma.h"
#include "dma_bsp.h"
#include "adt6101P_spi.h"
#include "adt6101P_system.h"
#include "gpio_bsp.h"

#define APP_SPI_STREAM_DMA          ((dmaSelTypedef)PROJECT_SPI_STREAM_DMA_CHANNEL)
#define APP_SPI_STREAM_WORDS        ((uint32_t)(PROJECT_SPI_STREAM_BYTES / 2UL))
#define APP_SPI_STREAM_WAIT_LOOPS   (2000000UL)

static volatile uint8 s_streamPrepared;
static volatile uint8 s_streamActive;
static volatile uint8 s_streamQueued;
static uint32_t s_streamAddress;

static void app_spi_stream_cs(uint8 active)
{
    /* P0/TX0 is the new active-low stream chip select. */
    gpioWritePin(GPIO_PIN_0, (active != 0u) ? GPIO_PIN_RESET : GPIO_PIN_SET);
}

static void app_spi_stream_drain_rx(void)
{
    while(isReceiveFifoNotEmpty(SPI0) != 0u)
    {
        (void)spiReadData(SPI0);
    }
    clearSpiInterrupt(SPI0);
}

void AppSpiStream_Init(void)
{
    gpioInitStrTypedef gpioConfig;

    /* UART0 init muxes P0 as TX even in RX-only mode; explicitly reclaim it. */
    gpioConfig.Pin = GPIO_PIN_0 | GPIO_PIN_4;
    gpioConfig.Mode = GPIO_MODE_OUTPUT;
    gpioConfig.Pull = GPIO_PULL_UP;
    gpioInit(&gpioConfig);

    /* Both chip selects idle high. P4 keeps the board SPI Flash permanently deselected. */
    gpioWritePin(GPIO_PIN_0, GPIO_PIN_SET);
    gpioWritePin(GPIO_PIN_4, GPIO_PIN_SET);

    setSpiTransmitDmaEnable(SPI0, 0u);
    setSpiReceiveDmaEnable(SPI0, 0u);
    disableSpiInterrupt(SPI0);
    app_spi_stream_drain_rx();

    setDmaIntHandler(APP_SPI_STREAM_DMA, INT_HANLDER_1);
    dmaIntConfig(APP_SPI_STREAM_DMA, DMA_4_4_DONE, DISABLE);
    setDmaDisable(APP_SPI_STREAM_DMA);
    setIntHandlerEn(INT_DMAB, ENABLEINT);

    s_streamPrepared = 0u;
    s_streamActive = 0u;
    s_streamQueued = 0u;
}

uint8_t AppSpiStream_Prepare(uint32_t sourceAddr)
{
    if((s_streamPrepared != 0u) ||
       (s_streamActive != 0u) ||
       ((sourceAddr & 1UL) != 0UL))
    {
        return 0u;
    }

    setSpiTransmitDmaEnable(SPI0, 0u);
    setDmaDisable(APP_SPI_STREAM_DMA);
    dmaIntConfig(APP_SPI_STREAM_DMA, DMA_4_4_DONE, DISABLE);
    app_spi_stream_drain_rx();

    s_streamAddress = sourceAddr;
    s_streamQueued = 0u;
    s_streamPrepared = 1u;
    return 1u;
}

static uint8_t app_spi_stream_write_words(uint32_t sourceAddr, uint32_t wordCount)
{
    volatile uint16_t *words = (volatile uint16_t *)(uintptr_t)sourceAddr;
    uint32_t i;
    uint32 timeoutLoops;

    for(i = 0u; i < wordCount; i++)
    {
        timeoutLoops = APP_SPI_STREAM_WAIT_LOOPS;
        while((isTransmitFifoNotFull(SPI0) == 0u) && (timeoutLoops > 0u))
        {
            timeoutLoops--;
        }
        if(timeoutLoops == 0u)
        {
            return 0u;
        }
        SPI0->SPIDR = words[i];
    }

    return 1u;
}

uint8_t AppSpiStream_Trigger(void)
{
    if((s_streamPrepared == 0u) || (s_streamActive != 0u))
    {
        return 0u;
    }

    app_spi_stream_cs(1u);
    s_streamPrepared = 0u;
    s_streamActive = 1u;
    s_streamQueued = 0u;

    if(app_spi_stream_write_words(s_streamAddress, APP_SPI_STREAM_WORDS) == 0u)
    {
        app_spi_stream_cs(0u);
        s_streamActive = 0u;
        return 0u;
    }

    s_streamQueued = 1u;
    return 1u;
}

void AppSpiStream_DmaCompleteIsr(void)
{
    /* CPU-fed stream mode does not use DMA completion. */
}

uint8_t AppSpiStream_Wait(void)
{
    uint32 timeoutLoops = APP_SPI_STREAM_WAIT_LOOPS;

    if(s_streamActive == 0u)
    {
        return 0u;
    }

    setSpiTransmitDmaEnable(SPI0, 0u);
    while((isSpiBusy(SPI0) != 0u) && (timeoutLoops > 0u))
    {
        timeoutLoops--;
    }

    app_spi_stream_cs(0u);
    app_spi_stream_drain_rx();
    s_streamActive = 0u;

    return (uint8)((s_streamQueued != 0u) && (timeoutLoops > 0u));
}

void AppSpiStream_Abort(void)
{
    setSpiTransmitDmaEnable(SPI0, 0u);
    dmaIntConfig(APP_SPI_STREAM_DMA, DMA_4_4_DONE, DISABLE);
    setDmaDisable(APP_SPI_STREAM_DMA);
    app_spi_stream_cs(0u);
    s_streamPrepared = 0u;
    s_streamActive = 0u;
    s_streamQueued = 0u;
}
