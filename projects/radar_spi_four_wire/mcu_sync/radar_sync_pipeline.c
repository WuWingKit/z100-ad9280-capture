#include "radar_sync_pipeline.h"
#include <string.h>
static void seal_header(RadarSyncPipeline *p,RadarFrame *f) {
    uint16_t h[16],x=0;unsigned i;
    h[0]=0xA610;h[1]=3;h[2]=16;h[3]=f->beam;h[4]=(uint16_t)f->angle_cdeg;
    h[5]=19;h[6]=512;h[7]=23;h[8]=24;h[9]=4;
    h[10]=(uint16_t)f->resolution_um;h[11]=(uint16_t)(f->resolution_um>>16);
    h[12]=RADAR_SPI_BYTES;h[13]=32;h[14]=(uint16_t)f->seq;
    for(i=0;i<15;i++) { x^=h[i]; }
    h[15]=x;
    for(i=0;i<16;i++)p->ports.write_word(p->ports.context,f->base+RADAR_HEADER_OFFSET+2u*i,h[i]);
}
int RadarSync_Init(RadarSyncPipeline *p,const RadarSyncPorts *ports,uintptr_t a,uintptr_t b,int scan) {
    if(!p||!ports||!ports->enter_critical||!ports->leave_critical||
       !ports->set_initial_beam||!ports->capture_start||!ports->capture_finish||
       !ports->wait_us||!ports->prepare_beam||!ports->load_beam||
       !ports->send_frame||!ports->write_word||(a&3u)||(b&3u)||
       a+RADAR_CAPTURE_BYTES<a||b+RADAR_CAPTURE_BYTES<b||
       !(a+RADAR_CAPTURE_BYTES<=b || b+RADAR_CAPTURE_BYTES<=a))return 0;
    memset(p,0,sizeof(*p));p->ports=*ports;p->frame[0].base=a;p->frame[1].base=b;
    p->previous=-1;p->loaded_beam=9;p->scan_enabled=scan!=0;
    if(!ports->set_initial_beam(ports->context,9)){p->fault=1;return 0;}
    return 1;
}
void RadarSync_OnRisingEdge(RadarSyncPipeline *p) {
    if(p->busy||p->pending||p->fault){p->rejected_sync++;return;}
    p->pending=1;
}
int RadarSync_Process(RadarSyncPipeline *p) {
    RadarFrame *f,*old;
    unsigned cap,next_beam;
    int result;
    uint32_t irq;
    void *ctx=p->ports.context;
    irq=p->ports.enter_critical(ctx);
    if(p->fault){p->ports.leave_critical(ctx,irq);return -1;}
    if(!p->pending){p->ports.leave_critical(ctx,irq);return 0;}
    p->pending=0;p->busy=1;p->ports.leave_critical(ctx,irq);
    cap=p->previous<0 ? 0u : 1u-(unsigned)p->previous;
    f=&p->frame[cap];
    if(f->state!=RADAR_FREE)goto fail;
    f->seq=p->next_seq++;f->beam=p->loaded_beam;
    f->angle_cdeg=(int16_t)(-4500+(int)f->beam*500);
    f->resolution_um=150000000u/1400u; /* Current fixed 1400MHz; snapshot actual config if made mutable. */
    f->state=RADAR_CAPTURING;
    if(!p->ports.capture_start(ctx,f->base))goto fail;
    p->ports.wait_us(ctx,100);
    next_beam=p->scan_enabled ? (p->loaded_beam+1u)%19u : p->loaded_beam;
    if(!p->ports.prepare_beam(ctx,(uint16_t)next_beam)) {
        (void)p->ports.capture_finish(ctx);goto fail;
    }
    if(!p->ports.capture_finish(ctx))goto fail;
    if(!p->ports.load_beam(ctx))goto fail;
    p->loaded_beam=(uint16_t)next_beam;
    seal_header(p,f);f->state=RADAR_READY;
    result=1;
    if(p->previous>=0) {
        old=&p->frame[(unsigned)p->previous];
        if(old->state!=RADAR_READY)goto fail;
        old->state=RADAR_SENDING;
        if(!p->ports.send_frame(ctx,old->base+RADAR_HEADER_OFFSET,RADAR_SPI_BYTES)) {
            old->state=RADAR_BAD;goto fail;
        }
        old->state=RADAR_FREE;result=2;
    }
    p->previous=(int8_t)cap;
    irq=p->ports.enter_critical(ctx);p->busy=0;p->ports.leave_critical(ctx,irq);
    return result;
fail:
    f->state=RADAR_BAD;
    irq=p->ports.enter_critical(ctx);p->fault=1;p->busy=0;p->ports.leave_critical(ctx,irq);
    return -1;
}
