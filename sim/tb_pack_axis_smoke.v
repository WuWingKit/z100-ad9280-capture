`timescale 1ns/1ps
// Ideal single-clock FWFT queue for protocol checks, not a vendor async FIFO model.
module tb_pack_axis_smoke;
  reg clk=0, rst_n=0, start=0, adc_valid=0, ready=0, force_full=0;
  reg [7:0] adc_data=0;
  wire [31:0] din, tdata;
  wire wr_en, rd_en, busy, done, overflow, valid, last, stream_done;
  wire [6:0] sample_count;
  wire [3:0] keep;
  reg [31:0] mem[0:63];
  integer wp=0, rp=0, count=0, cycles=0, words=0, stalls=0, ends=0, lane;
  reg held=0;
  reg [36:0] held_value;
  wire empty=(count==0);
  wire full=force_full || (count==64);
  always #5 clk=~clk;
  adc_capture_pack packer(clk,rst_n,start,adc_data,adc_valid,full,din,wr_en,busy,done,overflow,sample_count);
  fifo_to_axis streamer(clk,rst_n,mem[rp],empty,rd_en,tdata,keep,valid,ready,last,stream_done);
  always @(posedge clk) begin
    if(!rst_n) begin wp<=0; rp<=0; count<=0; end
    else begin
      case({wr_en,rd_en})
        2'b10: count<=count+1;
        2'b01: count<=count-1;
      endcase
      if(wr_en) begin mem[wp]<=din; wp<=(wp+1)%64; end
      if(rd_en) rp<=(rp+1)%64;
    end
  end
  always @(negedge clk) begin
    if(!rst_n) begin cycles=0; ready=0; end
    else begin cycles=cycles+1; ready=(cycles>30 && cycles%7>=2); end
  end
  always @(posedge clk) begin
    if(!rst_n) held=0;
    else begin
      if(held && (!valid || {last,keep,tdata} !== held_value)) $fatal(1,"AXIS changed under backpressure");
      held=valid && !ready;
      if(held) begin held_value={last,keep,tdata}; stalls=stalls+1; end
      if(valid && ready) begin
        if(words>=50) $fatal(1,"extra word");
        for(lane=0;lane<4;lane=lane+1)
          if(tdata[lane*8+:8] !== ((words*4+lane)&8'hff)) $fatal(1,"sample order at word %0d lane %0d",words,lane);
        if(keep!==4'hf || last!==(words%25==24)) $fatal(1,"keep/TLAST");
        if(stream_done!==last) $fatal(1,"stream_done handshake");
        if(last) ends=ends+1;
        words=words+1;
      end
    end
  end
  task capture;
    input integer base;
    integer i;
    begin
      @(negedge clk); start=1;
      @(negedge clk); start=0;
      for(i=0;i<100;i=i+1) begin
        if(i%5==0) begin adc_valid=0; @(negedge clk); end
        adc_valid=1; adc_data=base+i;
        @(negedge clk);
      end
      adc_valid=0;
      if(sample_count!==100 || busy || overflow || !done) $fatal(1,"capture completion");
    end
  endtask
  initial begin
    #22; rst_n=1;
    capture(0); wait(words==25);
    capture(100); wait(words==50);
    if(ends!=2 || stalls==0) $fatal(1,"frame or stall coverage");
    @(negedge clk); force_full=1; start=1;
    @(negedge clk); start=0; adc_valid=1;
    @(negedge clk); adc_valid=0;
    if(!overflow || busy || done || sample_count!=0 || wr_en) $fatal(1,"full must abort without success");
    $display("RTL_SMOKE_PASS frames=2 samples=200 words=50 stalls=%0d full_abort=PASS",stalls);
    $finish;
  end
  initial begin #100000; $fatal(1,"timeout"); end
endmodule
