`timescale 1ns/1ps
// TESTBENCH ONLY: represents PS SPI slave shift logic. It does not execute ARM C.
module spi_slave_model(input wire sclk, cs_n, mosi,
                       input wire [7:0] tx_data, output wire miso, input wire reset_n);
    reg [7:0] shift=0, latched=0, received=0;
    integer bits=0, transactions=0;
    assign miso = cs_n ? 1'b0 : shift[7];
    always @(negedge cs_n) begin
        shift=tx_data; latched=tx_data; bits=0; received=0;
    end
    always @(negedge sclk) if (!cs_n) begin
        received={received[6:0],mosi}; bits=bits+1;
    end
    always @(posedge sclk) if (!cs_n) shift={shift[6:0],1'b0};
    always @(posedge cs_n) if (bits != 0 && reset_n) begin
        if (bits != 8) $fatal(1,"SPI transaction was not 8 bits");
        if (received != 8'hA5) $fatal(1,"SPI MOSI bit order mismatch");
        transactions=transactions+1;
    end
endmodule
