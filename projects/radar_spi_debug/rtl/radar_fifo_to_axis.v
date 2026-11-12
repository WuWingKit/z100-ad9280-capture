`timescale 1ns/1ps
// 32-bit FWFT FIFO adapter. Count only successful AXIS handshakes.
module radar_fifo_to_axis(
    input wire axis_clk,rst_n,
    input wire [31:0] fifo_dout,
    input wire fifo_empty,
    output wire fifo_rd_en,
    output wire [31:0] m_axis_tdata,
    output wire [3:0] m_axis_tkeep,
    output wire m_axis_tvalid,
    input wire m_axis_tready,
    output wire m_axis_tlast,stream_done,
    input wire [15:0] frame_bytes
);
    reg [15:0] word_count;
    assign m_axis_tdata=fifo_dout;
    assign m_axis_tkeep=4'hf;
    assign m_axis_tvalid=rst_n && !fifo_empty;
    assign fifo_rd_en=m_axis_tvalid && m_axis_tready;
    assign m_axis_tlast=m_axis_tvalid && frame_bytes!=0 &&
                       word_count==((frame_bytes>>2)-1'b1);
    assign stream_done=fifo_rd_en && m_axis_tlast;
    always @(posedge axis_clk or negedge rst_n)
        if(!rst_n) word_count<=0;
        else if(fifo_rd_en) word_count<=m_axis_tlast ? 16'd0 : word_count+1'b1;
endmodule
