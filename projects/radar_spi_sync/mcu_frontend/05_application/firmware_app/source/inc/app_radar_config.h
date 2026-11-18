/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef APP_RADAR_CONFIG_H
#define APP_RADAR_CONFIG_H

#include <stdint.h>
#include "project_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define APP_BEAM_COUNT                         PROJECT_FRONTEND_BEAM_COUNT
#define APP_INITIAL_BEAM_INDEX                 PROJECT_FRONTEND_INITIAL_BEAM_INDEX
#define APP_PROTOCOL_CMD_FMCW_BANDWIDTH        (0x29u)
#define APP_PROTOCOL_CMD_RUN_INTERVAL_MS       (0x2Au)
#define APP_FMCW_BANDWIDTH_DEFAULT_MHZ         PROJECT_FMCW_BANDWIDTH_DEFAULT_MHZ
#define APP_FMCW_BANDWIDTH_MIN_MHZ             PROJECT_FMCW_BANDWIDTH_MIN_MHZ
#define APP_FMCW_BANDWIDTH_MAX_MHZ             PROJECT_FMCW_BANDWIDTH_MAX_MHZ
#define APP_RUN_INTERVAL_DEFAULT_MS            PROJECT_TRACKING_RUN_INTERVAL_DEFAULT_MS
#define APP_RUN_INTERVAL_MIN_MS                PROJECT_TRACKING_RUN_INTERVAL_MIN_MS
#define APP_RUN_INTERVAL_MAX_MS                PROJECT_TRACKING_RUN_INTERVAL_MAX_MS

typedef struct
{
    uint8_t psr[16];
} AppWaveBeamTypedef;

extern const AppWaveBeamTypedef g_appBeamTx[APP_BEAM_COUNT];
extern const AppWaveBeamTypedef g_appBeamRx[APP_BEAM_COUNT];

#ifdef __cplusplus
}
#endif

#endif /* APP_RADAR_CONFIG_H */
