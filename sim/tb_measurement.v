`timescale 1ns / 1ps
// ===== 1. 测试时钟、DUT和实际CORDIC/FIFO IP =====
module tb_measurement;
    reg axis_clk = 0, adc_clk = 0, rst_n = 0;
    always
        #5 axis_clk = ~axis_clk;
    always
        #50 adc_clk = ~adc_clk;
    reg [31 : 0] command = 0, k = 42949673, cfg = 0;
    reg [7 : 0] adc_raw = 128, analog_code = 128, p0 = 128, p1 = 128, p2 = 128;
    reg force_full = 0, ready = 0;
    wire [7 : 0] dac_data;
    wire [31 : 0] din, dout, tdata, status, cordic_data;
    wire full, empty, wr_en, rd_en, fifo_reset, axis_reset_n, valid, last, stream_done, phase_valid,
         cordic_valid, cordic_reset_n;
    wire [15 : 0] phase_data;
    wire [12 : 0] length;
    wire [3 : 0] keep;
    integer checks = 0, words = 0, sampled = 0, lasts = 0, cycles = 0, stalls = 0, job = 0, n = 0,
            i, lane, fd;
    reg [7 : 0] expected[0 : 4095], first_frame[0 : 4095];
    reg checking = 0, held = 0;
    reg [36 : 0] held_data;
    reg [15 : 0] phases[0 : 16383];
    integer pin = 0, pout = 0, sine_value, sine_expected, cordic_checks = 0;
    real angle;
    measurement_core core(axis_clk,
                          adc_clk,
                          rst_n,
                          command,
                          k,
                          cfg,
                          adc_raw,
                          1'b0,
                          full | force_full,
                          stream_done,
                          cordic_data,
                          cordic_valid,
                          phase_data,
                          phase_valid,
                          cordic_reset_n,
                          dac_data,
                          din,
                          wr_en,
                          fifo_reset,
                          axis_reset_n,
                          length,
                          status);
    sim_cordic cordic(.aclk(adc_clk),
                      .aresetn(cordic_reset_n),
                      .s_axis_phase_tvalid(phase_valid),
                      .s_axis_phase_tdata(phase_data),
                      .m_axis_dout_tvalid(cordic_valid),
                      .m_axis_dout_tdata(cordic_data));
    sim_fifo fifo(.rst(fifo_reset),
                  .wr_clk(adc_clk),
                  .rd_clk(axis_clk),
                  .din(din),
                  .wr_en(wr_en),
                  .rd_en(rd_en),
                  .dout(dout),
                  .full(full),
                  .empty(empty));
    fifo_to_axis axis(axis_clk,
                      axis_reset_n,
                      dout,
                      empty,
                      rd_en,
                      tdata,
                      keep,
                      valid,
                      ready,
                      last,
                      stream_done,
                      length);
    // Digital-only analog path model: DAC inversion, 2ns settling, 3-cycle ADC
    // pipeline + 25ns output delay. Does not model real analog filters/noise.
    always @(negedge adc_clk) begin
        analog_code <= #2 (256 - dac_data);
        p0 <= analog_code;
        p1 <= p0;
        p2 <= p1;
        adc_raw <= #25 p2;
    end
    // ===== 2. 断言工具：记录检查数，首个失败即停止仿真 =====
    task check;
        input ok;
        input [511 : 0] msg;
        begin
            checks = checks + 1;
            if (ok !== 1'b1) begin
                $display("FAIL job=%0d time=%0t %0s status=%08x", job, $time, msg, status);
                $fatal(1, "measurement test");
            end
        end
    endtask
    // ===== 3. CORDIC数值与采样记分牌：用数学sin核对IP，再记录采到的字节 =====
    always @(posedge adc_clk) begin
        if (!cordic_reset_n) begin
            pin = 0;
            pout = 0;
        end else begin
            if (phase_valid) begin
                phases[pin] = phase_data;
                pin = pin + 1;
            end
            if (cordic_valid) begin
                check(pout < pin, "CORDIC valid has a phase input");
                angle = $itor($signed(phases[pout])) * 3.141592653589793 / 8192.0;
                sine_expected = $rtoi($sin(angle) * 16384.0);
                sine_value = $signed(cordic_data[31 : 16]);

                check(sine_value - sine_expected < 8 && sine_expected - sine_value < 8,
                      "CORDIC phase/scaled-radian/output-lane correctness");
                pout = pout + 1;
                cordic_checks = cordic_checks + 1;
            end
        end
        if (checking && core.state == 4 && !full && !force_full && core.count < core.length) begin
            expected[sampled] = core.test_mode ? sampled[7 : 0] : adc_raw;
            sampled = sampled + 1;
        end
    end
    always @(negedge axis_clk) begin
        cycles = cycles + 1;
        ready = (cycles % 13 >= 4);
    end
    // ===== 4. AXIS记分牌：检查背压保持、字节顺序、TKEEP与TLAST =====
    always @(posedge axis_clk) begin
        if (!axis_reset_n)
            held = 0;
        else begin
            if (held)
                check(valid && {last, keep, tdata} === held_data, "AXIS stable under backpressure");
            held = valid && !ready;
            if (held) begin
                held_data = {last, keep, tdata};
                stalls = stalls + 1;
            end
            if (valid && ready && checking) begin
                check(words < n / 4, "no extra words");
                for (lane = 0; lane < 4; lane = lane + 1)
                    check(tdata[lane * 8 +: 8] === expected[words * 4 + lane],
                          "packed order matches captured sample");
                check(keep == 15 && last === (words == n / 4 - 1), "TKEEP and TLAST");
                if (last)
                    lasts = lasts + 1;
                words = words + 1;
            end
        end
    end
    // ===== 5. 单帧流程：配置、START、等待、比对、模拟PS确认DMA完成 =====
    task run_frame;
        input integer freq, num, delay, ramp;
        begin
            @(negedge axis_clk);
            n = num;
            sampled = 0;
            words = 0;
            lasts = 0;
            checking = 1;
            k = $rtoi(freq / 10000000.0 * 4294967296.0 + 0.5);
            cfg = num | (delay << 13) | (24 << 21);
            repeat (20) @(negedge adc_clk)
                ;
            command = (command & 17) | 8 | (ramp ? 2 : 0);
            command = command ^ 1;
            wait(core.state == 5);
            wait(words == n / 4);
            repeat (20) @(negedge axis_clk)
                ;
            check(sampled == n && lasts == 1, "exact sample/word/frame length");
            check(status[2] && status[25] && !status[1] && !status[4],
                  "capture complete plus streamed status");
            check(status[20 : 8] == n, "stable diagnostic count");
            if (job == 0) begin
                fd = $fopen("loopback_simulated.csv", "w");
                $fdisplay(fd, "sample_index,adc_code");
                for (i = 0; i < n; i = i + 1) begin
                    first_frame[i] = expected[i];
                    $fdisplay(fd, "%0d,%0d", i, expected[i]);
                end
                $fclose(fd);
                check(expected[0] >= 127 && expected[0] <= 129,
                      "calibrated first sample near zero in digital model");
                check(expected[3] > expected[0],
                      "first points follow rising sine, not pre-start idle plateau");
            end
            if (job == 1)
                for (i = 0; i < n; i = i + 1)
                    check(first_frame[i] === expected[i],
                          "repeat START exact phase/data repeatability");
            if (ramp)
                for (i = 0; i < n; i = i + 1)
                    check(expected[i] === i[7 : 0], "RAMP modulo256");
            checking = 0;
            command = (command ^ 16) & ~8;
            wait(core.state == 0);
            $display("FRAME_PASS job=%0d n=%0d freq=%0d first=%0d", job, n, freq, expected[0]);
            job = job + 1;
        end
    endtask
    // ===== 6. 测试场景：重复REAL、改变频率/长度、RAMP、非法配置、满FIFO及恢复 =====
    initial begin
        #1000;
        rst_n = 1;
        repeat (30) @(negedge adc_clk)
            ;
        run_frame(100000, 4096, 3, 0);
        run_frame(100000, 4096, 3, 0);
        run_frame(250000, 1024, 3, 0);
        run_frame(100000, 256, 0, 1);
        cfg = 300 | (24 << 21);
        repeat (20) @(negedge adc_clk)
            ;
        command = (command ^ 1) | 8;
        wait(core.state == 6);
        check(core.rejected, "invalid frame length rejected");
        command = 4;
        repeat (20) @(negedge adc_clk)
            ;
        command = 0;
        repeat (30) @(negedge adc_clk)
            ;
        cfg = 256 | (24 << 21);
        repeat (20) @(negedge adc_clk)
            ;
        command = 11;
        wait(core.state == 4);
        repeat (5) @(negedge adc_clk)
            ;
        force_full = 1;
        repeat (3) @(negedge adc_clk)
            ;
        check(core.overflow && core.state == 6, "full abort reports error");
        command = 4;
        repeat (20) @(negedge adc_clk)
            ;
        force_full = 0;
        command = 0;
        repeat (30) @(negedge adc_clk)
            ;
        run_frame(100000, 256, 0, 1);
        fd = $fopen("measurement_result.txt", "w");
        $fdisplay(
            fd,
            "PASS frames=%0d checks=%0d cordic_checks=%0d stalls=%0d invalid_config=PASS overflow_recovery=PASS",
            job, checks, cordic_checks, stalls);
        $fclose(fd);
        $display("MEASUREMENT_SIM_PASS checks=%0d", checks);
        $finish;
    end
    initial begin
        #3000000;
        $fatal(1, "timeout");
    end
endmodule
