`timescale 1ns/1ps
// Compatibility shell: existing AXI GPIO controls a Mode0 16-bit SPI master.
// FIFO/DMA is idle in this diagnostic route; PS stores SPI responses in DDR.
module radar_frame_capture #(
 parameter integer SAMPLE_POINTS=512,CAPTURE_CHIRPS=24,PAYLOAD_CHIRPS=23,
 parameter integer SYNC_PULSE_CYCLES=1000,DEFAULT_PRIME_CYCLES=5000000,
 parameter integer DEFAULT_TIMEOUT_CYCLES=20000000,HALF_DIV=500
)(input wire axis_clk,rst_n,input wire [31:0] command,prime_wait_cycles,timeout_cycles,
 output reg spi_sclk,spi_cs_n,spi_mosi,input wire mcu_ready,output wire radar_sync,
 input wire fifo_full,stream_done,output wire [31:0] fifo_din,
 output wire fifo_wr_en,fifo_reset,axis_reset_n,output wire [15:0] frame_bytes_axis,
 output wire [31:0] status);
 wire reset_n=rst_n&&!command[2];
 (* ASYNC_REG="TRUE" *)reg [1:0] miso_pipe;
 reg seen,done,busy;reg [15:0] tx,rx;reg [5:0] edge_count;reg [15:0] divider;
 assign status={rx,14'd0,busy,done};
 assign fifo_din=0;assign fifo_wr_en=0;assign fifo_reset=!reset_n;
 assign axis_reset_n=reset_n;assign frame_bytes_axis=47136;assign radar_sync=0;
 always @(posedge axis_clk)begin
  miso_pipe<={miso_pipe[0],mcu_ready};
  if(!reset_n)begin seen<=0;done<=0;busy<=0;spi_sclk<=0;spi_cs_n<=1;
   spi_mosi<=0;tx<=0;rx<=0;divider<=0;edge_count<=0;end
  else if(!busy)begin
   if(command[0]!=seen)begin
    seen<=command[0];tx<=command[31:16];rx<=0;spi_mosi<=command[31];
    spi_cs_n<=0;spi_sclk<=0;busy<=1;divider<=HALF_DIV-1;edge_count<=0;
   end
  end else if(divider!=0)divider<=divider-1;
  else begin
   divider<=HALF_DIV-1;edge_count<=edge_count+1;
   if(!spi_sclk)begin spi_sclk<=1;rx<={rx[14:0],miso_pipe[1]};end
   else begin
    spi_sclk<=0;tx<={tx[14:0],1'b0};spi_mosi<=tx[14];
    if(edge_count==31)begin busy<=0;done<=seen;spi_cs_n<=1;end
   end
  end
 end
endmodule
