`timescale 1ns / 1ps
// PS command crosses into ADC domain as a toggle, never a short pulse.
// command[0]: request toggle; command[1]: 0=real ADC, 1=counter self-test.
module adc_board_control (
    input wire axis_clk,
    input wire adc_clk,
    input wire rst_n,
    input wire [31:0] command,
    input wire [7:0] adc_raw,
    input wire adc_otr,
    input wire capture_busy,
    input wire capture_done,
    input wire capture_overflow,
    input wire [6:0] sample_count,
    output wire capture_rst_n,
    output reg start_pulse,
    output wire [7:0] adc_sample,
    output wire [31:0] status
);
    (* ASYNC_REG = "TRUE" *) reg [1:0] adc_reset_sync;
    (* ASYNC_REG = "TRUE" *) reg [1:0] request_sync;
    (* ASYNC_REG = "TRUE" *) reg [1:0] mode_sync;
    reg request_seen;
    reg done_latched;
    reg command_rejected;
    reg otr_latched;
    reg heartbeat;
    reg [15:0] heartbeat_count;
    (* ASYNC_REG = "TRUE" *) reg [6:0] status_meta, status_sync;
    reg [6:0] count_meta, count_sync;

    // Async assertion, ADC-clock synchronous release.
    always @(posedge adc_clk or negedge rst_n)
        if (!rst_n) adc_reset_sync <= 2'b00;
        else adc_reset_sync <= {adc_reset_sync[0], 1'b1};
    assign capture_rst_n = adc_reset_sync[1];

    always @(posedge adc_clk or negedge capture_rst_n) begin
        if (!capture_rst_n) begin
            request_sync <= 0;
            mode_sync <= 0;
            request_seen <= 0;
            start_pulse <= 0;
            done_latched <= 0;
            command_rejected <= 0;
            otr_latched <= 0;
            heartbeat_count <= 0;
            heartbeat <= 0;
        end else begin
            request_sync <= {request_sync[0], command[0]};
            mode_sync <= {mode_sync[0], command[1]};
            start_pulse <= 0;
            heartbeat_count <= heartbeat_count + 1'b1;
            if (&heartbeat_count) heartbeat <= ~heartbeat;
            if (capture_done) done_latched <= 1;
            if (capture_busy && adc_otr && !mode_sync[1]) otr_latched <= 1;
            if (request_sync[1] != request_seen) begin
                request_seen <= request_sync[1];
                if (!capture_busy && !start_pulse) begin
                    start_pulse <= 1;
                    done_latched <= 0;
                    command_rejected <= 0;
                    otr_latched <= 0;
                end else command_rejected <= 1;
            end
        end
    end

    // Counter self-test uses the same packing/FIFO/DMA path as the physical ADC.
    // Software sets mode first and holds it stable throughout the capture.
    assign adc_sample = mode_sync[1] ? {1'b0, sample_count} : adc_raw;

    always @(posedge axis_clk or negedge rst_n) begin
        if (!rst_n) begin
            status_meta <= 0;
            status_sync <= 0;
            count_meta <= 0;
            count_sync <= 0;
        end else begin
            status_meta <= {otr_latched, heartbeat, command_rejected, request_seen,
                            done_latched, capture_overflow, capture_busy};
            status_sync <= status_meta;
            count_meta <= sample_count;
            count_sync <= count_meta;
        end
    end
    // count_sync is diagnostic only while busy; read it after completed status
    // and a settling interval, when the multi-bit source is held stable.
    // [0]busy [1]overflow [2]done [3]request_ack [4]rejected [5]heartbeat
    // [6]ADC reset released, [14:8]sample_count.
    (* ASYNC_REG = "TRUE" *) reg ready_meta, ready_sync;
    always @(posedge axis_clk or negedge rst_n)
        if (!rst_n) begin ready_meta <= 0; ready_sync <= 0; end
        else begin ready_meta <= capture_rst_n; ready_sync <= ready_meta; end
    assign status = {17'd0, count_sync, status_sync[6], ready_sync, status_sync[5:0]};
endmodule
