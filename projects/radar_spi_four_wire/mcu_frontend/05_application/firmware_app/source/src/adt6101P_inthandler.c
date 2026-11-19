/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 中; basis recorded in AUTHORS.json.
 */
#include "adt6101P_inthandler.h"

#include "adt6101P.h"
#include "adt6101P_dma.h"
#include "adt6101P_gpio.h"
#include "adt6101P_timer.h"
#include "adt6101P_uart.h"
#include "adt6101P_pmu.h"
#include "adt6101P_rtc.h"
#include "adt6101P_trng.h"
#include "dma_ctype_map.h"
#include "gpio_ctype_map.h"
#include "trng_ctype_map.h"
#include "timer_bsp.h"
#include "app_main_tasks.h"
#include "app_spi_stream.h"
#include "app_timebase.h"
#include "app_sync_frontend.h"

extern volatile int32 g_delayUsFlag;

void NMI_Handler(void) {}

void uart0InterruptHandler(const uint16 mode)
{
    if((mode & UART_INTSTATUS_RX) != 0u)
    {
        if((getUartState(UART0) & UART_STATE_RX_BUFF_FULL) != 0u)
        {
            (void)AppProtocol_PushRxByte((uint8_t)getUartData(UART0));
        }
    }
}

void uart1InterruptHandler(const uint16 mode) { (void)mode; }
void uart2InterruptHandler(const uint16 mode) { (void)mode; }

void timer0InterruptHandler(void)
{
    uint32 reload = getReload(FRAME_CONCTRL);
    timerInitStrTypedef timer0;
    timer0.Timerx = Timer0;
    timer0.mode = DEFUALT_MODE;
    timer0.reload = reload;
    timer0.intHandler = timer0InterruptHandler;
    timerInit(&timer0);
}

void timer1InterruptHandler(void)
{
    AppTimebase_OnTimer1Interrupt();
}

void dualtimer1InterruptHandler(void) { g_delayUsFlag = 1; }
void dualtimer2InterruptHandler(void) { g_delayUsFlag = 1; }
void spi0InterruptHandler(const uint8 mode) { (void)mode; }
void spi1InterruptHandler(const uint8 mode) { (void)mode; }

void pmuHandler(void)
{
    PMU->pmu_int_clear_reg = 0x1f;
}

void rtcHandler(void)
{
    RTC->RTCICR = 0x01;
    RTC->RTCLR = 0u;
}

void gpioHandler(void)
{
    AppSyncFrontend_GpioIsr();
}

void dmaAHandler(void)
{
    /*
     * ADC DMA completion is polled in adcSampleWait().  Keep this handler as a
     * defensive status clear only; it must never perform ADT2011 register I/O.
     */
    const uint32 dmaAStatus = DMA->dmair_dma_int_status & 0x00007FFFUL;
    DMA->dmair_dma_int_clear = dmaAStatus;
}

void dmaBHandler(void)
{
    const uint32 dmaBStatus = DMA->dmair_dma_int_status & 0x01FF8000UL;
    DMA->dmair_dma_int_clear = dmaBStatus;
    AppSpiStream_DmaCompleteIsr();
}

void trngHandler(void)
{
    TRNG->RNG_ICR = 1u;
}
