// SPI-only simulation; model represents the PS slave, not ARM execution.
`timescale 1ns / 1ps
module tb_spi_capture;
    reg axis_clk = 0, spi_clk16 = 0, rst_n = 0;
    always
        #5 axis_clk = ~axis_clk;
    always
        #31.25 spi_clk16 = ~spi_clk16;
    reg [31 : 0] command = 0, k = 42949673, cfg = 0;
    reg force_full = 0, ready = 0;
    wire [31 : 0] din, dout, tdata, status;
    wire full, empty, wr_en, rd_en, fifo_reset, axis_reset_n, valid, last, stream_done;
    wire [12 : 0] length;
    wire [3 : 0] keep;
    integer checks = 0, words = 0, sampled = 0, lasts = 0, cycles = 0, stalls = 0, job = 0, n = 0,
            i, lane, fd;
    reg [7 : 0] expected[0 : 4095], first_frame[0 : 4095];
    reg checking = 0, held = 0;
    reg [36 : 0] held_data;

    wire spi_sclk, spi_cs_n, spi_mosi, spi_miso;
    integer source_index=0, source_frequency=10000;
    reg [7:0] source_byte;
    reg signed [7:0] source_lut[0:255];
    reg [31:0] source_phase;
    initial $readmemh("sine_lut.hex", source_lut);
    always @* begin
        source_phase = source_index * k;
        source_byte = core.test_mode ? source_index[7:0] :
            128 + $signed(source_lut[source_phase[31:24]]) * 24 / 127;
    end
    spi_slave_model ps_spi_model(spi_sclk, spi_cs_n, spi_mosi, source_byte, spi_miso, core.spi_reset_n);
    always @(posedge spi_cs_n) if (ps_spi_model.bits == 8) source_index=source_index+1;

    spi_capture_core core(.axis_clk(axis_clk),.spi_clk16(spi_clk16),.rst_n(rst_n),
        .command(command),.phase_word(k),.config_word(cfg),.fifo_full(full|force_full),
        .stream_done(stream_done),.spi_miso(spi_miso),.spi_sclk(spi_sclk),
        .spi_cs_n(spi_cs_n),.spi_mosi(spi_mosi),.fifo_din(din),.fifo_wr_en(wr_en),
        .fifo_reset(fifo_reset),.axis_reset_n(axis_reset_n),.frame_samples_axis(length),.status(status));
    sim_fifo fifo(.rst(fifo_reset),
                  .wr_clk(spi_clk16),
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
    always @(posedge spi_clk16) begin
        if (checking && core.state == 4 && core.sample_valid && !full && !force_full && core.count < core.length) begin
            expected[sampled] = ps_spi_model.latched;
            check(core.sample === ps_spi_model.latched, "SPI reconstructed byte equals PS model queued byte");
            sampled = sampled + 1;
        end
    end
    always @(negedge axis_clk) begin
        cycles = cycles + 1;
        ready = (cycles % 13 >= 4);
    end
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
    task run_frame;
        input integer freq, num, delay, ramp;
        begin
            @(negedge axis_clk);
            n = num;
            sampled = 0;
            source_index = 0;
            source_frequency = freq;
            words = 0;
            lasts = 0;
            checking = 1;
            k = $rtoi(freq / 1000000.0 * 4294967296.0 + 0.5);
            cfg = num | (delay << 13) | (24 << 21);
            repeat (20) @(negedge spi_clk16)
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
    initial begin
        #1000;
        rst_n = 1;
        repeat (30) @(negedge spi_clk16)
            ;
        run_frame(100000, 8, 3, 0);
        run_frame(100000, 8, 3, 0);
        run_frame(250000, 8, 3, 0);
        run_frame(100000, 8, 0, 1);
        cfg = 300 | (24 << 21);
        repeat (20) @(negedge spi_clk16)
            ;
        command = (command ^ 1) | 8;
        wait(core.state == 6);
        check(core.rejected, "invalid frame length rejected");
        command = 4;
        repeat (20) @(negedge spi_clk16)
            ;
        command = 0;
        repeat (30) @(negedge spi_clk16)
            ;
        cfg = 8 | (24 << 21);
        repeat (20) @(negedge spi_clk16)
            ;
        command = 11;
        wait(core.state == 4);
        repeat (5) @(negedge spi_clk16)
            ;
        force_full = 1;
        repeat (3) @(negedge spi_clk16)
            ;
        check(core.overflow && core.state == 6, "full abort reports error");
        command = 4;
        repeat (20) @(negedge spi_clk16)
            ;
        force_full = 0;
        command = 0;
        repeat (30) @(negedge spi_clk16)
            ;
        run_frame(100000, 8, 0, 1);
        fd = $fopen("spi_capture_result.txt", "w");
        $fdisplay(
            fd,
            "PASS frames=%0d checks=%0d stalls=%0d invalid_config=PASS overflow_recovery=PASS",
            job, checks, stalls);
        $fclose(fd);
        $display("MEASUREMENT_SIM_PASS checks=%0d", checks);
        $finish;
    end
    initial begin
        #100000000;
        $fatal(1, "timeout");
    end
endmodule
