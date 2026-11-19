/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#include "app_hardware_ports.h"

static const AppHardwarePortsTypedef *s_ports;

uint8_t AppHardwarePorts_Bind(const AppHardwarePortsTypedef *ports)
{
    if((ports == (const AppHardwarePortsTypedef *)0) ||
       (ports->setBeam == 0) ||
       (ports->prepareBeam == 0) ||
       (ports->loadBeam == 0) ||
       (ports->captureStart == 0) ||
       (ports->captureWait == 0) ||
       (ports->configureBandwidth == 0) ||
       (ports->waitUs == 0))
    {
        s_ports = (const AppHardwarePortsTypedef *)0;
        return 0u;
    }
    s_ports = ports;
    return 1u;
}

uint8_t AppHardware_SetBeam(const uint8_t txPhase[16], const uint8_t rxPhase[16])
{
    return (uint8_t)((s_ports != 0) && (txPhase != 0) && (rxPhase != 0) &&
                     (s_ports->setBeam(s_ports->context, txPhase, rxPhase) != 0u));
}

uint8_t AppHardware_PrepareBeam(const uint8_t txPhase[16], const uint8_t rxPhase[16])
{
    return (uint8_t)((s_ports != 0) && (txPhase != 0) && (rxPhase != 0) &&
                     (s_ports->prepareBeam(s_ports->context, txPhase, rxPhase) != 0u));
}

uint8_t AppHardware_LoadBeam(void)
{
    return (uint8_t)((s_ports != 0) && (s_ports->loadBeam(s_ports->context) != 0u));
}

uint8_t AppHardware_CaptureStart(uint32_t adcDestinationAddr)
{
    return (uint8_t)((s_ports != 0) &&
                     (s_ports->captureStart(s_ports->context, adcDestinationAddr) != 0u));
}

uint8_t AppHardware_CaptureWait(void)
{
    return (uint8_t)((s_ports != 0) && (s_ports->captureWait(s_ports->context) != 0u));
}

uint8_t AppHardware_ConfigureBandwidth(uint16_t bandwidthMHz)
{
    return (uint8_t)((s_ports != 0) &&
                     (s_ports->configureBandwidth(s_ports->context, bandwidthMHz) != 0u));
}

void AppHardware_WaitUs(uint32_t microseconds)
{
    if(s_ports != 0)
    {
        s_ports->waitUs(s_ports->context, microseconds);
    }
}
