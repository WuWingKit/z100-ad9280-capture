/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef APP_SCAN_H
#define APP_SCAN_H

#include <stdint.h>
#include "app_radar_config.h"

uint8_t adt_scan_set_beam(uint8_t beamIndex);
uint8_t adt_scan_prepare_beam(uint8_t beamIndex);
uint8_t adt_scan_load_beam(void);
uint8_t adt_scan_acquire_start(uint32_t adcDestinationAddr);
uint8_t adt_scan_acquire_wait(void);

#endif /* APP_SCAN_H */
