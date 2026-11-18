#ifndef RADAR_SYNC_PIPELINE_H
#define RADAR_SYNC_PIPELINE_H
#include <stdint.h>
#include <stddef.h>
#define RADAR_CAPTURE_BYTES (49152u)
#define RADAR_HEADER_OFFSET (2016u)
#define RADAR_SPI_BYTES (47136u)
typedef enum {RADAR_FREE,RADAR_CAPTURING,RADAR_READY,RADAR_SENDING,RADAR_BAD} RadarFrameState;
typedef struct {
    uintptr_t base;
    uint32_t seq;
    uint16_t beam;
    int16_t angle_cdeg;
    uint32_t resolution_um;
    RadarFrameState state;
} RadarFrame;
typedef struct {
    void *context;
    uint32_t (*enter_critical)(void *);
    void (*leave_critical)(void *,uint32_t);
    int (*set_initial_beam)(void *,uint16_t);
    int (*capture_start)(void *,uintptr_t);
    int (*capture_finish)(void *); /* Must wait ADC DMA AND shut RF down. */
    void (*wait_us)(void *,uint32_t);
    int (*prepare_beam)(void *,uint16_t);
    int (*load_beam)(void *);
    int (*send_frame)(void *,uintptr_t,uint32_t); /* Must wait SPI busy=0, CS high. */
    void (*write_word)(void *,uintptr_t,uint16_t);
} RadarSyncPorts;
typedef struct {
    RadarSyncPorts ports;
    RadarFrame frame[2];
    volatile uint8_t pending,busy,fault;
    volatile uint32_t rejected_sync;
    uint32_t next_seq;
    uint16_t loaded_beam;
    uint8_t scan_enabled;
    int8_t previous;
} RadarSyncPipeline;
/* Call with SYNC interrupt disabled. Initial fixed beam is index9=0 degrees. */
int RadarSync_Init(RadarSyncPipeline *,const RadarSyncPorts *,uintptr_t,uintptr_t,int);
/* Call only from the confirmed GPIO rising-edge ISR. No RF/SPI work in ISR. */
void RadarSync_OnRisingEdge(RadarSyncPipeline *);
/* Poll from main. 0=no event, 1=prime captured, 2=previous frame sent, -1=fault. */
int RadarSync_Process(RadarSyncPipeline *);
#endif
