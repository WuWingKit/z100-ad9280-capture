/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#include "app_main_tasks.h"

#include "app_protocol.h"
#include "app_radar_config.h"
#include "app_hardware_ports.h"
#include "gpio_bsp.h"
#include "app_timebase.h"
#include "scan.h"

#define APP_PROTOCOL_RX_RING_LEN       (32u)
#define APP_PROTOCOL_MAX_PAYLOAD_LEN   (2u)

static AppProtocolTypedef s_commandProtocol;
static AppProtocolConfigTypedef s_commandProtocolConfig;
static volatile uint8_t s_commandRxRing[APP_PROTOCOL_RX_RING_LEN];
static uint8_t s_commandPayload[APP_PROTOCOL_MAX_PAYLOAD_LEN];

static uint16_t s_bandwidthMHz = APP_FMCW_BANDWIDTH_DEFAULT_MHZ;

static uint16_t app_get_u16_le(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void app_protocol_on_frame(AppProtocolTypedef *protocol,
                                  uint8_t command,
                                  const uint8_t *payload,
                                  uint16_t payloadLength,
                                  void *context)
{
    uint16_t requestedValue;

    (void)protocol;
    (void)context;

    if((payload == (const uint8_t *)0) || (payloadLength != 2u))
    {
        return;
    }

    requestedValue = app_get_u16_le(payload);
    if(command == APP_PROTOCOL_CMD_FMCW_BANDWIDTH)
    {
        if((requestedValue < APP_FMCW_BANDWIDTH_MIN_MHZ) ||
           (requestedValue > APP_FMCW_BANDWIDTH_MAX_MHZ) ||
           (requestedValue == s_bandwidthMHz))
        {
            return;
        }

        if(AppHardware_ConfigureBandwidth(requestedValue) != 0u)
        {
            s_bandwidthMHz = requestedValue;
        }
        return;
    }

    if(command == APP_PROTOCOL_CMD_RUN_INTERVAL_MS)
    {
        if((requestedValue < APP_RUN_INTERVAL_MIN_MS) ||
           (requestedValue > APP_RUN_INTERVAL_MAX_MS))
        {
            return;
        }
        /* Timing is owned by FPGA SYNC; legacy interval command is ignored. */
    }
}

uint8_t AppMainProtocol_Init(uint8_t header0, uint8_t header1)
{
    AppProtocolStorageTypedef storage;

    s_commandProtocolConfig.format.header0 = header0;
    s_commandProtocolConfig.format.header1 = header1;
    s_commandProtocolConfig.format.lengthEndian = APP_PROTOCOL_ENDIAN_LITTLE;
    s_commandProtocolConfig.format.checksum = APP_PROTOCOL_CHECKSUM_XOR8;
    s_commandProtocolConfig.format.maxPayloadLength = APP_PROTOCOL_MAX_PAYLOAD_LEN;
    s_commandProtocolConfig.onFrame = app_protocol_on_frame;

    storage.rxBuffer = s_commandRxRing;
    storage.rxBufferBytes = (uint16_t)sizeof(s_commandRxRing);
    storage.payloadBuffer = s_commandPayload;
    storage.payloadBufferBytes = (uint16_t)sizeof(s_commandPayload);

    return (uint8_t)(AppProtocol_Init(&s_commandProtocol,
                                      &s_commandProtocolConfig,
                                      &storage,
                                      (void *)0) == APP_PROTOCOL_OK);
}

uint8_t AppProtocol_PushRxByte(uint8_t byteValue)
{
    return AppProtocol_PushByte(&s_commandProtocol, byteValue);
}

void AppFrontend_ProcessProtocol(void)
{
    /* Consume all currently buffered bytes. Partial frames remain in parser state. */
    (void)AppProtocol_Process(&s_commandProtocol, 0u);
}

/* Initial beam is loaded once; all captures use the same timed cycle path. */
uint8_t AppFrontend_Prime(void)
{
    s_bandwidthMHz = APP_FMCW_BANDWIDTH_DEFAULT_MHZ;
    return adt_scan_set_beam(APP_INITIAL_BEAM_INDEX);
}

/* Legacy automatic timing is disabled. Only AppSyncFrontend_Process owns ADC.
 * Keeping the symbol avoids breaking old headers while preventing P0/P1 writes.
 */
uint8_t AppFrontend_RunCycle(void)
{
    return 0u;
}
