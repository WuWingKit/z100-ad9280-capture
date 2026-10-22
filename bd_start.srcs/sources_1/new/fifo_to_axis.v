`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2026/10/21 16:41:05
// Design Name: 
// Module Name: fifo_to_axis
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module fifo_to_axis(
    input   wire        axis_clk,       // FIFO读侧、AXI侧时钟
    input   wire        rst_n,          // 低有效复位
    
    // FIFO读接口
    input   wire [31:0] fifo_dout,      // FIFO头部数据
    input   wire        fifo_empty,     // FIFO空标志
    output  wire        fifo_rd_en,     // 消费当前FIFO数据
    
    // AXI-Stream输出接口
    output  wire [31:0] m_axis_tdata,   // 输出数据
    output  wire [3:0]  m_axis_tkeep,   // 4个字节是否有效
    output  wire        m_axis_tvalid,  // 输出数据是否有效
    input   wire        m_axis_tready,  // 下游准备接收
    output  wire        m_axis_tlast,   // 本帧最后一个字
    
    output  wire        stream_done     //  最后一个字成功传输
    );
    
    // 已成功传输的字位置：0~24
    reg [4:0] word_count;
    
    // 非空时dout就是队首数据
    assign m_axis_tdata = fifo_dout;
    
    // 每个32字的4个字节全部有效
    assign m_axis_tkeep = 4'b1111;
    
    // 复位释放且FIFO非空时，提供有效数据
    assign m_axis_tvalid = rst_n && !fifo_empty;
    
    // 当握手成功时，FIFO出第一个字
    assign fifo_rd_en = m_axis_tvalid && m_axis_tready;
    
    // 第25个字是第一帧中的最后一个字
    assign m_axis_tlast = m_axis_tvalid
                        && (word_count == 5'd24);
                        
    // 最后一个字握手成功，表示本帧发送完毕
    assign stream_done = fifo_rd_en && m_axis_tlast;
    
    // 按成功传输的字数计算
    always @(posedge axis_clk or negedge rst_n) begin
        if (!rst_n) begin
            word_count <=5'd0;
        end
        else if (fifo_rd_en) begin
            if (word_count == 5'd24)
                word_count <= 5'd0;
            else
                word_count <= word_count + 5'd1;
        end
    end
endmodule
