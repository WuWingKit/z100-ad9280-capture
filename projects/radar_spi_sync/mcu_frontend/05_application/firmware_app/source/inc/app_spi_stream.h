/*
 * @author jingran Cheng
 * @note 仅供内部学习使用。
 * Attribution: confirmed by project owner; see AUTHORS.json.
 */
#ifndef APP_SPI_STREAM_H
#define APP_SPI_STREAM_H

#include <stdint.h>

void AppSpiStream_Init(void);
uint8_t AppSpiStream_Prepare(uint32_t sourceAddr);
uint8_t AppSpiStream_Trigger(void);
uint8_t AppSpiStream_Wait(void);
void AppSpiStream_Abort(void);
void AppSpiStream_DmaCompleteIsr(void);

#endif /* APP_SPI_STREAM_H */
