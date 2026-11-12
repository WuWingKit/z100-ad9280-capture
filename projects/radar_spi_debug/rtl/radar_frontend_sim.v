`timescale 1ns/1ps
// Synthesizable front-end surrogate: no external MCU, ADC, RF or wiring.
// Two real BRAM banks hold captures. The first SYNC fills A only; subsequent
// SYNCs fill the other bank, then send the previous bank over real SPI signals.
// Output pattern matches the MCU RAM selftest: {Q=0x8000|seq, I=sample_index}.
module radar_frontend_sim #(
    parameter integer SAMPLE_POINTS=512,
    parameter integer CAPTURE_CHIRPS=24,
    parameter integer SCAN_ENABLE=1,
    parameter integer HALF_SCLK_CYCLES=4
)(
    input wire clk,rst_n,sync,
    input wire [31:0] command,
    output reg ready,spi_sclk,spi_cs_n,spi_mosi,
    output reg [15:0] next_capture_seq,last_tx_seq,
    output reg [31:0] completed_events,rejected_sync
);
    localparam integer CAPTURE_WORDS=SAMPLE_POINTS*CAPTURE_CHIRPS;
    localparam integer ADDRESS_BITS=$clog2(CAPTURE_WORDS);
    localparam integer BANK_WORDS=1<<ADDRESS_BITS;
    localparam integer START_WORD=SAMPLE_POINTS-8;
    localparam integer FRAME_BYTES=32+(CAPTURE_CHIRPS-1)*SAMPLE_POINTS*4;
    localparam IDLE=0,FILL=1,HEADER=2,PICK_TX=3,FETCH=4,LOAD=5,BITS=6,ADVANCE=7,FINISH=8;

    reg [3:0] state;
    reg sync_q,have_previous,previous_bank,capture_bank,tx_bank;
    reg [15:0] capture_seq;
    reg [4:0] next_beam,capture_beam;
    reg [15:0] bank_seq[0:1];
    reg [ADDRESS_BITS-1:0] fill_index,tx_index;
    reg [2:0] header_index,fault;
    reg [31:0] shift,ram_read;
    reg [5:0] bits_left;
    reg [15:0] half_ticks;
    (* ram_style="block" *) reg [31:0] banks[0:BANK_WORDS*2-1];

    function [15:0] beam_angle;
        input [4:0] beam;
        begin
            case(beam)
                0:beam_angle=-4500; 1:beam_angle=-4000; 2:beam_angle=-3500;
                3:beam_angle=-3000; 4:beam_angle=-2500; 5:beam_angle=-2000;
                6:beam_angle=-1500; 7:beam_angle=-1000; 8:beam_angle=-500;
                9:beam_angle=0; 10:beam_angle=500; 11:beam_angle=1000;
                12:beam_angle=1500; 13:beam_angle=2000; 14:beam_angle=2500;
                15:beam_angle=3000; 16:beam_angle=3500; 17:beam_angle=4000;
                18:beam_angle=4500; default:beam_angle=0;
            endcase
        end
    endfunction
    function [15:0] header_word;
        input [3:0] index;
        input [15:0] seq;
        input [4:0] beam;
        begin
            case(index)
                0:header_word=16'hA610;
                1:header_word=3;
                2:header_word=16;
                3:header_word=beam;
                4:header_word=beam_angle(beam);
                5:header_word=19;
                6:header_word=SAMPLE_POINTS;
                7:header_word=CAPTURE_CHIRPS-1;
                8:header_word=CAPTURE_CHIRPS;
                9:header_word=4;
                10:header_word=16'hA286;
                11:header_word=1;
                12:header_word=FRAME_BYTES;
                13:header_word=32;
                14:header_word=seq;
                15:header_word=16'hA610^16'd3^16'd16^{11'd0,beam}^beam_angle(beam)^16'd19^
                    SAMPLE_POINTS^(CAPTURE_CHIRPS-1)^CAPTURE_CHIRPS^16'd4^
                    16'hA286^16'd1^FRAME_BYTES^16'd32^seq;
            endcase
        end
    endfunction

    // BRAM ports have synchronous controls and no memory reset. Filling a
    // complete bank overwrites old contents before sealing its 32-byte header.
    // A single muxed write port is required for Vivado BRAM inference.
    // Capture and header sealing are mutually exclusive FSM states.
    wire [ADDRESS_BITS-1:0] header_address=START_WORD+header_index;
    wire [ADDRESS_BITS:0] write_address={capture_bank,
                           state==HEADER ? header_address : fill_index};
    wire [31:0] write_data=state==HEADER ?
                {header_word({header_index,1'b1},capture_seq,capture_beam),
                 header_word({header_index,1'b0},capture_seq,capture_beam)} :
                ((32'h8000|capture_seq)<<16)|fill_index;
    always @(posedge clk)begin
        if(state==FILL || state==HEADER)banks[write_address]<=write_data;
        if(state==FETCH)ram_read<=banks[{tx_bank,tx_index}];
    end

    // Faults latch at SYNC: bit8=bad header XOR, bit9=short by one 32-bit word,
    // bit10=no SPI reply. This never changes or bypasses the receiver itself.
    wire [31:0] tx_data=(fault[0] && tx_index==START_WORD+7) ?
                         ram_read^32'h00010000 : ram_read;
    always @(posedge clk)begin
        if(!rst_n)begin
            state<=IDLE;sync_q<=0;have_previous<=0;previous_bank<=0;
            capture_bank<=0;tx_bank<=0;capture_seq<=0;
            next_capture_seq<=0;last_tx_seq<=0;next_beam<=9;capture_beam<=9;
            fill_index<=0;tx_index<=0;header_index<=0;fault<=0;
            shift<=0;bits_left<=0;half_ticks<=0;
            ready<=0;spi_sclk<=0;spi_cs_n<=1;spi_mosi<=0;
            completed_events<=0;rejected_sync<=0;bank_seq[0]<=0;bank_seq[1]<=0;
        end else begin
            sync_q<=sync;
            if(sync&&!sync_q && state!=IDLE)rejected_sync<=rejected_sync+1'b1;
            case(state)
                IDLE:begin
                    ready<=1;
                    if(sync&&!sync_q)begin
                        ready<=0;capture_bank<=have_previous ? ~previous_bank : 1'b0;
                        capture_seq<=next_capture_seq;capture_beam<=next_beam;
                        fill_index<=0;fault<=command[10:8];state<=FILL;
                    end
                end
                FILL:begin
                    if(fill_index==CAPTURE_WORDS-1)begin header_index<=0;state<=HEADER;end
                    else fill_index<=fill_index+1'b1;
                end
                HEADER:begin
                    if(header_index==7)begin bank_seq[capture_bank]<=capture_seq;state<=PICK_TX;end
                    else header_index<=header_index+1'b1;
                end
                PICK_TX:begin
                    if(!have_previous || fault[2])state<=FINISH;
                    else begin
                        tx_bank<=previous_bank;tx_index<=START_WORD;
                        spi_cs_n<=0;spi_sclk<=0;state<=FETCH;
                    end
                end
                FETCH:state<=LOAD;
                LOAD:begin
                    // Each RAM word sends I/low16 first, then Q/high16.
                    shift<={tx_data[15:0],tx_data[31:16]};
                    spi_mosi<=tx_data[15];bits_left<=31;half_ticks<=0;state<=BITS;
                end
                BITS:begin
                    if(half_ticks==HALF_SCLK_CYCLES-1)begin
                        half_ticks<=0;
                        if(!spi_sclk)spi_sclk<=1;
                        else begin
                            spi_sclk<=0;
                            if(bits_left==0)state<=ADVANCE;
                            else begin
                                bits_left<=bits_left-1'b1;
                                shift<={shift[30:0],1'b0};spi_mosi<=shift[30];
                            end
                        end
                    end else half_ticks<=half_ticks+1'b1;
                end
                ADVANCE:begin
                    if(tx_index==CAPTURE_WORDS-1-(fault[1]?1:0))begin
                        last_tx_seq<=bank_seq[tx_bank];state<=FINISH;
                    end else begin tx_index<=tx_index+1'b1;state<=FETCH;end
                end
                FINISH:begin
                    spi_cs_n<=1;spi_sclk<=0;spi_mosi<=0;
                    previous_bank<=capture_bank;have_previous<=1;
                    next_capture_seq<=next_capture_seq+1'b1;
                    if(SCAN_ENABLE)next_beam<=next_beam==18 ? 5'd0 : next_beam+1'b1;
                    completed_events<=completed_events+1'b1;
                    ready<=1;state<=IDLE;
                end
                default:state<=IDLE;
            endcase
        end
    end
endmodule
