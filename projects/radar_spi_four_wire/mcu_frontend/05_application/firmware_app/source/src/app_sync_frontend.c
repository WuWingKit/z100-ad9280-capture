/* ADT6101P SPI-only mailbox: FPGA master and MCU slave.
 * SPI-only slave: P3=SCK, P4=/CS, P5=MISO, P6=MOSI. No TX/RX handshake.
 * P4 is slave /CS: board Flash isolation MUST be verified before use.
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
static volatile uintptr_t published_address;
static volatile uint32_t published_bytes;
static volatile uint16_t published_seq;
static volatile uint8_t trigger_pending;
static uint16_t request_words[8], rx_index, reply_words[40], reply_read, reply_count;
static volatile uint16_t link_fault;
static uint16_t last_trigger_id, have_trigger_id;

/* CRC16/CCITT-FALSE over each word high byte first. */
static uint16_t crc_word(uint16_t crc,uint16_t v) {
    unsigned i;for(i=0;i<16;i++){unsigned bit=((crc>>15)^((v>>(15-i))&1u))&1u;
    crc=(uint16_t)(crc<<1);if(bit)crc^=0x1021u;}return crc;
}
static uint16_t flags(void) {
    return (uint16_t)((g_radarSync.busy||g_radarSync.pending||trigger_pending?1u:0u)|
        (published_bytes?2u:0u)|(g_radarSync.fault||link_fault?4u:0u));
}
static int port_send(void *context, uintptr_t address, uint32_t bytes) {
    (void)context;
    /* Lease the old bank until ACK. No GPIO/SPI-master operation here. */
    if(published_bytes) return 0;
    published_address=address;published_seq=*(volatile uint16_t *)(address+28u);
    published_bytes=bytes;return 1;
}
static void mailbox_request(void) {
    uint16_t crc=0xffffu,op=request_words[1],id=request_words[2];
    uint16_t len=0,error=0,i;
    uint32_t off=(uint32_t)request_words[3]|((uint32_t)request_words[4]<<16);
    for(i=0;i<7;i++)crc=crc_word(crc,request_words[i]);
    if(crc!=request_words[7])return;
    if(reply_read!=reply_count){link_fault=1;return;}
    if(op==2u){
        if(have_trigger_id&&id==last_trigger_id){} /* Retry is idempotent. */
        else if(flags()&7u)error=1;
        else {trigger_pending=1;last_trigger_id=id;have_trigger_id=1;}
    } else if(op==3u){
        len=request_words[5];
        if(!published_bytes||len>64u||off>published_bytes||len>published_bytes-off){error=1;len=0;}
    } else if(op==4u){
        if(published_bytes&&off==published_seq)published_bytes=0;
        else error=1;
    } else if(op!=1u)error=1;
    reply_words[0]=0x5ad0u;reply_words[1]=op;reply_words[2]=id;
    reply_words[3]=(uint16_t)(flags()|(error?0x8000u:0));reply_words[4]=len;
    reply_words[5]=(uint16_t)off;reply_words[6]=(uint16_t)(off>>16);
    for(i=0;i<(len+1u)/2u;i++){
        uintptr_t a=published_address+off+2u*i;
        uint16_t v=(uint16_t)*(volatile uint8_t *)a<<8;
        if(2u*i+1u<len)v|=*(volatile uint8_t *)(a+1u);
        reply_words[7u+i]=v;
    }
    crc=0xffffu;for(i=0;i<7u+(len+1u)/2u;i++)crc=crc_word(crc,reply_words[i]);
    reply_words[i]=crc;reply_read=0;reply_count=i+1u;
}
static void mailbox_isr(const uint8 mode) {
    uint16_t w;
    if(mode&SPI_INTSTATUS_ROR)link_fault=1;
    while(isReceiveFifoNotEmpty(SPI0)){
        w=(uint16_t)SPI0->SPIDR;
        if(rx_index==0u){if(w==0xd5a0u)request_words[rx_index++]=w;}
        else {request_words[rx_index++]=w;if(rx_index==8u){rx_index=0;mailbox_request();}}
    }
    while(isTransmitFifoNotFull(SPI0)){
        SPI0->SPIDR=reply_read<reply_count?reply_words[reply_read++]:0xffffu;
    }
}
static void port_write(void *context, uintptr_t address, uint16_t value)
{ (void)context; *(volatile uint16_t *)address = value; }

uint8_t AppSyncFrontend_Init(void)
{
    gpioInitStrTypedef pins;
    SpiInitStrTypedef spi;
    RadarSyncPorts ports;

    /* Only SPI0 pins P3/P4/P5/P6 are used for board communication.
     * HARDWARE GATE: isolate boot Flash /CS and MISO before enabling this image.
     * Initial UART pins are released here and never used as handshake GPIOs.
     */
    setIntHandlerEn(INT_GPIO, DISABLEINT);
    pins.Pin=GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_10|GPIO_PIN_12;
    pins.Mode=GPIO_MODE_HIGH_INPENDECE;pins.Pull=GPIO_NO_PULL;gpioInit(&pins);
    setSpiSsp(SPI0,0u);
    spi.SpiDataSize=SPI_DATASIZE_16BIT;spi.SpiPrescaler=SPI_Prescaler_4;
    spi.SpiGaterMode=SPI_CPOL_Low_CPHA_1Edge;spi.SpiMode=SPI_slave;
    spi.SpiNSS=SPI_NSS_Hard;
    spi.intEn=SPI_INTSTATUS_RX|SPI_INTSTATUS_RT|SPI_INTSTATUS_ROR;
    spi.intHandler=mailbox_isr;spiInit(SPI0,&spi);
    setSpiSod(SPI0,0u); /* Enable slave MISO output when selected. */
    while(isReceiveFifoNotEmpty(SPI0))(void)SPI0->SPIDR;
    while(isTransmitFifoNotFull(SPI0))SPI0->SPIDR=0xffffu;
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

    return 1u;
}
void AppSyncFrontend_GpioIsr(void) { /* No SYNC GPIO in SPI-only mode. */ }
void AppSyncFrontend_Process(void) {
    int result;uint32_t saved=port_enter(0);
    if(trigger_pending){trigger_pending=0;RadarSync_OnRisingEdge(&g_radarSync);}
    port_leave(0,saved);
    result=RadarSync_Process(&g_radarSync);
    if(result){g_radarLastResult=result;if(result>0)g_radarCompletedEvents++;}
}
