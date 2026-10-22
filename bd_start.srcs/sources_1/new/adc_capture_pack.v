`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: SWU
// Engineer: Wu Wing_Kit
// 
// Create Date: 2026/10/21 15:11:23
// Design Name: 
// Module Name: adc_capture_pack
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


module adc_capture_pack(
    // 启动与采集：决定开始和接受点的时间
    input   wire        adc_clk,        // ADC数据采集时钟
    input   wire        rst_n,          // 低有效位复位
    input   wire        start_pulse,    // 一个时钟周期的启动脉冲
    
    input   wire [7:0]  adc_data,       //  ADC输出的8位数据
    input   wire        adc_valid,      // 当前采样点是否有效
    
    // FIFO连接：接受满标志和输出32位数据以及写使能
    input   wire        fifo_full,      // FIFO满标志
    output  wire [31:0] fifo_din,       // 打包后送给FIFO的数据
    output  wire        fifo_wr_en,     // FIFO写使能
    
    // 状态观察：在仿真波形中观察采集进度
    output  reg         busy,           // 正在采集
    output  reg         done,           // 本次采集完成脉冲
    output  reg         overflow,       // FIFO满导致本次采集失败
    output  reg [6:0]   sample_count    // 已采集点数：0~100   
    );
    
    // 暂存前三个采样点，共24位
    reg [23:0]          pack_buffer;
    // 当前正在接收一组4点中的第几个点：0、1、2、3
    reg [1:0]           byte_count;
    
    always @(posedge adc_clk or negedge rst_n) begin
        if (!rst_n) begin
            // 复位
            busy            <= 1'b0;
            done            <= 1'b0;
            overflow        <= 1'b0;
            sample_count    <= 7'd0;
            
            pack_buffer     <= 24'd0;
            byte_count      <= 2'd0;
        end
        
        else begin
            // 默认清零完成状态
            done            <= 1'b0;
            
            // 空闲时启动
            if (!busy) begin
                // 接收到开始采集信号后，开始本次采集前的初始化
                if (start_pulse) begin
                    busy            <= 1'b1;        // 忙碌模式
                    overflow        <= 1'b0; 
                    sample_count    <= 7'd0;
                    
                    pack_buffer     <= 24'd0;
                    byte_count      <= 2'd0;
                end
            end
            
            else begin
                // 当数据有效的时候才开始接收第一个采样点
                if (adc_valid) begin
                    if (fifo_full) begin
                        // FIFO满，停止采集并记录失败
                        overflow    <= 1'b1;
                        busy        <= 1'b0;
                    end
                    else begin
                        // 依据组内位置保存前三个点
                        case (byte_count)
                            2'd0: pack_buffer[7:0]      <= adc_data;
                            2'd1: pack_buffer[15:8]     <= adc_data;
                            2'd2: pack_buffer[23:16]    <= adc_data;
                            
                            // 第四点直接参与FIFO输入的拼接
                            // 本拍写入后，清空缓存，准备下一组
                            2'd3: pack_buffer            <= 24'd0;
                        endcase
                        
                        // 2位计数器：0-1-2-3-0
                        byte_count <= byte_count + 2'd1;
                        
                        // 整数帧增加有效采样点数
                        sample_count <= sample_count + 7'd1;
                        
                        // 更新前99时说明本拍接收的是第100点
                        if (sample_count == 7'd99) begin
                            busy <= 1'b0;   // 恢复空闲状态
                            done <= 1'b1;   // 进入完成状态
                        end
                    end
                end
            end
        end
    end
    
    // 第四点放在高8位，缓存中的前3点放在低24位
    assign fifo_din = {adc_data, pack_buffer};
    
    // 只有第4个有效点到来时，才写入一个32位字
    assign fifo_wr_en = rst_n
                     && busy
                     && adc_valid
                     && !fifo_full
                     && (byte_count == 2'd3);
endmodule
