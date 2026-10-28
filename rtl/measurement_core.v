`timescale 1ns/1ps
// GPIO0 command/status; GPIO1 phase increment/config. Config must be held
// stable before START and until DMA acknowledgement. No hardware auto-start.
module measurement_core(
 input wire axis_clk, adc_clk, rst_n,
 input wire [31:0] command, phase_word, config_word,
 input wire [7:0] adc_raw,
 input wire adc_otr, fifo_full, stream_done,
 input wire [31:0] cordic_data,
 input wire cordic_valid,
 output wire [15:0] phase_data,
 output wire phase_valid, cordic_reset_n,
 output reg [7:0] dac_data,
 output wire [31:0] fifo_din,
 output wire fifo_wr_en, fifo_reset, axis_reset_n,
 output wire [12:0] frame_samples_axis,
 output wire [31:0] status
);
 localparam IDLE=0, PREP=1, WAIT_ZERO=2, DELAY=3, CAPTURE=4, WAIT_DMA=5, ERROR=6;
 wire logic_rst_n=rst_n && !command[2];
 (* ASYNC_REG="TRUE" *) reg [1:0] ar, xr, req_sync, ack_sync;
 (* ASYNC_REG="TRUE" *) reg [31:0] cfg_meta,cfg_sync,k_meta,k_sync;
 (* ASYNC_REG="TRUE" *) reg [1:0] mode_sync, armed_sync;
 reg [3:0] state;
 reg req_seen, ack_seen, test_mode, rejected, overflow, otr, frame_done;
 reg [31:0] phase, increment;
 reg [12:0] length, count;
 reg [7:0] delay_count, delay_cfg;
 reg [6:0] amplitude;
 reg [2:0] prep_count;
 reg [2:0] cordic_warm;
 reg [23:0] pack;
 reg wave_started, heartbeat;
 reg [15:0] heartbeat_count;
 wire adc_reset_n=ar[1];
 wire active_wave=!test_mode && state>=WAIT_ZERO && state<=WAIT_DMA;
 wire [7:0] sample=test_mode ? count[7:0] : adc_raw;
 wire valid_length=cfg_sync[12:0]>=256 && cfg_sync[12:0]<=4096 &&
                   ((cfg_sync[12:0] & (cfg_sync[12:0]-1'b1))==0);
 wire signed [15:0] sine=cordic_data[31:16];
 wire signed [23:0] product=sine * $signed({1'b0,amplitude});
 wire signed [24:0] dac_code=25'sd128 - (product >>> 14);
 assign phase_data={{2{phase[31]}},phase[31:18]}; // Q3.13 scaled radians (pi units)
 assign phase_valid=active_wave && (&cordic_warm);
 assign cordic_reset_n=adc_reset_n && state!=PREP;
 assign fifo_din={sample,pack};
 assign fifo_wr_en=adc_reset_n && state==CAPTURE && count<length && !fifo_full && count[1:0]==3;
 assign fifo_reset=!logic_rst_n;
 assign axis_reset_n=xr[1];
 always @(posedge adc_clk or negedge logic_rst_n)
  if(!logic_rst_n) ar<=0; else ar<={ar[0],1'b1};
 always @(posedge axis_clk or negedge logic_rst_n)
  if(!logic_rst_n) xr<=0; else xr<={xr[0],1'b1};
 always @(posedge adc_clk or negedge adc_reset_n) begin
  if(!adc_reset_n) begin
   req_sync<=0;ack_sync<=0;mode_sync<=0;armed_sync<=0;
   cfg_meta<=0;cfg_sync<=0;k_meta<=0;k_sync<=0;
   state<=IDLE;req_seen<=0;ack_seen<=0;test_mode<=0;rejected<=0;
   overflow<=0;otr<=0;frame_done<=0;phase<=0;increment<=0;
   length<=4096;count<=0;delay_cfg<=0;delay_count<=0;amplitude<=0;
   prep_count<=0;cordic_warm<=0;pack<=0;wave_started<=0;dac_data<=128;
   heartbeat<=0;heartbeat_count<=0;
  end else begin
   req_sync<={req_sync[0],command[0]};ack_sync<={ack_sync[0],command[4]};
   mode_sync<={mode_sync[0],command[1]};armed_sync<={armed_sync[0],command[3]};
   cfg_meta<=config_word;cfg_sync<=cfg_meta;k_meta<=phase_word;k_sync<=k_meta;
   heartbeat_count<=heartbeat_count+1'b1;
   if(&heartbeat_count)heartbeat<=~heartbeat;
   if(!active_wave)cordic_warm<=0;else cordic_warm<={cordic_warm[1:0],1'b1};
   if(phase_valid) phase<=phase+increment;
   if(active_wave && cordic_valid) begin
    if(dac_code<0)dac_data<=0;
    else if(dac_code>255)dac_data<=255;
    else dac_data<=dac_code[7:0];
   end else if(!active_wave) dac_data<=128;
   if(req_sync[1]!=req_seen) begin
    req_seen<=req_sync[1];
    if(state!=IDLE || !armed_sync[1] || !valid_length || k_sync==0 || k_sync>=32'h80000000) begin
     rejected<=1;
     if(state==IDLE)state<=ERROR;
    end else begin
     increment<=k_sync;length<=cfg_sync[12:0];delay_cfg<=cfg_sync[20:13];
     amplitude<=cfg_sync[27:21];test_mode<=mode_sync[1];
     rejected<=0;overflow<=0;otr<=0;frame_done<=0;count<=0;pack<=0;
     phase<=0;prep_count<=0;wave_started<=0;ack_seen<=ack_sync[1];state<=PREP;
    end
   end else case(state)
    PREP: begin
     phase<=0;
     if(prep_count==3) begin
      if(test_mode) begin state<=DELAY;delay_count<=0;end
      else state<=WAIT_ZERO;
     end else prep_count<=prep_count+1'b1;
    end
    WAIT_ZERO: if(cordic_valid) begin
     wave_started<=1;delay_count<=delay_cfg;state<=DELAY;
    end
    DELAY: if(delay_count==0)state<=CAPTURE;else delay_count<=delay_count-1'b1;
    CAPTURE: begin
     if(adc_otr && !test_mode)otr<=1;
     if(fifo_full)begin overflow<=1;state<=ERROR;end
     else if(count<length)begin
      case(count[1:0])
       0:pack[7:0]<=sample;
       1:pack[15:8]<=sample;
       2:pack[23:16]<=sample;
       3:pack<=0;
      endcase
      count<=count+1'b1;
      if(count==length-1'b1)begin frame_done<=1;state<=WAIT_DMA;end
     end
    end
    WAIT_DMA: if(ack_sync[1]!=ack_seen)begin ack_seen<=ack_sync[1];state<=IDLE;end
   endcase
  end
 end
 (* ASYNC_REG="TRUE" *) reg [31:0] smeta,ssync;
 (* ASYNC_REG="TRUE" *) reg [12:0] nmeta,nsync;
 reg streamed,req_axis;
 wire [31:0] adc_status_next={4'd0,(state==WAIT_DMA),wave_started,1'b0,state,
                         count,otr,adc_reset_n,heartbeat,rejected,req_seen,frame_done,overflow,
                         (state!=IDLE && state!=ERROR)};
 reg [31:0] adc_status;
 always @(posedge adc_clk or negedge adc_reset_n)
  if(!adc_reset_n)adc_status<=0;else adc_status<=adc_status_next;
 always @(posedge axis_clk or negedge axis_reset_n) begin
  if(!axis_reset_n)begin smeta<=0;ssync<=0;nmeta<=4096;nsync<=4096;streamed<=0;req_axis<=0;end
  else begin
   smeta<=adc_status;ssync<=smeta;nmeta<=length;nsync<=nmeta;
   req_axis<=command[0];
   if(command[0]!=req_axis)streamed<=0;
   else if(stream_done)streamed<=1;
  end
 end
 assign frame_samples_axis=nsync;
 assign status=(ssync & 32'hfdffffff) | ({31'd0,streamed}<<25);
endmodule


