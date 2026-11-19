/*
 * @author Andar
 * @note 厂商来源代码，保留原版权；项目适配归属见作者清单。
 * Attribution confidence: 中; basis recorded in AUTHORS.json.
 */
#ifndef __ADT6101P_INIT_H
#define __ADT6101P_INIT_H

#include "adt6101P_type_define.h"
#include "adt6101P_system.h"
#include "pmu_bsp.h"
#include "adc_bsp.h"
#include "signal_porcess.h"

typedef struct
{
    sysHighClkTypedef sysHighClk;
    AnalogDataStrTypedef rfStr;
    adcStrTypedef adcStr;
    adcDspStrTypedef adcDspStr;
} MMwMsgMCBStrTypedef;

extern MMwMsgMCBStrTypedef g_MMWMsgMCB;

void adt6101PSystemInit(void);
void adt6101PValInit(void);
void adt6101PModuleInit(void);
void ModuleResetClock(void);

#endif /* __ADT6101P_INIT_H */
