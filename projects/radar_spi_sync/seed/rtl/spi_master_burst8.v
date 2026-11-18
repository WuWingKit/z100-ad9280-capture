`timescale 1ns/1ps
// clk16 is the MMCM 16MHz output. SPI mode 2: idle high, sample falling edge.
// Each byte has CS low for 8 clocks and high for 8 clocks: byte cadence = 1MHz.
// Exactly 8 bytes per start. CS remains high afterwards; extra data is ignored.
module spi_master_burst8(
    input wire clk16, rst_n, start, miso,
    output wire sclk,
    output reg cs_n, mosi, busy,
    output reg [7:0] rx_data,
    output reg rx_valid,
    output reg [63:0] frame_data,
    output reg frame_done
);
    reg [3:0] phase;
    reg [2:0] byte_index;
    reg [7:0] rx_shift, tx_shift;
    reg [63:0] frame_shift;
    assign sclk=clk16;
    // Bit shift register changes ONLY on falling SCLK while selected.
    always @(negedge clk16 or negedge rst_n)
        if(!rst_n) rx_shift<=0;
        else if(busy && !cs_n) rx_shift<={rx_shift[6:0],miso};
    // The following rising SCLK is also CS's rising edge after the 8th bit.
    // Copy the complete shift register to stable parallel registers here.
    always @(posedge clk16 or negedge rst_n) begin
        if(!rst_n) begin
            cs_n<=1; mosi<=0; busy<=0; phase<=0; byte_index<=0;
            tx_shift<=0; rx_data<=0; rx_valid<=0;
            frame_shift<=0; frame_data<=0; frame_done<=0;
        end else begin
            rx_valid<=0; frame_done<=0;
            if(!busy) begin
                cs_n<=1;
                if(start) begin
                    busy<=1; cs_n<=0; phase<=0; byte_index<=0;
                    tx_shift<=8'hA5; mosi<=1; frame_shift<=0;
                end
            end else begin
                phase<=phase+1'b1;
                if(phase<7) begin
                    tx_shift<={tx_shift[6:0],1'b0}; mosi<=tx_shift[6];
                end
                if(phase==7) begin
                    cs_n<=1; rx_data<=rx_shift; rx_valid<=1;
                    frame_shift<={rx_shift,frame_shift[63:8]};
                    if(byte_index==7) begin
                        frame_data<={rx_shift,frame_shift[63:8]};
                        frame_done<=1; busy<=0;
                    end else byte_index<=byte_index+1'b1;
                end
                if(phase==15) begin
                    cs_n<=0; tx_shift<=8'hA5; mosi<=1;
                end
            end
        end
    end
endmodule
