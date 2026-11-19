/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#include "adt6101P_system.h"
#include "adt6101P_init.h"
#include "adt6101P.h"
#include "app_main_tasks.h"
#include "app_sync_frontend.h"
#include "app_hardware_ports.h"
#include "gpio_bsp.h"
#include "app_timebase.h"
#include "adt2011_driver.h"
#include "adt2011_gpio_bus.h"
#include "project_config.h"
#include "lowpower_bsp.h"
#include "tx_bsp.h"
#include "signal_porcess.h"

#if !RADAR_SYNC_SELFTEST
static Adt2011BusTypedef s_adt2011Bus;
#endif
static Adt2011DriverTypedef s_adt2011;

static uint8_t app_hw_prepare_beam(void *context,
                                   const uint8_t txPhase[16],
                                   const uint8_t rxPhase[16])
{
    Adt2011DriverTypedef *driver = (Adt2011DriverTypedef *)context;

    if(Adt2011Driver_WriteBeamConfig(driver, ADT2011_CHIP_TX, txPhase) != ADT2011_STATUS_OK)
    {
        return 0u;
    }
    return (uint8_t)(Adt2011Driver_WriteBeamConfig(driver, ADT2011_CHIP_RX, rxPhase) ==
                     ADT2011_STATUS_OK);
}

static uint8_t app_hw_load_beam(void *context)
{
    Adt2011DriverTypedef *driver = (Adt2011DriverTypedef *)context;

    if(Adt2011Driver_LoadBeam(driver, ADT2011_CHIP_TX) != ADT2011_STATUS_OK)
    {
        return 0u;
    }
    return (uint8_t)(Adt2011Driver_LoadBeam(driver, ADT2011_CHIP_RX) == ADT2011_STATUS_OK);
}

static uint8_t app_hw_set_beam(void *context,
                               const uint8_t txPhase[16],
                               const uint8_t rxPhase[16])
{
    if(app_hw_prepare_beam(context, txPhase, rxPhase) == 0u)
    {
        return 0u;
    }
    return app_hw_load_beam(context);
}

static uint8_t app_hw_capture_start(void *context, uint32_t adcDestinationAddr)
{
    Adt2011DriverTypedef *driver = (Adt2011DriverTypedef *)context;

    if((adcDestinationAddr != PROJECT_ADC_BUFFER_A_ADDR) &&
       (adcDestinationAddr != PROJECT_ADC_BUFFER_B_ADDR))
    {
        return 0u;
    }

    g_MMWMsgMCB.adcDspStr.adc0DstAddr = adcDestinationAddr;
    lowPowerActiveGotoRadio(FRAME_LOWPOWER, LOWPOWER_SLOW,
                            &g_MMWMsgMCB.rfStr, SYS_HIGH_CLK_100M);
    if(Adt2011Driver_SetRfEnabled(driver, 1u) != ADT2011_STATUS_OK)
    {
        lowPowerRadioGotoActive(FRAME_LOWPOWER, LOWPOWER_SLOW,
                                &g_MMWMsgMCB.rfStr, SYS_HIGH_CLK_100M);
        return 0u;
    }

    if(adcSampleStartAsync(&g_MMWMsgMCB.adcDspStr) == 0u)
    {
        (void)Adt2011Driver_SetRfEnabled(driver, 0u);
        lowPowerRadioGotoActive(FRAME_LOWPOWER, LOWPOWER_SLOW,
                                &g_MMWMsgMCB.rfStr, SYS_HIGH_CLK_100M);
        return 0u;
    }
    return 1u;
}

static uint8_t app_hw_capture_wait(void *context)
{
    uint8_t adcOk;
    Adt2011DriverTypedef *driver = (Adt2011DriverTypedef *)context;

    /*
     * f1_0 front-end timing rule:
     *   RF on + ADC DMA -> 100 us -> TX PSR -> RX PSR -> poll ADC DMA done
     *   -> RF off -> TX load -> RX load.
     *
     * RF shutdown is deliberately performed here in thread/main-loop context.
     * It must never be issued from the ADC DMA interrupt, because the ADT2011
     * register writes are blocking GPIO transactions and can starve UART RX.
     */
    adcOk = adcSampleWait();
    /* P0 now belongs exclusively to SPI CS; P1 is an input. */
    if(adcOk == 0u)
    {
        /* Best-effort shutdown on timeout/error before returning failure. */
        if(driver->rfEnabled != 0u)
        {
            (void)Adt2011Driver_SetRfEnabled(driver, 0u);
        }
        lowPowerRadioGotoActive(FRAME_LOWPOWER, LOWPOWER_SLOW,
                                &g_MMWMsgMCB.rfStr, SYS_HIGH_CLK_100M);
        return 0u;
    }

    if((driver->rfEnabled != 0u) &&
       (Adt2011Driver_SetRfEnabled(driver, 0u) != ADT2011_STATUS_OK))
    {
        lowPowerRadioGotoActive(FRAME_LOWPOWER, LOWPOWER_SLOW,
                                &g_MMWMsgMCB.rfStr, SYS_HIGH_CLK_100M);
        return 0u;
    }

    lowPowerRadioGotoActive(FRAME_LOWPOWER, LOWPOWER_SLOW,
                            &g_MMWMsgMCB.rfStr, SYS_HIGH_CLK_100M);
    return 1u;
}

static uint8_t app_hw_configure_bandwidth(void *context, uint16_t bandwidthMHz)
{
    (void)context;
    lowPowerActiveGotoRadio(FRAME_LOWPOWER, LOWPOWER_SLOW,
                            &g_MMWMsgMCB.rfStr, SYS_HIGH_CLK_100M);
    pllLock(FSTART, bandwidthMHz, g_MMWMsgMCB.adcDspStr.scanNum);
    fmcwConfig(FSTART, bandwidthMHz, CHIRP_T0, CHIRP_T1, CHIRP_T2,
               g_MMWMsgMCB.adcDspStr.scanNum);
    lowPowerRadioGotoActive(FRAME_LOWPOWER, LOWPOWER_SLOW,
                            &g_MMWMsgMCB.rfStr, SYS_HIGH_CLK_100M);
    return 1u;
}

static void app_hw_wait_us(void *context, uint32_t microseconds)
{
    (void)context;
    delayUs(microseconds);
}

static const AppHardwarePortsTypedef s_hardwarePorts =
{
    &s_adt2011,
    app_hw_set_beam,
    app_hw_prepare_beam,
    app_hw_load_beam,
    app_hw_capture_start,
    app_hw_capture_wait,
    app_hw_configure_bandwidth,
    app_hw_wait_us
};

static void app_fatal_stop(void)
{
    /* No TX/RX handshake outputs in SPI-only mode. */
    setIntHandlerEn(INT_GPIO, DISABLEINT);
    while(1)
    {
    }
}

int main(void)
{
    MY_NVIC_SetVectorTable(0x00000000u, 0x8000u);

    /* Bind parser storage before UART RX interrupts are enabled. */
    if(AppMainProtocol_Init(PROJECT_PROTOCOL_HEADER0, PROJECT_PROTOCOL_HEADER1) == 0u)
    {
        app_fatal_stop();
    }

    adt6101PSystemInit();
    adt6101PValInit();
    adt6101PModuleInit();

    if(AppHardwarePorts_Bind(&s_hardwarePorts) == 0u)
    {
        app_fatal_stop();
    }

    AppTimebase_InitTimer1();
    /* Start with no RF acquisition; wait for the FPGA SYNC rising edge. */

#if !RADAR_SYNC_SELFTEST
    Adt2011GpioBus_Create(&s_adt2011Bus);
    if(Adt2011Driver_Initialize(&s_adt2011, &s_adt2011Bus) != ADT2011_STATUS_OK)
    {
        app_fatal_stop();
    }

#endif
    if(AppSyncFrontend_Init() == 0u)
    {
        app_fatal_stop();
    }

    while(1)
    {
        AppSyncFrontend_Process();
    }
}
