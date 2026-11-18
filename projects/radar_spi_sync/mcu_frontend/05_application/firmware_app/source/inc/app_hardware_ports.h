/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef APP_HARDWARE_PORTS_H
#define APP_HARDWARE_PORTS_H

#include <stdint.h>

typedef struct
{
    void *context;
    uint8_t (*setBeam)(void *context,
                       const uint8_t txPhase[16],
                       const uint8_t rxPhase[16]);
    uint8_t (*prepareBeam)(void *context,
                           const uint8_t txPhase[16],
                           const uint8_t rxPhase[16]);
    uint8_t (*loadBeam)(void *context);
    uint8_t (*captureStart)(void *context, uint32_t adcDestinationAddr);
    uint8_t (*captureWait)(void *context);
    uint8_t (*configureBandwidth)(void *context, uint16_t bandwidthMHz);
    void (*waitUs)(void *context, uint32_t microseconds);
} AppHardwarePortsTypedef;

uint8_t AppHardwarePorts_Bind(const AppHardwarePortsTypedef *ports);
uint8_t AppHardware_SetBeam(const uint8_t txPhase[16], const uint8_t rxPhase[16]);
uint8_t AppHardware_PrepareBeam(const uint8_t txPhase[16], const uint8_t rxPhase[16]);
uint8_t AppHardware_LoadBeam(void);
uint8_t AppHardware_CaptureStart(uint32_t adcDestinationAddr);
uint8_t AppHardware_CaptureWait(void);
uint8_t AppHardware_ConfigureBandwidth(uint16_t bandwidthMHz);
void AppHardware_WaitUs(uint32_t microseconds);

#endif /* APP_HARDWARE_PORTS_H */
