/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#include "lab_time_probe.h"

#include <string.h>

LabTimeProbeStatusTypedef LabTimeProbe_Init(
    LabTimeProbeTypedef *probe,
    const AppTimestampTypedef *timestamp,
    LabTimeProbeSlotTypedef *slots,
    uint8_t slotCount)
{
    if((probe == (LabTimeProbeTypedef *)0) ||
       (timestamp == (const AppTimestampTypedef *)0) ||
       (timestamp->initialized == 0u) ||
       (slots == (LabTimeProbeSlotTypedef *)0) ||
       (slotCount == 0u))
    {
        return LAB_TIME_PROBE_BAD_PARAM;
    }

    (void)memset(slots, 0, sizeof(*slots) * slotCount);
    probe->timestamp = timestamp;
    probe->slots = slots;
    probe->slotCount = slotCount;
    probe->initialized = 1u;
    return LAB_TIME_PROBE_OK;
}

LabTimeProbeStatusTypedef LabTimeProbe_Begin(
    LabTimeProbeTypedef *probe,
    uint8_t probeId)
{
    LabTimeProbeSlotTypedef *slot;

    if((probe == (LabTimeProbeTypedef *)0) ||
       (probe->initialized == 0u) ||
       (probeId >= probe->slotCount))
    {
        return LAB_TIME_PROBE_BAD_PARAM;
    }

    slot = &probe->slots[probeId];
    slot->startTick = AppTimestamp_Now(probe->timestamp);
    slot->running = 1u;
    return LAB_TIME_PROBE_OK;
}

LabTimeProbeStatusTypedef LabTimeProbe_End(
    LabTimeProbeTypedef *probe,
    uint8_t probeId)
{
    LabTimeProbeSlotTypedef *slot;
    AppTimestampTickTypedef elapsed;

    if((probe == (LabTimeProbeTypedef *)0) ||
       (probe->initialized == 0u) ||
       (probeId >= probe->slotCount))
    {
        return LAB_TIME_PROBE_BAD_PARAM;
    }

    slot = &probe->slots[probeId];
    if(slot->running == 0u)
    {
        return LAB_TIME_PROBE_BAD_STATE;
    }

    elapsed = AppTimestamp_Now(probe->timestamp) - slot->startTick;
    slot->lastTicks = elapsed;
    if((slot->sampleCount == 0u) || (elapsed < slot->minTicks))
    {
        slot->minTicks = elapsed;
    }
    if((slot->sampleCount == 0u) || (elapsed > slot->maxTicks))
    {
        slot->maxTicks = elapsed;
    }
    slot->totalTicks += elapsed;
    slot->sampleCount++;
    slot->running = 0u;
    return LAB_TIME_PROBE_OK;
}

const LabTimeProbeSlotTypedef *LabTimeProbe_Get(
    const LabTimeProbeTypedef *probe,
    uint8_t probeId)
{
    if((probe == (const LabTimeProbeTypedef *)0) ||
       (probe->initialized == 0u) ||
       (probeId >= probe->slotCount))
    {
        return (const LabTimeProbeSlotTypedef *)0;
    }
    return &probe->slots[probeId];
}
