`timescale 1ns/1ps
// GPIO control, MCU SYNC, complete-frame validation, then RAM -> FIFO -> DMA.
// Reject malformed frames BEFORE releasing any word to DMA.
// command bits: 0=request toggle, 1=prime-only, 2=soft reset,
//               3=DMA armed, 4=DDR-complete acknowledgement toggle.
// prime_wait_cycles / timeout_cycles are stable throughout a transaction.
module radar_frame_capture #(
    parameter integer SAMPLE_POINTS=512,
    parameter integer CAPTURE_CHIRPS=24,
    parameter integer PAYLOAD_CHIRPS=23,
    parameter integer SYNC_PULSE_CYCLES=1000,
    parameter integer DEFAULT_PRIME_CYCLES=5000000,
    parameter integer DEFAULT_TIMEOUT_CYCLES=20000000
)(
    input wire axis_clk,rst_n,
    input wire [31:0] command,prime_wait_cycles,timeout_cycles,
    input wire spi_sclk,spi_cs_n,spi_mosi,
    input wire mcu_ready,
    output reg radar_sync,
    input wire fifo_full,stream_done,
    output wire [31:0] fifo_din,
    output wire fifo_wr_en,fifo_reset,axis_reset_n,
    output wire [15:0] frame_bytes_axis,
    output wire [31:0] status
);
    localparam integer FRAME_BYTES=32+PAYLOAD_CHIRPS*SAMPLE_POINTS*4;
    localparam integer SPI_WORDS=FRAME_BYTES/2;
    localparam integer AXIS_WORDS=FRAME_BYTES/4;
    localparam IDLE=0,PRIME=1,RECEIVE=2,FETCH=3,PUT=4,DRAIN=5,WAIT_ACK=6,FAULT=7;
    wire logic_rst_n=rst_n && !command[2];
    (* ASYNC_REG="TRUE" *) reg [1:0] reset_release;
    wire reset_n=reset_release[1];
    always @(posedge axis_clk or negedge logic_rst_n)
        if(!logic_rst_n) reset_release<=0;else reset_release<={reset_release[0],1'b1};
    // READY is an asynchronous MCU GPIO, not an AXI-domain signal.
    (* ASYNC_REG="TRUE" *) reg [2:0] ready_pipe;
    reg ready_went_low;
    always @(posedge axis_clk or negedge reset_n)
        if(!reset_n)ready_pipe<=0;
        else ready_pipe<={ready_pipe[1:0],mcu_ready};
    assign axis_reset_n=reset_n;
    assign fifo_reset=!reset_n;
    assign frame_bytes_axis=FRAME_BYTES;

    wire [15:0] rx_word;
    wire rx_valid,rx_start,rx_end,rx_partial,rx_selected;
    spi_slave_rx16 u_spi_rx(axis_clk,reset_n,spi_sclk,spi_cs_n,spi_mosi,
        rx_word,rx_valid,rx_start,rx_end,rx_partial,rx_selected);
    reg [3:0] state;
    reg request_seen,ack_seen,primed,rejected,frame_valid,streamed;
    reg in_frame,header_ok,receive_error;
    reg [7:0] error_code;
    reg [15:0] received_words,header_xor,low_word;
    reg [15:0] header[0:14];
    reg [15:0] last_seq;
    reg have_seq;
    reg [31:0] timer,prime_limit,timeout_limit,pulse_remaining;
    reg [15:0] read_index;
    (* ram_style="block" *) reg [31:0] frame_ram[0:AXIS_WORDS-1];
    reg [31:0] ram_q;
    reg ram_write_enable;
    reg [15:0] ram_write_address;
    reg [31:0] ram_write_data;
    reg [23:0] heartbeat;
    integer i;
    // Fixed 19-beam table avoids a combinational multiply/DSP in the parser.
    // The testbench checks all entries against the physical 5-degree spacing.
    function signed [15:0] beam_angle;
        input [4:0] index;
        begin
            case(index)
                0:beam_angle=-4500; 1:beam_angle=-4000; 2:beam_angle=-3500;
                3:beam_angle=-3000; 4:beam_angle=-2500; 5:beam_angle=-2000;
                6:beam_angle=-1500; 7:beam_angle=-1000; 8:beam_angle=-500;
                9:beam_angle=0; 10:beam_angle=500; 11:beam_angle=1000;
                12:beam_angle=1500; 13:beam_angle=2000; 14:beam_angle=2500;
                15:beam_angle=3000; 16:beam_angle=3500; 17:beam_angle=4000;
                18:beam_angle=4500; default:beam_angle=32767;
            endcase
        end
    endfunction
    assign fifo_din=ram_q;
    assign fifo_wr_en=reset_n && state==PUT && !fifo_full;
    // received_words reports 16-bit SPI words, NOT bytes.
    wire [15:0] count_or_fault=state==FAULT ? {error_code,received_words[7:0]} : received_words;
    assign status={ready_pipe[2],command[1],command[3],streamed,state,
                   count_or_fault,primed,(state==IDLE && !radar_sync && ready_pipe[2]),
                   heartbeat[23],rejected,request_seen,frame_valid,
                   (error_code!=0),(state!=IDLE)};

    // RAM has NO reset. Register ALL write-port inputs, including enable.
    // Async reset/receiver-valid signals must not directly gate a RAM enable.
    // A pending old-session write may retire on reset; contents are discarded
    // and completely overwritten before the next valid frame is read.
    always @(posedge axis_clk) begin
        if(!reset_n)ram_write_enable<=0;
        else begin
            ram_write_enable<=state==RECEIVE && rx_valid && in_frame &&
                              received_words<SPI_WORDS && received_words[0];
            if(state==RECEIVE && rx_valid && in_frame &&
               received_words<SPI_WORDS && received_words[0])begin
                ram_write_address<=received_words>>1;
                ram_write_data<={rx_word,low_word};
            end
        end
        if(ram_write_enable)frame_ram[ram_write_address]<=ram_write_data;
        if(state==FETCH)ram_q<=frame_ram[read_index];
    end

    // Control and BRAM address registers reset synchronously. The dedicated
    // reset-release synchronizer above still provides asynchronous assertion.
    // This prevents async changes on RAM address/control pins (REQP-1839).
    always @(posedge axis_clk) begin
        if(!reset_n) begin
            state<=IDLE;radar_sync<=0;request_seen<=0;ack_seen<=0;
            primed<=0;rejected<=0;frame_valid<=0;streamed<=0;
            in_frame<=0;header_ok<=0;receive_error<=0;error_code<=0;
            received_words<=0;header_xor<=0;low_word<=0;
            have_seq<=0;last_seq<=0;timer<=0;pulse_remaining<=0;
            ready_went_low<=0;
            prime_limit<=DEFAULT_PRIME_CYCLES;timeout_limit<=DEFAULT_TIMEOUT_CYCLES;
            read_index<=0;heartbeat<=0;
            for(i=0;i<15;i=i+1)header[i]<=0;
        end else begin
            heartbeat<=heartbeat+1'b1;
            if(pulse_remaining!=0) begin
                pulse_remaining<=pulse_remaining-1'b1;
                if(pulse_remaining==1)radar_sync<=0;
            end
            if(state!=IDLE && state!=FAULT)timer<=timer+1'b1;
            if(stream_done)streamed<=1;
            if(command[0]!=request_seen && state==IDLE) begin
                request_seen<=command[0];
                if(state!=IDLE || radar_sync || rx_selected || !ready_pipe[2] ||
                   (!command[1] && (!primed || !command[3]))) begin
                    rejected<=1;
                    if(state==IDLE)begin error_code<=8'h01;state<=FAULT;end
                end else begin
                    rejected<=0;frame_valid<=0;streamed<=0;error_code<=0;
                    received_words<=0;header_xor<=0;in_frame<=0;
                    header_ok<=0;receive_error<=0;timer<=0;read_index<=0;
                    prime_limit<=prime_wait_cycles!=0 ? prime_wait_cycles : DEFAULT_PRIME_CYCLES;
                    timeout_limit<=timeout_cycles!=0 ? timeout_cycles : DEFAULT_TIMEOUT_CYCLES;
                    ack_seen<=command[4];
                    ready_went_low<=0;
                    radar_sync<=1;pulse_remaining<=SYNC_PULSE_CYCLES;
                    state<=command[1] ? PRIME : RECEIVE;
                end
            end else begin
                // Reject a busy request without skipping a simultaneous SPI
                // word, FIFO write, or stream completion from the active job.
                if(command[0]!=request_seen)begin
                    request_seen<=command[0];rejected<=1;
                end
                case(state)
                    IDLE: begin end
                    PRIME: begin
                        // First SYNC fills A but sends no SPI frame. Require a
                        // real READY falling/rising acknowledgement, not a timer.
                        if(!ready_pipe[2])ready_went_low<=1;
                        if(ready_went_low && ready_pipe[2] && !radar_sync)begin
                            primed<=1;state<=IDLE;
                        end
                        if(timer>=prime_limit)begin
                            error_code<=8'h40;state<=FAULT;
                            radar_sync<=0;pulse_remaining<=0;
                        end
                    end
                    RECEIVE: begin
                        if(rx_start)begin
                            in_frame<=1;received_words<=0;header_xor<=0;
                            header_ok<=0;receive_error<=0;
                        end
                        if(rx_valid && in_frame) begin
                            if(received_words<SPI_WORDS) begin
                                if(!received_words[0])low_word<=rx_word;
                                if(received_words<15)begin
                                    header[received_words]<=rx_word;
                                    header_xor<=header_xor^rx_word;
                                end
                                if(received_words==15)begin
                                    if(header[0]!=16'hA610 ||
                                       (header[1]!=2 && header[1]!=3) || header[2]!=16 ||
                                       header[3]>=19 || header[5]!=19 ||
                                       $signed(header[4])!=beam_angle(header[3][4:0]) ||
                                       header[6]!=SAMPLE_POINTS || header[7]!=PAYLOAD_CHIRPS ||
                                       header[8]!=CAPTURE_CHIRPS || header[9]!=4 ||
                                       header[12]!=FRAME_BYTES || header[13]!=32 ||
                                       (header_xor^rx_word)!=0 ||
                                       (header[1]==2 && header[14]!=0))begin
                                        receive_error<=1;error_code<=8'h02;
                                    end else if(header[1]==3 && have_seq &&
                                                header[14]!=(last_seq+16'd1))begin
                                        receive_error<=1;error_code<=8'h04;
                                    end else header_ok<=1;
                                end
                            end else begin receive_error<=1;error_code<=8'h08;end
                            if(received_words!=16'hffff)received_words<=received_words+1'b1;
                        end
                        if(rx_end && in_frame)begin
                            in_frame<=0;
                            if(rx_partial || received_words!=SPI_WORDS || !header_ok || receive_error)begin
                                state<=FAULT;
                                if(error_code==0)error_code<=8'h10;
                            end else begin
                                frame_valid<=1;read_index<=0;state<=FETCH;
                                if(header[1]==3)begin last_seq<=header[14];have_seq<=1;end
                            end
                        end
                    end
                    FETCH:begin state<=PUT;end
                    PUT:if(!fifo_full)begin
                        if(read_index==AXIS_WORDS-1)state<=DRAIN;
                        else begin read_index<=read_index+1'b1;state<=FETCH;end
                    end
                    DRAIN:if(stream_done || streamed)state<=WAIT_ACK;
                    WAIT_ACK:if(command[4]!=ack_seen)begin ack_seen<=command[4];state<=IDLE;end
                    default:begin end
                endcase
                if(rx_start && state!=RECEIVE)begin
                    error_code<=8'h20;state<=FAULT;
                end
                if(state>=RECEIVE && state<=DRAIN && timer>=timeout_limit)begin
                    error_code<=8'h40;state<=FAULT;radar_sync<=0;pulse_remaining<=0;
                end
            end
        end
    end
endmodule
