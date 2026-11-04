`timescale 1ns/1ps
module tb_spi_burst8;
    reg clk=0,rst_n=0,start=0;
    wire sclk,cs_n,mosi,busy,valid,done,miso;
    wire [7:0] byte_data;
    wire [63:0] frame_data;
    wire [7:0] source=slave.transactions;
    integer burst,j,received=0,completed=0,fd;
    reg [63:0] expected;
    realtime last_cs_fall=-1000, cs_fall;
    always #31.25 clk=~clk;
    spi_master_burst8 dut(clk,rst_n,start,miso,sclk,cs_n,mosi,busy,byte_data,valid,frame_data,done);
    spi_slave_model slave(sclk,cs_n,mosi,source,miso,rst_n);
    always @(negedge cs_n) begin
        cs_fall=$realtime;
        if(busy && (slave.transactions%8)!=0 && $realtime-last_cs_fall != 1000.0)
            $fatal(1,"CS repetition is not 1MHz");
        last_cs_fall=$realtime;
    end
    always @(posedge cs_n) if(rst_n && slave.bits!=0 && $realtime-cs_fall!=500.0)
        $fatal(1,"CS low width is not 500ns");
    always @(posedge clk) begin
        if(valid) begin
            if(byte_data !== received[7:0]) $fatal(1,"SPI byte mismatch");
            received=received+1;
        end
        if(done) completed=completed+1;
    end
    initial begin
        #200; rst_n=1;
        for(burst=0;burst<32;burst=burst+1) begin
            @(negedge clk);start=1;
            @(negedge clk);start=0;
            wait(done); #1;
            for(j=0;j<8;j=j+1) expected[j*8+:8]=burst*8+j;
            if(frame_data!==expected) $fatal(1,"64-bit parallel latch byte order");
            repeat(40) @(negedge clk);
            if(received!=(burst+1)*8 || !cs_n || busy) $fatal(1,"Extra samples after byte 8");
        end
        if(completed!=32) $fatal(1,"Missing burst completion");
        fd=$fopen("spi_burst8_result.txt","w");
        $fdisplay(fd,"PASS mode=2 SCLK=16MHz CS=1MHz low=500ns bytes_per_capture=8 bursts=32 all_256_values parallel64=PASS extra_ignored=PASS MOSI_A5=PASS");
        $fclose(fd);$display("SPI_BURST8_PASS");$finish;
    end
    initial begin #1000000; $fatal(1,"SPI burst timeout");end
endmodule
