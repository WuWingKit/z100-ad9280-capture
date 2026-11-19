/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef LAB_TIME_PROBE_H
#define LAB_TIME_PROBE_H

#include <stdint.h>

#include "app_timestamp.h"

typedef enum
{
    LAB_TIME_PROBE_OK = 0u,
    LAB_TIME_PROBE_BAD_PARAM,
    LAB_TIME_PROBE_BAD_STATE
} LabTimeProbeStatusTypedef;

typedef struct
{
    AppTimestampTickTypedef startTick;
    AppTimestampTickTypedef lastTicks;
    AppTimestampTickTypedef minTicks;
    AppTimestampTickTypedef maxTicks;
    uint32_t totalTicks;
    uint32_t sampleCount;
    uint8_t running;
} LabTimeProbeSlotTypedef;

typedef struct
{
    const AppTimestampTypedef *timestamp;
    LabTimeProbeSlotTypedef *slots;
    uint8_t slotCount;
    uint8_t initialized;
} LabTimeProbeTypedef;

LabTimeProbeStatusTypedef LabTimeProbe_Init(
    LabTimeProbeTypedef *probe,
    const AppTimestampTypedef *timestamp,
    LabTimeProbeSlotTypedef *slots,
    uint8_t slotCount);

LabTimeProbeStatusTypedef LabTimeProbe_Begin(
    LabTimeProbeTypedef *probe,
    uint8_t probeId);

LabTimeProbeStatusTypedef LabTimeProbe_End(
    LabTimeProbeTypedef *probe,
    uint8_t probeId);

const LabTimeProbeSlotTypedef *LabTimeProbe_Get(
    const LabTimeProbeTypedef *probe,
    uint8_t probeId);

#endif /* LAB_TIME_PROBE_H */
