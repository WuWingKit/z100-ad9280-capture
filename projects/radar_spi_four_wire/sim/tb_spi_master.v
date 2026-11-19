`timescale 1ns/1ps
module tb_spi_master;
reg clk=0;always #5 clk=~clk;
reg rst=0;reg [31:0] cmd=0;wire sck,cs,mosi;wire [31:0] status;
integer rises=0;reg [15:0] slave_rx=0;
radar_frame_capture #(.HALF_DIV(5)) dut(.axis_clk(clk),.rst_n(rst),.command(cmd),.prime_wait_cycles(32'd0),.timeout_cycles(32'd0),.spi_sclk(sck),.spi_cs_n(cs),.spi_mosi(mosi),.mcu_ready(mosi),.fifo_full(1'b0),.stream_done(1'b0),.status(status));
always @(posedge sck)if(!cs)begin slave_rx={slave_rx[14:0],mosi};rises=rises+1;end
initial begin
 #100 rst=1;#100;cmd=32'ha65c0001;#2000;
 if(status[31:16]!==16'ha65c||slave_rx!==16'ha65c||rises!=16||!cs||sck||status[1:0]!=1)$fatal(1,"Master loopback mismatch");
 $display("SPI_MASTER_MODE0_PASS edges=16 msb_first=1 full_duplex=1");$finish;
end
endmodule
