/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef __SIGNAL_PROCESS_H
#define __SIGNAL_PROCESS_H

#include "adt6101P_type_define.h"

typedef enum
{
    SAMPLE_ADC0  = 0x0,
    SAMPLE_ADC1  = 0x1,
    SAMPLE_ADC01 = 0x2
} adcSampleSelTypedef;

typedef struct
{
    adcSampleSelTypedef adcSel;
    uint16 samplePoint;
    uint16 scanNum;
    uint16 dmaSrcStep;
    uint16 dmaDstStep;
    uint32 adc0SrcAddr;
    uint32 adc0DstAddr;
    uint32 adc1SrcAddr;
    uint32 adc1DstAddr;
    float stableTime;
} adcDspStrTypedef;

uint8 adcSampleStartAsync(const adcDspStrTypedef *adcDspStr);
uint8 adcSampleWait(void);

#endif /* __SIGNAL_PROCESS_H */
