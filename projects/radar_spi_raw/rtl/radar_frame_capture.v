`timescale 1ns/1ps
// RAW SPI flight recorder. Mode 0, byte-oriented, no radar protocol checks.
// Collect every complete byte across CS segments until READY completes or timeout.
// Storage is bounded; total byte/CS/partial counters expose truncation explicitly.
module radar_frame_capture #(
 parameter integer SAMPLE_POINTS=512,CAPTURE_CHIRPS=24,PAYLOAD_CHIRPS=23,
 parameter integer SYNC_PULSE_CYCLES=1000,DEFAULT_PRIME_CYCLES=5000000,
 parameter integer DEFAULT_TIMEOUT_CYCLES=20000000
)(input wire axis_clk,rst_n,input wire [31:0] command,prime_wait_cycles,timeout_cycles,
 input wire spi_sclk,spi_cs_n,spi_mosi,mcu_ready,output reg radar_sync,
 input wire fifo_full,stream_done,output wire [31:0] fifo_din,
 output wire fifo_wr_en,fifo_reset,axis_reset_n,output wire [15:0] frame_bytes_axis,
 output wire [31:0] status);
 localparam MAX=32+PAYLOAD_CHIRPS*SAMPLE_POINTS*4,WORDS=MAX/4;
 localparam IDLE=0,PRIME=1,RX=2,FETCH=3,PUT=4,DRAIN=5,ACK=6,FAULT=7;
 wire reset_n=rst_n&&!command[2];
 (* ASYNC_REG="TRUE" *) reg [2:0] cp,sp,mp,rp;
 reg pc,ps;wire start=pc&&!cp[2],finish=!pc&&cp[2],rise=!ps&&sp[2];
 reg [3:0] state;reg seen,ack_seen,primed,went_low,streamed;
 reg [31:0] timer,pulse,total_bytes,cs_count,partial_count;
 reg [15:0] stored;reg [2:0] bits,last_partial_bits;reg [7:0] shift,last_partial;
 reg [31:0] packed; reg overflow,timed_out;
 reg [15:0] read_index;reg [31:0] ram_q;
 (* ram_style="block" *) reg [31:0] mem[0:WORDS-1];
 reg we;reg [15:0] wa;reg [31:0] wd;
 wire [7:0] nextbyte={shift[6:0],mp[2]};
 assign axis_reset_n=reset_n;assign fifo_reset=!reset_n;
 assign frame_bytes_axis=MAX;assign fifo_din=ram_q;
 assign fifo_wr_en=reset_n&&state==PUT&&!fifo_full;
 wire [31:0] normal={rp[2],command[1],command[3],streamed,state,stored,primed,
 (state==IDLE&&!radar_sync&&rp[2]),5'b0,(state!=IDLE)};
 // Diagnostic pages retain full counters; command[0] is never toggled to read.
 assign status=!command[7]?normal:command[6:5]==0?total_bytes:
 command[6:5]==1?cs_count:command[6:5]==2?partial_count:
 {14'd0,timed_out,overflow,last_partial_bits,5'd0,last_partial};
 always @(posedge axis_clk) begin
  if(we)mem[wa]<=wd;
  if(state==FETCH)ram_q<=read_index<((stored+3)>>2)?mem[read_index]:32'd0;
 end
 always @(posedge axis_clk or negedge reset_n) begin
  if(!reset_n)begin cp<=7;sp<=0;mp<=0;rp<=0;pc<=1;ps<=0;end
  else begin cp<={cp[1:0],spi_cs_n};sp<={sp[1:0],spi_sclk};
   mp<={mp[1:0],spi_mosi};rp<={rp[1:0],mcu_ready};pc<=cp[2];ps<=sp[2];end
 end
 always @(posedge axis_clk) begin
  if(!reset_n)begin
   state<=IDLE;seen<=0;ack_seen<=0;primed<=0;radar_sync<=0;went_low<=0;
   timer<=0;pulse<=0;stored<=0;total_bytes<=0;cs_count<=0;partial_count<=0;
   bits<=0;shift<=0;last_partial<=0;last_partial_bits<=0;packed<=0;
   read_index<=0;streamed<=0;overflow<=0;timed_out<=0;we<=0;wa<=0;wd<=0;
  end else begin
   we<=0;
   if(pulse!=0)begin pulse<=pulse-1;if(pulse==1)radar_sync<=0;end
   if(state!=IDLE)timer<=timer+1;
   if(stream_done)streamed<=1;
   if(command[0]!=seen&&state==IDLE)begin
    seen<=command[0];ack_seen<=command[4];timer<=0;went_low<=0;
    if(rp[2]&&cp[2])begin
     radar_sync<=1;pulse<=SYNC_PULSE_CYCLES;state<=command[1]?PRIME:RX;
     stored<=0;total_bytes<=0;cs_count<=0;partial_count<=0;bits<=0;shift<=0;
     last_partial<=0;last_partial_bits<=0;packed<=0;overflow<=0;timed_out<=0;streamed<=0;
    end
   end else case(state)
    PRIME:begin
     if(!rp[2])went_low<=1;
     if(went_low&&rp[2]&&!radar_sync)begin primed<=1;state<=IDLE;end
     if(timer>=(prime_wait_cycles!=0?prime_wait_cycles:DEFAULT_PRIME_CYCLES))state<=FAULT;
    end
    RX:begin
     if(!rp[2])went_low<=1;
     if(start)begin cs_count<=cs_count+1;bits<=0;shift<=0;end
     else if(!cp[2]&&rise)begin
      shift<={shift[6:0],mp[2]};bits<=bits+1;
      if(bits==7)begin
       total_bytes<=total_bytes+1;
       if(stored<MAX)begin
        packed[8*stored[1:0]+:8]<=nextbyte;stored<=stored+1;
        if(stored[1:0]==3)begin we<=1;wa<=stored>>2;wd<={nextbyte,packed[23:0]};packed<=0;end
       end else overflow<=1;
      end
     end
     if(finish)begin
      if(bits!=0)begin partial_count<=partial_count+1;last_partial<=shift;last_partial_bits<=bits;end
      bits<=0;shift<=0;
     end
     if((went_low&&rp[2]&&cp[2]&&!finish&&timer>SYNC_PULSE_CYCLES+10)||
        timer>=(timeout_cycles!=0?timeout_cycles:DEFAULT_TIMEOUT_CYCLES))begin
      timed_out<=!(went_low&&rp[2]&&cp[2]);
      if(stored[1:0]!=0)begin we<=1;wa<=stored>>2;wd<=packed;end
      read_index<=0;state<=FETCH;
     end
    end
    FETCH:state<=PUT;
    PUT:if(!fifo_full)begin
     if(read_index==WORDS-1)state<=DRAIN;else begin read_index<=read_index+1;state<=FETCH;end
    end
    DRAIN:if(stream_done||streamed)state<=ACK;
    ACK:if(command[4]!=ack_seen)begin ack_seen<=command[4];state<=IDLE;end
    default:begin end
   endcase
  end
 end
endmodule

