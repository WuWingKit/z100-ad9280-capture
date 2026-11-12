`timescale 1ns/1ps
// External MCU is SPI master. Mode 0, MSB first, 16 bits per word.
// Sample external pins in the 100MHz AXIS domain; target SCLK <=12.5MHz.
// All pins require board timing constraints before physical implementation.
module spi_slave_rx16(
    input wire clk, rst_n,
    input wire spi_sclk, spi_cs_n, spi_mosi,
    output reg [15:0] word_data,
    output reg word_valid, frame_start, frame_end, partial_word,
    output wire selected
);
    (* ASYNC_REG="TRUE" *) reg [2:0] sck_pipe,cs_pipe,mosi_pipe;
    reg sck_prev,cs_prev;
    reg [3:0] bit_count;
    reg [15:0] shift;
    assign selected=~cs_pipe[2];
    wire cs_fall=cs_prev && !cs_pipe[2];
    wire cs_rise=!cs_prev && cs_pipe[2];
    wire sck_rise=!sck_prev && sck_pipe[2];
    always @(posedge clk or negedge rst_n) begin
        if(!rst_n) begin
            sck_pipe<=0;cs_pipe<=3'b111;mosi_pipe<=0;
            sck_prev<=0;cs_prev<=1;bit_count<=0;shift<=0;
            word_data<=0;word_valid<=0;frame_start<=0;frame_end<=0;partial_word<=0;
        end else begin
            sck_pipe<={sck_pipe[1:0],spi_sclk};
            cs_pipe<={cs_pipe[1:0],spi_cs_n};
            mosi_pipe<={mosi_pipe[1:0],spi_mosi};
            sck_prev<=sck_pipe[2];cs_prev<=cs_pipe[2];
            word_valid<=0;frame_start<=0;frame_end<=0;partial_word<=0;
            if(cs_fall) begin bit_count<=0;shift<=0;frame_start<=1;end
            else if(!cs_pipe[2] && sck_rise) begin
                shift<={shift[14:0],mosi_pipe[2]};
                if(bit_count==15) begin
                    word_data<={shift[14:0],mosi_pipe[2]};word_valid<=1;bit_count<=0;
                end else bit_count<=bit_count+1'b1;
            end
            if(cs_rise) begin
                frame_end<=1;partial_word<=bit_count!=0;bit_count<=0;
            end
        end
    end
endmodule
