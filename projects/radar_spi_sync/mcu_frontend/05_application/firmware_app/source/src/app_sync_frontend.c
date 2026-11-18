/* ADT6101P board binding for FPGA-generated SYNC and MCU-master SPI.
 * GPIO: P1=SYNC input, P12=READY output, P0=CS output, P3=SCLK, P6=MOSI.
 * P4 keeps external boot Flash deselected; P19/SWD pins are not changed.
 * English ASCII source comments avoid legacy Keil/Vivado encoding problems;
 * the project documentation contains the corresponding Chinese explanation.
 */
#include "app_sync_frontend.h"
#include "project_config.h"
#include "scan.h"
#include "app_spi_stream.h"
#include "gpio_bsp.h"
#include "spi_bsp.h"
#include "timer_bsp.h"
#include "sys_bsp.h"
#include "adt6101P_system.h"
#include "adt6101P_inthandler.h"
#include "CMSDK_CM3.h"

#define SYNC_PIN GPIO_PIN_1
#define READY_PIN GPIO_PIN_12

RadarSyncPipeline g_radarSync;
volatile int g_radarLastResult;
volatile uint32_t g_radarCompletedEvents;

/* ----- Critical section: transfer ISR events to the main loop atomically. */
static uint32_t port_enter(void *context)
{
    uint32_t saved = __get_PRIMASK();
    (void)context;
    __disable_irq();
    return saved;
}
static void port_leave(void *context, uint32_t saved)
{
    (void)context;
    __set_PRIMASK(saved);
}

/* ----- Existing radar functions retain their RF/ADC/beam timing. */
static int port_initial_beam(void *context, uint16_t beam)
{
    (void)context;
#if RADAR_SYNC_SELFTEST
    (void)beam; return 1;
#else
    return adt_scan_set_beam((uint8_t)beam) != 0u;
#endif
}
static int port_capture(void *context, uintptr_t address)
{
    (void)context;
#if RADAR_SYNC_SELFTEST
    /* Link-only test: preserve frame size, chirp discard and SPI transport.
     * I carries the sample index; Q tags the capture sequence. No RF enable.
     */
    uint32_t i, seq = g_radarSync.next_seq - 1u;
    volatile uint32_t *data = (volatile uint32_t *)address;
    for(i=0;i<PROJECT_ADC_FRAME_BYTES/4u;i++)
        data[i]=((uint32_t)(uint16_t)(0x8000u|seq)<<16)|(i&0x7fffu);
    return 1;
#else
    return adt_scan_acquire_start((uint32_t)address) != 0u;
#endif
}
static int port_finish(void *context)
{
    (void)context;
#if RADAR_SYNC_SELFTEST
    return 1;
#else
    return adt_scan_acquire_wait() != 0u;
#endif
}
static void port_delay(void *context, uint32_t us)
{ (void)context; delayUs(us); }
static int port_prepare(void *context, uint16_t beam)
{
    (void)context;
#if RADAR_SYNC_SELFTEST
    (void)beam; return 1;
#else
    return adt_scan_prepare_beam((uint8_t)beam) != 0u;
#endif
}
static int port_load(void *context)
{
    (void)context;
#if RADAR_SYNC_SELFTEST
    return 1;
#else
    return adt_scan_load_beam() != 0u;
#endif
}

/* ----- One complete header+payload transaction, CS low across the frame.
 * The CPU feeds the SPI TX FIFO. This is NOT an SPI DMA implementation.
 * Wait() must observe the hardware busy flag before releasing the old bank.
 */
static int port_send(void *context, uintptr_t address, uint32_t bytes)
{
    (void)context;
    if(bytes != PROJECT_SPI_STREAM_BYTES) return 0;
    if(!AppSpiStream_Prepare((uint32_t)address) ||
       !AppSpiStream_Trigger() || !AppSpiStream_Wait())
    {
        AppSpiStream_Abort();
        return 0;
    }
    return 1;
}
static void port_write(void *context, uintptr_t address, uint16_t value)
{ (void)context; *(volatile uint16_t *)address = value; }

uint8_t AppSyncFrontend_Init(void)
{
    gpioInitStrTypedef pins;
    SpiInitStrTypedef spi;
    RadarSyncPorts ports;

    /* Keep READY low until all hardware and protocol state is initialized. */
    setIntHandlerEn(INT_GPIO, DISABLEINT);
    setGpioInterruptEnable(SYNC_PIN, DISABLE);
    gpioWritePin(READY_PIN, GPIO_PIN_RESET);
    pins.Pin = READY_PIN; pins.Mode = GPIO_MODE_OUTPUT; pins.Pull = GPIO_PULL_DOWN;
    gpioInit(&pins);
    if(getPclk()!=SYS_HIGH_CLK_50M) return 0u;
    setSpiSsp(SPI0,0u);
    setSpiSCR(SPI0,0u); /* 50MHz / (CPSR=4 * (SCR+1)) = 12.5MHz. */

    /* Soft NSS keeps P4 high and lets AppSpiStream own P0 instead.
     * SPI's peripheral input clock in this board configuration is 50MHz;
     * CPSR /4 gives the reference project's 12.5MHz target.
     */
    spi.SpiDataSize = SPI_DATASIZE_16BIT;
    spi.SpiPrescaler = SPI_Prescaler_4;
    spi.SpiGaterMode = SPI_CPOL_Low_CPHA_1Edge;
    spi.SpiMode = SPI_Master;
    spi.SpiNSS = SPI_NSS_Soft;
    spi.intEn = 0;
    spi.intHandler = spi0InterruptHandler;
    spiInit(SPI0, &spi);
    disableSpiInterrupt(SPI0);
    setIntHandlerEn(INT_SPI0, DISABLEINT);
    AppSpiStream_Init();
    setGpioConfigPull(GPIO_CFG_P5, GPIO_PULL_UP); /* unused MISO */

    ports.context = 0;
    ports.enter_critical = port_enter; ports.leave_critical = port_leave;
    ports.set_initial_beam = port_initial_beam;
    ports.capture_start = port_capture; ports.capture_finish = port_finish;
    ports.wait_us = port_delay; ports.prepare_beam = port_prepare;
    ports.load_beam = port_load; ports.send_frame = port_send;
    ports.write_word = port_write;
    if(!RadarSync_Init(&g_radarSync, &ports, PROJECT_ADC_BUFFER_A_ADDR,
                       PROJECT_ADC_BUFFER_B_ADDR, PROJECT_SYNC_SCAN_ENABLE))
        return 0;

    /* P1 used to be UART0 RX / output marker. Reclaim it as GPIO input.
     * Remove the baseboard USB-UART RX0 jumper before wiring FPGA SYNC.
     */
    pins.Pin = SYNC_PIN; pins.Mode = GPIO_MODE_INPUT; pins.Pull = GPIO_PULL_DOWN;
    gpioInit(&pins);
    setGpioConfigSmt(GPIO_CFG_P1, SCHMITT_TRIGGER);
    setGpioInterruptTrigger(SYNC_PIN, EDGE_MODE_POSEDGE);
    clearGpioInterrupt(SYNC_PIN);
    NVIC_ClearPendingIRQ((IRQn_Type)INT_GPIO);
    setGpioInterruptEnable(SYNC_PIN, ENABLE);
    gpioWritePin(READY_PIN, GPIO_PIN_SET);
    setIntHandlerEn(INT_GPIO, ENABLEINT);
    return 1u;
}

/* Short ISR only: latch the event and drop READY before returning.
 * No ADC waits, phase-shifter transactions or SPI loops in interrupt context.
 */
void AppSyncFrontend_GpioIsr(void)
{
    uint32_t pending = getGpioInterruptState();
    if(pending & SYNC_PIN)
    {
        clearGpioInterrupt(SYNC_PIN);
        gpioWritePin(READY_PIN, GPIO_PIN_RESET);
        RadarSync_OnRisingEdge(&g_radarSync);
    }
    /* This application owns GPIO IRQ; clear other unexpected GPIO sources. */
    if(pending & ~SYNC_PIN) clearGpioInterrupt(pending & ~SYNC_PIN);
}

void AppSyncFrontend_Process(void)
{
    int result;
    uint32_t saved;
    result = RadarSync_Process(&g_radarSync);
    if(result != 0)
    {
        g_radarLastResult = result;
        if(result > 0) g_radarCompletedEvents++;
    }
    /* Do not reassert READY if a new ISR event arrived after Process returned.
     * Faults keep READY low until BOTH endpoints are explicitly restarted.
     */
    saved = port_enter(0);
    gpioWritePin(READY_PIN, (!g_radarSync.fault && !g_radarSync.busy &&
                 !g_radarSync.pending) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    port_leave(0, saved);
}
