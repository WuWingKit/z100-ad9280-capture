`timescale 1ns / 1ps

// Simulation only: real FIFO/DMA/interconnect RTL and the official PS7 VIP.
// PS VIP tasks replace ARM software; DDR is the VIP's behavioral memory.
`define PS_VIP dut.capture_bd_i.processing_system7_0.inst
`define BD dut.capture_bd_i

module tb_capture_bd;
    reg adc_clk = 1'b0;
    reg ps_clk = 1'b0;
    reg rst_n = 1'b0;
    reg ps_reset_n = 1'b0;
    reg start_pulse = 1'b0;
    reg [7:0] adc_data = 8'd0;
    reg adc_valid = 1'b0;
    wire busy, done, overflow, stream_done;
    wire [6:0] sample_count;
    wire ps_clk_pin, ps_porb_pin, ps_srstb_pin;
    assign ps_clk_pin = ps_clk;
    assign ps_porb_pin = ps_reset_n;
    assign ps_srstb_pin = ps_reset_n;

    // 10 MHz simulated ADC and approximately 33.333 MHz PS reference.
    always #50 adc_clk = ~adc_clk;
    always #15 ps_clk = ~ps_clk;

    capture_bd_wrapper dut (
        .FIXED_IO_0_ps_clk(ps_clk_pin),
        .FIXED_IO_0_ps_porb(ps_porb_pin),
        .FIXED_IO_0_ps_srstb(ps_srstb_pin),
        .adc_clk_0(adc_clk), .rst_n_0(rst_n),
        .start_pulse_0(start_pulse), .adc_data_0(adc_data),
        .adc_valid_0(adc_valid), .busy_0(busy), .done_0(done),
        .overflow_0(overflow), .sample_count_0(sample_count),
        .stream_done_0(stream_done)
    );
    // External DDR pins are intentionally open: PS VIP models memory internally.

    localparam [31:0] DMA_BASE = 32'h4040_0000;
    localparam [31:0] DDR_BASE = 32'h0100_0000;
    integer frame_id = 0;
    integer axis_words = 0;
    integer last_count = 0;
    integer capture_done_count = 0;
    integer stall_cycles = 0;
    integer checks = 0;
    integer i, lane, frame_index, poll_count, report_file;
    reg [7:0] expected_byte;
    reg [31:0] status, expected_word, buffer_addr;
    reg [1023:0] memory_data;
    reg stalled = 1'b0;
    reg [31:0] held_data;
    reg [3:0] held_keep;
    reg held_last;

    task check;
        input condition;
        input [511:0] description;
        begin
            checks = checks + 1;
            if (condition !== 1'b1) begin
                $display("FAIL frame=%0d time=%0t: %0s", frame_id, $time, description);
                $fatal(1, "Self-check failed");
            end
        end
    endtask

    // GP0 accesses pass through the actual AXI interconnect to DMA registers.
    task dma_write;
        input [31:0] offset;
        input [31:0] value;
        reg [1:0] response;
        begin
            `PS_VIP.write_data(DMA_BASE + offset, 4, value, response);
            check(response == 2'b00, "DMA register write response");
        end
    endtask

    task dma_read;
        input [31:0] offset;
        output [31:0] value;
        reg [2047:0] data;
        reg [1:0] response;
        begin
            `PS_VIP.read_data(DMA_BASE + offset, 4, data, response);
            check(response == 2'b00, "DMA register read response");
            value = data[31:0];
        end
    endtask

    task start_dma;
        input [31:0] address;
        begin
            dma_write(32'h34, 32'h0000_1000); // DMASR: clear old IOC flag
            dma_write(32'h30, 32'h0000_0001); // DMACR: run S2MM
            dma_write(32'h48, address);       // destination address
            dma_write(32'h58, 32'd100);       // length in BYTES; write last
        end
    endtask

    // Observe the actual AXIS handshake; check ordering, TLAST and stalled data.
    always @(posedge `BD.processing_system7_0_FCLK_CLK0) begin
        if (!`BD.proc_sys_reset_0_peripheral_aresetn) begin
            stalled = 1'b0;
        end else begin
            if (stalled) begin
                check(`BD.fifo_to_axis_0.m_axis_tvalid === 1'b1,
                      "TVALID must remain asserted during stall");
                check({`BD.fifo_to_axis_0.m_axis_tlast,
                       `BD.fifo_to_axis_0.m_axis_tkeep,
                       `BD.fifo_to_axis_0.m_axis_tdata} ===
                      {held_last, held_keep, held_data},
                      "AXIS data and sidebands must remain stable during stall");
            end
            stalled = `BD.fifo_to_axis_0.m_axis_tvalid &&
                     !`BD.fifo_to_axis_0.m_axis_tready;
            if (stalled) begin
                held_data = `BD.fifo_to_axis_0.m_axis_tdata;
                held_keep = `BD.fifo_to_axis_0.m_axis_tkeep;
                held_last = `BD.fifo_to_axis_0.m_axis_tlast;
                stall_cycles = stall_cycles + 1;
            end
            if (`BD.fifo_to_axis_0.m_axis_tvalid &&
                `BD.fifo_to_axis_0.m_axis_tready) begin
                check(axis_words < 25, "No extra word in frame");
                expected_word = 0;
                for (lane = 0; lane < 4; lane = lane + 1)
                    expected_word[lane*8 +: 8] = frame_id*100 + axis_words*4 + lane;
                check(`BD.fifo_to_axis_0.m_axis_tdata === expected_word,
                      "Packed AXIS word and byte order");
                check(`BD.fifo_to_axis_0.m_axis_tkeep === 4'b1111,
                      "All four byte lanes valid");
                check(`BD.fifo_to_axis_0.m_axis_tlast === (axis_words == 24),
                      "TLAST only on word 25");
                check(stream_done === (axis_words == 24),
                      "stream_done on final handshake");
                if (`BD.fifo_to_axis_0.m_axis_tlast) last_count = last_count + 1;
                axis_words = axis_words + 1;
            end
        end
    end

    always @(posedge adc_clk) begin
        if (done) capture_done_count = capture_done_count + 1;
        if (overflow === 1'b1) $fatal(1, "Unexpected ADC FIFO overflow");
    end

    initial begin
        #2000000;
        $fatal(1, "Simulation timed out: check reset, DMA configuration and clocks");
    end

    initial begin
        #1000;
        ps_reset_n = 1'b1;
        #1000;
        `PS_VIP.fpga_soft_reset(32'h0000_0000);
        @(negedge adc_clk);
        rst_n = 1'b1;
        poll_count = 0;
        while ((`BD.proc_sys_reset_0_peripheral_aresetn !== 1'b1) &&
               poll_count < 2000) begin
            @(posedge `BD.processing_system7_0_FCLK_CLK0);
            poll_count = poll_count + 1;
        end
        check(`BD.proc_sys_reset_0_peripheral_aresetn === 1'b1,
              "PSR releases read-side reset within 20 us");
        $display("RESET_READY time=%0t", $time);
        repeat (100) @(posedge adc_clk); // FIFO/reset recovery, no early writes

        // Reset DMA once, then run two frames without resetting the data path.
        dma_write(32'h30, 32'h0000_0004);
        status = 32'h4;
        poll_count = 0;
        while (status[2] && poll_count < 100) begin
            dma_read(32'h30, status);
            poll_count = poll_count + 1;
        end
        check(!status[2], "DMA reset completes");

        for (frame_index = 0; frame_index < 2; frame_index = frame_index + 1) begin
            frame_id = frame_index;
            axis_words = 0;
            last_count = 0;
            stall_cycles = 0;
            buffer_addr = DDR_BASE + frame_index*32'h1000;
            // Sentinel protects the 28 bytes immediately after the 100-byte frame.
            `PS_VIP.write_mem({128{8'hA5}}, buffer_addr, 128);
            if (frame_index == 1) start_dma(buffer_addr);

            @(negedge adc_clk);
            start_pulse = 1'b1;
            @(negedge adc_clk);
            start_pulse = 1'b0;
            for (i = 0; i < 100; i = i + 1) begin
                // Frame 2 includes invalid cycles: only valid samples may count.
                if ((frame_index == 1) && (i % 7 == 0)) begin
                    adc_valid = 1'b0;
                    repeat (2) @(negedge adc_clk);
                end
                adc_data = frame_index*100 + i;
                adc_valid = 1'b1;
                @(negedge adc_clk);
            end
            adc_valid = 1'b0;
            check(sample_count === 7'd100 && busy === 1'b0,
                  "Exactly 100 valid ADC samples captured");

            // Frame 1 deliberately starts DMA late to exercise real backpressure.
            if (frame_index == 0) begin
                repeat (20) @(negedge adc_clk);
                start_dma(buffer_addr);
            end

            status = 0;
            poll_count = 0;
            while (!status[12] && poll_count < 200) begin
                dma_read(32'h34, status);
                check((status & 32'h0000_4770) == 0, "DMA reports no transfer error");
                poll_count = poll_count + 1;
                #100;
            end
            check(status[12] === 1'b1, "DMA IOC confirms memory transfer completion");
            repeat (5) @(negedge adc_clk);
            check(axis_words == 25 && last_count == 1, "25 handshakes, one TLAST");
            check(capture_done_count == frame_index + 1, "One capture done per frame");
            if (frame_index == 0) check(stall_cycles > 0, "DMA delay exercises backpressure");

            `PS_VIP.read_mem(buffer_addr, 128, memory_data);
            for (i = 0; i < 100; i = i + 1) begin
                expected_byte = frame_index*100 + i;
                check(memory_data[i*8 +: 8] === expected_byte,
                      "DDR byte matches ADC sample");
            end
            for (i = 100; i < 128; i = i + 1)
                check(memory_data[i*8 +: 8] === 8'hA5, "No write beyond 100-byte buffer");
            $display("FRAME_PASS frame=%0d samples=100 axis_words=%0d DDR_bytes=100 stalls=%0d",
                     frame_index, axis_words, stall_cycles);
        end
        report_file = $fopen("capture_sim_result.txt", "w");
        $fdisplay(report_file, "PASS frames=2 samples=200 axis_words=50 DDR_bytes=200 checks=%0d", checks);
        $fclose(report_file);
        $display("CAPTURE_BD_SIM_PASS checks=%0d", checks);
        $finish;
    end
endmodule

`undef PS_VIP
`undef BD
