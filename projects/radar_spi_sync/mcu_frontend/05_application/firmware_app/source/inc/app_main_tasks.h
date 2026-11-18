/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef APP_MAIN_TASKS_H
#define APP_MAIN_TASKS_H

#include <stdint.h>
#include "project_config.h"

uint8_t AppMainProtocol_Init(uint8_t header0, uint8_t header1);
uint8_t AppProtocol_PushRxByte(uint8_t byteValue);
void AppFrontend_ProcessProtocol(void);

/* Configure 0 deg once, then prime the first -45 deg frame into buffer A. */
uint8_t AppFrontend_Prime(void);

/* One steady-state ping-pong cycle. SPI stream completion is the work-complete event;
 * the function then idles until the configured period boundary. */
uint8_t AppFrontend_RunCycle(void);


#endif /* APP_MAIN_TASKS_H */
