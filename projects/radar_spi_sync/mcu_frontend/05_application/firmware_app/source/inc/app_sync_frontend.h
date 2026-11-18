#ifndef APP_SYNC_FRONTEND_H
#define APP_SYNC_FRONTEND_H
#include <stdint.h>
#include "radar_sync_pipeline.h"

/* Watch these symbols in Keil. No automatic RF acquisition is performed. */
extern RadarSyncPipeline g_radarSync;
extern volatile int g_radarLastResult;
extern volatile uint32_t g_radarCompletedEvents;
uint8_t AppSyncFrontend_Init(void);
void AppSyncFrontend_Process(void);
void AppSyncFrontend_GpioIsr(void);
#endif
