`timescale 1ns / 1ps
// GPIO0 command/status; GPIO1 phase increment/config. Config must be held
// stable before START and until DMA acknowledgement. No hardware auto-start.
// ===== 1. 模块接口：控制域、采样域、CORDIC 和 FIFO =====
// axis_clk：100 MHz AXI 域；adc_clk：10 MHz 采样域；rst_n：低有效总复位。
// command 来自控制 GPIO：bit0 START toggle，bit1 RAMP，bit2 软复位，
// bit3 DMA 已准备，bit4 DMA 完成 ACK toggle。toggle 表示翻转一次触发一次。
// phase_word 是频率控制字 K；config_word 打包 N、delay、amplitude。
// 配置必须先稳定、再 START，并保持至本帧完成；详见 CODE_GUIDE.md。
module measurement_core
    (input wire axis_clk, adc_clk, rst_n,
     input wire [31 : 0] command, phase_word, config_word,
     input wire [7 : 0] adc_raw,
     input wire adc_otr, fifo_full, stream_done,
     input wire [31 : 0] cordic_data,
     input wire cordic_valid,
     output wire [15 : 0] phase_data,
     output wire phase_valid, cordic_reset_n,
     output reg [7 : 0] dac_data,
     output wire [31 : 0] fifo_din,
     output wire fifo_wr_en, fifo_reset, axis_reset_n,
     output wire [12 : 0] frame_samples_axis,
     output wire [31 : 0] status);
    // ===== 2. 状态与寄存器 =====
    // IDLE 空闲；PREP 复位发波流水线；WAIT_ZERO 等待第一笔有效正弦；
    // DELAY 补偿链路延迟；CAPTURE 接收 N 点；WAIT_DMA 等 PS 确认 DDR 完成。
    // ERROR 停止本帧，必须由 PS 同时复位 DMA/FIFO 后恢复。
    localparam IDLE = 0, PREP = 1, WAIT_ZERO = 2, DELAY = 3, CAPTURE = 4, WAIT_DMA = 5, ERROR = 6;
    wire logic_rst_n = rst_n && !command[2];
    // ASYNC_REG 提示布局工具：以下是跨时钟域同步链。
    // ar/xr 负责两域复位同步释放；req/ack 同步触发翻转；cfg/k 是稳定参数总线。
    // 两级寄存器本身不保证任意多位变化的原子性，PS 的稳定总线协议不可省略。
    (* ASYNC_REG = "TRUE" *) reg [1 : 0] ar, xr, req_sync, ack_sync;
    (* ASYNC_REG = "TRUE" *) reg [31 : 0] cfg_meta, cfg_sync, k_meta, k_sync;
    (* ASYNC_REG = "TRUE" *) reg [1 : 0] mode_sync, armed_sync;
    reg [3 : 0] state;
    reg req_seen, ack_seen, test_mode, rejected, overflow, otr, frame_done;
    reg [31 : 0] phase, increment;
    reg [12 : 0] length, count;
    reg [7 : 0] delay_count, delay_cfg;
    reg [6 : 0] amplitude;
    reg [2 : 0] prep_count;
    reg [2 : 0] cordic_warm;
    reg [23 : 0] pack;
    reg wave_started, heartbeat;
    reg [15 : 0] heartbeat_count;
    // ===== 3. 组合运算：取样来源、定点正弦、DA 码值 =====
    // RAMP 取 count 的低 8 位，在不依赖外部 ADC 的情况下检查数据路径。
    // N 必须是 256..4096 中的 2 的幂，因此也必然是 4 的倍数。
    wire adc_reset_n = ar[1];
    wire active_wave = !test_mode && state >= WAIT_ZERO && state <= WAIT_DMA;
    wire[7 : 0] sample = test_mode ? count[7 : 0] : adc_raw;
    wire valid_length = cfg_sync[12 : 0] >= 256 && cfg_sync[12 : 0] <= 4096 &&
                        ((cfg_sync[12 : 0] & (cfg_sync[12 : 0] - 1'b1)) == 0);
    // CORDIC 输出高 16 位为正弦，低 16 位为余弦；正弦采用 Q2.14。
    // 有符号乘法后算术右移 14 位还原幅度码；128 是 DAC 名义中点。
    // 减号补偿配套 DA 模块反相，实际电压和增益仍需实测。
    wire signed [15 : 0] sine = cordic_data[31 : 16];
    wire signed [23 : 0] product = sine * $signed({1'b0, amplitude});
    wire signed [24 : 0] dac_code = 25'sd128 - (product >>> 14);
    // ===== 4. 连续信号与 FIFO 写条件 =====
    // phase[31:18] 对应完整相位的有符号高 14 位，再符号扩展成 Q3.13 π单位。
    // CORDIC NonBlocking 没有 TREADY；发波不跟随 DMA/FIFO 背压暂停。
    // cordic_warm 留出复位释放后的启动时间，确保第一笔相位 0 不被跳过。
    assign phase_data = {{2{phase[31]}}, phase[31 : 18]}; // Q3.13 scaled radians (pi units)
    assign phase_valid = active_wave && (&cordic_warm);
    assign cordic_reset_n = adc_reset_n && state != PREP;
    // 本拍是第 4 点时，{当前点, 前三点} 同时被 FIFO 在时钟沿接收。
    // 首点占最低字节，四点组成一个 32 位字：{s3,s2,s1,s0}。
    // 写使能是组合条件；时序块中清 pack 不影响本沿已被 FIFO 接收的数据。
    assign fifo_din = {sample, pack};
    assign fifo_wr_en =
               adc_reset_n && state == CAPTURE && count < length && !fifo_full && count[1 : 0] == 3;
    assign fifo_reset = !logic_rst_n;
    assign axis_reset_n = xr[1];
    // ===== 5. 复位：异步断言，两级同步释放 =====
    // 每个时钟域分别同步；不要把 AXI 域复位释放直接当成 ADC 域安全释放。
    always @(posedge adc_clk or negedge logic_rst_n)
        if (!logic_rst_n)
            ar <= 0;
        else
            ar <= {ar[0], 1'b1};
    always @(posedge axis_clk or negedge logic_rst_n)
        if (!logic_rst_n)
            xr <= 0;
        else
            xr <= {xr[0], 1'b1};
    // ===== 6. ADC 时钟域主逻辑 =====
    // 非阻塞赋值 <= 在时钟沿后统一更新；条件判断读取的是本沿之前的旧值。
    always @(posedge adc_clk or negedge adc_reset_n) begin
        if (!adc_reset_n) begin
            req_sync <= 0;
            ack_sync <= 0;
            mode_sync <= 0;
            armed_sync <= 0;
            cfg_meta <= 0;
            cfg_sync <= 0;
            k_meta <= 0;
            k_sync <= 0;
            state <= IDLE;
            req_seen <= 0;
            ack_seen <= 0;
            test_mode <= 0;
            rejected <= 0;
            overflow <= 0;
            otr <= 0;
            frame_done <= 0;
            phase <= 0;
            increment <= 0;
            length <= 4096;
            count <= 0;
            delay_cfg <= 0;
            delay_count <= 0;
            amplitude <= 0;
            prep_count <= 0;
            cordic_warm <= 0;
            pack <= 0;
            wave_started <= 0;
            dac_data <= 128;
            heartbeat <= 0;
            heartbeat_count <= 0;
        end else begin
            // 6.1 连续同步 PS 控制和参数；参数仅在接受 START 时锁存到工作寄存器。
            req_sync <= {req_sync[0], command[0]};
            ack_sync <= {ack_sync[0], command[4]};
            mode_sync <= {mode_sync[0], command[1]};
            armed_sync <= {armed_sync[0], command[3]};
            cfg_meta <= config_word;
            cfg_sync <= cfg_meta;
            k_meta <= phase_word;
            k_sync <= k_meta;
            heartbeat_count <= heartbeat_count + 1'b1;
            if (&heartbeat_count)
                heartbeat <= ~heartbeat;
            // 6.2 波形发生器：连续累加相位，等待 CORDIC 有效后更新 DA。
            // IDLE/错误/复位时保持 128；它是名义零电压码，不能用 0 码代替。
            if (!active_wave)
                cordic_warm <= 0;
            else
                cordic_warm <= {cordic_warm[1 : 0], 1'b1};
            if (phase_valid)
                phase <= phase + increment;
            if (active_wave && cordic_valid) begin
                if (dac_code < 0)
                    dac_data <= 0;
                else if (dac_code > 255)
                    dac_data <= 255;
                else
                    dac_data <= dac_code[7 : 0];
            end else if (!active_wave)
                dac_data <= 128;
            // 6.3 优先处理新的 START 翻转。
            // 忙时不重启本帧，只记录 rejected；空闲时若参数/DMA 状态非法则进入 ERROR。
            if (req_sync[1] != req_seen) begin
                req_seen <= req_sync[1];
                if (state != IDLE || !armed_sync[1] || !valid_length || k_sync == 0 ||
                    k_sync >= 32'h80000000) begin
                    rejected <= 1;
                    if (state == IDLE)
                        state <= ERROR;
                end else begin
                    increment <= k_sync;
                    length <= cfg_sync[12 : 0];
                    delay_cfg <= cfg_sync[20 : 13];
                    amplitude <= cfg_sync[27 : 21];
                    test_mode <= mode_sync[1];
                    rejected <= 0;
                    overflow <= 0;
                    otr <= 0;
                    frame_done <= 0;
                    count <= 0;
                    pack <= 0;
                    phase <= 0;
                    prep_count <= 0;
                    wave_started <= 0;
                    ack_seen <= ack_sync[1];
                    state <= PREP;
                end
            end else
                case (state)
                        // 6.4 PREP 保持四拍复位和零相位；RAMP 不需要经过 CORDIC。
                    PREP: begin
                        phase <= 0;
                        if (prep_count == 3) begin
                            if (test_mode) begin
                                state <= DELAY;
                                delay_count <= 0;
                            end else
                                state <= WAIT_ZERO;
                        end else
                            prep_count <= prep_count + 1'b1;
                    end
                        // 6.5 以 CORDIC 首个有效输出作为数字发波起点，再等待可配置延迟。
                        // 实现中首个采样沿位于该 DAC 更新沿之后 delay+2 个采样周期。
                        // delay=3 只在数字模型中验证，不保证真实模拟过零点。
                    WAIT_ZERO:
                        if (cordic_valid) begin
                            wave_started <= 1;
                            delay_count <= delay_cfg;
                            state <= DELAY;
                        end
                    DELAY:
                        if (delay_count == 0)
                            state <= CAPTURE;
                        else
                            delay_count <= delay_count - 1'b1;
                        // 6.6 连续接受 N 个采样点，记录过量程；满 FIFO 直接中止而非静默丢点。
                    CAPTURE: begin
                        if (adc_otr && !test_mode)
                            otr <= 1;
                        if (fifo_full) begin
                            overflow <= 1;
                            state <= ERROR;
                        end else if (count < length) begin
                            // count[1:0] 相当于 count % 4；只缓存前三点，第四点直接拼接写 FIFO。
                            case (count[1 : 0])
                                0:
                                    pack[7 : 0] <= sample;
                                1:
                                    pack[15 : 8] <= sample;
                                2:
                                    pack[23 : 16] <= sample;
                                3:
                                    pack <= 0;
                            endcase
                            count <= count + 1'b1;
                            if (count == length - 1'b1) begin
                                frame_done <= 1;
                                state <= WAIT_DMA;
                            end
                        end
                    end
                        // 6.7 capture_done/TLAST 都不能证明 DDR 写完。
                        // 只有 PS 读到 DMA IOC 且不忙后才翻转 ACK，本状态才恢复空闲。
                    WAIT_DMA:
                        if (ack_sync[1] != ack_seen) begin
                            ack_seen <= ack_sync[1];
                            state <= IDLE;
                        end
                endcase
        end
    end
    // ===== 7. 状态回传到 AXI/PS 域 =====
    // 先在源域寄存组合状态以减少毛刺，再同步到 axis_clk。
    // count/state 的中间值仅用于诊断；PS 等整帧完成并额外等待后再读计数。
    // nmeta/nsync 回传帧长，在第一笔 FIFO 数据到达前已经稳定。
    (* ASYNC_REG = "TRUE" *) reg [31 : 0] smeta, ssync;
    (* ASYNC_REG = "TRUE" *) reg [12 : 0] nmeta, nsync;
    reg streamed, req_axis;
    wire [31 : 0] adc_status_next = {4'd0,         (state == WAIT_DMA),
                                     wave_started, 1'b0,
                                     state,        count,
                                     otr,          adc_reset_n,
                                     heartbeat,    rejected,
                                     req_seen,     frame_done,
                                     overflow,     (state != IDLE && state != ERROR)};
    reg [31 : 0] adc_status;
    always @(posedge adc_clk or negedge adc_reset_n)
        if (!adc_reset_n)
            adc_status <= 0;
        else
            adc_status <= adc_status_next;
    always @(posedge axis_clk or negedge axis_reset_n) begin
        if (!axis_reset_n) begin
            smeta <= 0;
            ssync <= 0;
            nmeta <= 4096;
            nsync <= 4096;
            streamed <= 0;
            req_axis <= 0;
        end else begin
            smeta <= adc_status;
            ssync <= smeta;
            nmeta <= length;
            nsync <= nmeta;
            req_axis <= command[0];
            if (command[0] != req_axis)
                streamed <= 0;
            else if (stream_done)
                streamed <= 1;
        end
    end
    // streamed 在 AXI 域锁存最后一笔握手；新 START 翻转时清除。
    // 状态 bit25 由本域结果覆盖，其余位来自 ADC 域稳定状态快照。
    assign frame_samples_axis = nsync;
    assign status = (ssync & 32'hfdffffff) | ({31'd0, streamed} << 25);
endmodule
