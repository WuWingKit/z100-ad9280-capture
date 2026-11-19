/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef APP_TIMEBASE_H
#define APP_TIMEBASE_H

#include <stdint.h>
#include "project_config.h"

#define APP_TIMEBASE_TIMER1_TICK_US   PROJECT_TIMEBASE_TICK_US

void AppTimebase_InitTimer1(void);
void AppTimebase_OnTimer1Interrupt(void);
uint32_t AppTimebase_NowMs(void *context);

#endif /* APP_TIMEBASE_H */
