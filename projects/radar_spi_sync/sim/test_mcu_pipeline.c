/* Host test of the actual portable MCU C module, with mocked hardware ports.
 * Does not emulate MCU registers, IRQ latency, RF hardware or Keil firmware. */
#include "../mcu_sync/radar_sync_pipeline.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static unsigned char a[RADAR_CAPTURE_BYTES],b[RADAR_CAPTURE_BYTES];
typedef struct {
    RadarSyncPipeline *pipeline;
    unsigned captures,sends,checks;
    uint16_t loaded,prepared;
    int rf,send_failure;
} Model;
#define CHECK(m,x) do {(m)->checks++;assert(x);}while(0)
static uint32_t enter(void *v){(void)v;return 0;}
static void leave(void *v,uint32_t key){(void)v;(void)key;}
static int set_beam(void *v,uint16_t beam){((Model *)v)->loaded=beam;return 1;}
static int capture_start(void *v,uintptr_t addr){
    Model *m=v;unsigned i;
    CHECK(m,!m->rf);m->rf=1;
    for(i=0;i<RADAR_CAPTURE_BYTES;i++)((unsigned char *)addr)[i]=(unsigned char)(m->captures+i);
    m->captures++;
    /* A rising edge during acquisition must be rejected, not queued. */
    RadarSync_OnRisingEdge(m->pipeline);
    return 1;
}
static int finish(void *v){Model *m=v;CHECK(m,m->rf);m->rf=0;return 1;}
static void delay(void *v,uint32_t us){Model *m=v;CHECK(m,us==100);}
static int prepare(void *v,uint16_t beam){Model *m=v;CHECK(m,m->rf);m->prepared=beam;return 1;}
static int load(void *v){Model *m=v;CHECK(m,!m->rf);m->loaded=m->prepared;return 1;}
static void write_word(void *v,uintptr_t addr,uint16_t value){(void)v;memcpy((void *)addr,&value,2);}
static int send_frame(void *v,uintptr_t addr,uint32_t bytes){
    Model *m=v;uint16_t h[16],x=0;unsigned i,seq=m->sends;
    CHECK(m,!m->rf);CHECK(m,bytes==RADAR_SPI_BYTES);
    if(m->send_failure)return 0;
    memcpy(h,(void *)addr,32);
    for(i=0;i<16;i++)x^=h[i];
    CHECK(m,x==0&&h[0]==0xA610&&h[1]==3&&h[14]==seq);
    CHECK(m,h[3]==(9+seq)%19);
    CHECK(m,(int16_t)h[4]==-4500+(int)h[3]*500);
    if(m->sends<2){
        FILE *out=fopen(m->sends==0 ? "evidence/mcu_frame_0.bin" : "evidence/mcu_frame_1.bin","wb");
        CHECK(m,out!=NULL);CHECK(m,fwrite((void *)addr,1,bytes,out)==bytes);fclose(out);
    }
    for(i=32;i<bytes;i++)CHECK(m,((unsigned char *)addr)[i]==(unsigned char)(seq+RADAR_HEADER_OFFSET+i));
    m->sends++;return 1;
}
int main(void){
    RadarSyncPipeline p;Model m={0};unsigned i;int r;
    RadarSyncPorts ports={&m,enter,leave,set_beam,capture_start,finish,delay,prepare,load,send_frame,write_word};
    m.pipeline=&p;
    CHECK(&m,!RadarSync_Init(&p,&ports,(uintptr_t)a,(uintptr_t)a,1));
    CHECK(&m,RadarSync_Init(&p,&ports,(uintptr_t)a,(uintptr_t)b,1));
    CHECK(&m,RadarSync_Process(&p)==0);
    for(i=0;i<5;i++){
        RadarSync_OnRisingEdge(&p);r=RadarSync_Process(&p);
        CHECK(&m,r==(i==0?1:2));CHECK(&m,m.captures==i+1&&m.sends==i);
        CHECK(&m,p.previous==(int)(i%2));
        CHECK(&m,p.frame[i%2].state==RADAR_READY);
        if(i)CHECK(&m,p.frame[1-i%2].state==RADAR_FREE);
        CHECK(&m,p.rejected_sync==i+1&&!p.busy&&!p.pending);
    }
    m.send_failure=1;RadarSync_OnRisingEdge(&p);
    CHECK(&m,RadarSync_Process(&p)==-1&&p.fault&&!p.busy&&!m.rf);
    CHECK(&m,p.frame[0].state==RADAR_BAD&&p.frame[1].state==RADAR_BAD);
    RadarSync_OnRisingEdge(&p);CHECK(&m,!p.pending);
    CHECK(&m,RadarSync_Process(&p)==-1);
    printf("PASS actual_portable_MCU_C checks=%u captures=%u sent=%u previous_frame_header_payload=PASS alternating_ownership=PASS ISR_busy_reject=PASS send_fault_latch=PASS; mocked hardware only\n",m.checks,m.captures,m.sends);
    return 0;
}
