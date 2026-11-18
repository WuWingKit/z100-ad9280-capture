`timescale 1ns/1ps
// Behavioral MCU model only; does not execute the supplied Cortex-M3 firmware.
// First SYNC captures A only. Later SYNC captures other bank, then sends previous.
module mcu_pingpong_model #(
    parameter integer SAMPLE_POINTS=512,
    parameter integer CAPTURE_CHIRPS=24,
    parameter integer PAYLOAD_CHIRPS=23,
    parameter integer C_VECTORS=0,
    parameter real HALF_SCLK_NS=40.25,
    parameter integer CAPTURE_DELAY_NS=233
)(input wire sync,rst_n,input wire [3:0] fault_mode,
  output reg sclk=0,cs_n=1,mosi=0,
  output reg busy=0,output integer sync_count=0,output integer sent_frames=0);
    localparam WORDS=16+PAYLOAD_CHIRPS*SAMPLE_POINTS*2;
    reg [15:0] bank_a[0:WORDS-1],bank_b[0:WORDS-1];
    integer capture_seq=0,previous_bank=0;
    reg have_previous=0;
    integer i,k,tx_bank,fill_bank,tx_words;
    reg [15:0] tx_word;
    reg [3:0] fault_latched;
    reg [15:0] c_words[0:WORDS*2-1];
    initial if(C_VECTORS)$readmemh("mcu_reference_words.hex",c_words);
    function [15:0] make_word;
        input integer seq,index;
        integer j;
        reg [15:0] v;
        begin
            if(C_VECTORS && seq<2)make_word=c_words[seq*WORDS+index];
            else begin
            case(index)
                0:make_word=16'hA610;
                1:make_word=3;
                2:make_word=16;
                3:make_word=seq%19;
                4:make_word=-4500+(seq%19)*500;
                5:make_word=19;
                6:make_word=SAMPLE_POINTS;
                7:make_word=PAYLOAD_CHIRPS;
                8:make_word=CAPTURE_CHIRPS;
                9:make_word=4;
                10:make_word=16'hA286;
                11:make_word=1;
                12:make_word=WORDS*2;
                13:make_word=32;
                14:make_word=seq;
                15:begin
                    // Explicit header XOR, independent of payload.
                    v=16'hA610^16'd3^16'd16^(seq%19)^((-4500+(seq%19)*500)&16'hffff)^16'd19;
                    v=v^SAMPLE_POINTS^PAYLOAD_CHIRPS^CAPTURE_CHIRPS^16'd4^16'hA286^16'd1^(WORDS*2)^16'd32^(seq&16'hffff);
                    make_word=v;
                end
                default:make_word=16'h2300+seq*257+index*7;
            endcase
            end
        end
    endfunction
    always @(negedge rst_n) begin
        have_previous=0;previous_bank=0;capture_seq=0;sync_count=0;sent_frames=0;
        sclk=0;cs_n=1;mosi=0;
    end
    always @(posedge sync)begin
        if(busy)$fatal(1,"SYNC retriggered while MCU busy");
    end
    initial begin
        forever begin
            @(posedge sync);
            if(rst_n)begin
                busy<=1;sync_count=sync_count+1;
                fault_latched=fault_mode;
                tx_bank=previous_bank;fill_bank=have_previous ? 1-previous_bank : 0;
                #CAPTURE_DELAY_NS;
                for(i=0;i<WORDS;i=i+1)begin
                    if(fill_bank==0)bank_a[i]=make_word(capture_seq,i);
                    else bank_b[i]=make_word(capture_seq,i);
                end
                if(have_previous && fault_latched!=6)begin
                    cs_n=0;sclk=0;
                    tx_words=WORDS;
                    if(fault_latched==2)tx_words=WORDS-2;
                    if(fault_latched==3)tx_words=WORDS+1;
                    for(i=0;i<tx_words;i=i+1)begin
                        tx_word=i<WORDS ? (tx_bank==0 ? bank_a[i] : bank_b[i]) : 16'hFFFF;
                        if(fault_latched==1 && i==15)tx_word=tx_word^1;
                        if(fault_latched==4 && (i==14 || i==15))tx_word=tx_word^1;
                        if(fault_latched==7 && i==3)tx_word=20;
                        if(fault_latched==7 && i==15)tx_word=tx_word^20;
                        if(fault_latched==8 && (i==4 || i==15))tx_word=tx_word^1;
                        for(k=15;k>=0;k=k-1)begin
                            mosi=tx_word[k];#HALF_SCLK_NS;sclk=1;#HALF_SCLK_NS;sclk=0;
                        end
                    end
                    if(fault_latched==5)begin
                        repeat(3)begin mosi=1;#HALF_SCLK_NS;sclk=1;#HALF_SCLK_NS;sclk=0;end
                    end
                    #(HALF_SCLK_NS*2);cs_n=1;mosi=0;
                    sent_frames=sent_frames+1;
                end
                previous_bank=fill_bank;capture_seq=capture_seq+1;have_previous=1;
                busy<=0;
            end
        end
    end
endmodule
