`timescale 1ns / 1ps
// ===== 1. FWFT FIFO → AXI4-Stream 适配器 =====
// 本模块只适用于 First Word Fall Through：非空时 dout 已是队首数据。
// Standard FIFO 有额外读延迟，不能直接套用此组合接口。
// 所有端口均属于 axis_clk；frame_samples 在整帧期间必须保持稳定。
module fifo_to_axis
    (input wire axis_clk, rst_n,
     input wire [31 : 0] fifo_dout,
     input wire fifo_empty,
     output wire fifo_rd_en,
     output wire [31 : 0] m_axis_tdata,
     output wire [3 : 0] m_axis_tkeep,
     output wire m_axis_tvalid,
     input wire m_axis_tready,
     output wire m_axis_tlast, stream_done,
     input wire [12 : 0] frame_samples);
    // ===== 2. 当前字序号与握手 =====
    // 4096 个字节最多 1024 个 32 位字，10 位计数器计 0..1023。
    // word_count 只在 TVALID && TREADY 时前进，停顿不消耗 FIFO 数据。
    reg [9 : 0] word_count;
    assign m_axis_tdata = fifo_dout;
    assign m_axis_tkeep = 4'hf;
    assign m_axis_tvalid = rst_n && !fifo_empty;
    assign fifo_rd_en = m_axis_tvalid && m_axis_tready;
    // ===== 3. 包尾与完成 =====
    // N/4 - 1 是最后一个字序号；等待 TREADY 时，TLAST 与 TDATA 一起保持。
    // stream_done 仅表示 AXIS 包尾已被下游接收，不能代替 DMA DDR 完成标志。
    assign m_axis_tlast = m_axis_tvalid && ({1'b0, word_count} == ((frame_samples >> 2) - 1'b1));
    assign stream_done = fifo_rd_en && m_axis_tlast;
    // ===== 4. 时序计数：仅按实际成功传输的字数计数 =====
    always @(posedge axis_clk or negedge rst_n)
        if (!rst_n)
            word_count <= 0;
        else if (fifo_rd_en) begin
            if (m_axis_tlast)
                word_count <= 0;
            else
                word_count <= word_count + 1'b1;
        end
endmodule
