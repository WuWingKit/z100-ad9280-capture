`timescale 1ns/1ps
// Full 47136-byte SPI wire frames, without slow PS7 VIP. No decoder forcing.
// Combines the actual receiver, parser, full BRAM and AXIS adapter. FIFO is
// modeled here; the separate DDR test uses the actual FIFO/DMA/HP0 IPs.
module tb_radar_full_wire;
    reg clk=0,rst_n=0;
    always #5 clk=~clk;
    reg [31:0] command=0;
    wire sync,sclk,cs,mosi,mcu_busy;
    wire [31:0] sync_count,sent_frames,status,din,dout,tdata;
    wire wr,rd,full,empty,freset,areset,valid,last,done;
    wire [15:0] bytes;
    wire [3:0] keep;
    reg [3:0] wp=0,rp=0;
    reg [4:0] used=0;
    reg [31:0] fifo[0:15];
    integer cycle=0,beat=0,frame=0,checks=0,fd,observed_frames=0;
    reg [31:0] expected;
    reg held=0;
    reg [31:0] held_data;
    reg held_last;
    wire ready=(cycle%11)>2;
    radar_frame_capture core(clk,rst_n,command,32'd5000000,32'd20000000,
        sclk,cs,mosi,(rst_n&&!mcu_busy),sync,full,done,din,wr,freset,areset,bytes,status);
    mcu_pingpong_model #(.C_VECTORS(1)) model(sync,rst_n,4'd0,sclk,cs,mosi,mcu_busy,sync_count,sent_frames);
    radar_fifo_to_axis axis(clk,areset,dout,empty,rd,tdata,keep,valid,ready,last,done,bytes);
    assign full=used==16;assign empty=used==0;assign dout=fifo[rp];
    always @(posedge clk or posedge freset)begin
        if(freset)begin wp<=0;rp<=0;used<=0;end
        else begin
            if(wr&&!full)begin fifo[wp]<=din;wp<=wp+1'b1;end
            if(rd&&!empty)rp<=rp+1'b1;
            case({wr&&!full,rd&&!empty})2'b10:used<=used+1'b1;2'b01:used<=used-1'b1;endcase
        end
    end
    task check;
        input ok;input [511:0] why;
        begin checks=checks+1;if(ok!==1'b1)$fatal(1,"FULL_WIRE_FAIL %s status=%h beat=%0d",why,status,beat);end
    endtask
    always @(posedge clk)begin
        if(rst_n)begin
            cycle<=cycle+1;
            if(held)check(valid&&tdata===held_data&&last===held_last,"AXIS stable under backpressure");
            held<=valid&&!ready;held_data<=tdata;held_last<=last;
            if(valid&&ready)begin
                expected={model.make_word(frame,beat*2+1),model.make_word(frame,beat*2)};
                check(tdata===expected,"word equals previous MCU capture");
                check(keep==15&&last==(beat==11783),"KEEP and full-frame TLAST");
                if(last)begin observed_frames=observed_frames+1;beat=0;end
                else beat=beat+1;
            end
        end
    end
    initial begin
        #123;rst_n=1;wait(status[6]);
        @(negedge clk);command=3;
        wait(!status[6]);wait(status[6]);
        check(status[7]&&sync_count==1&&sent_frames==0,"prime acknowledged with no SPI output");
        for(frame=0;frame<2;frame=frame+1)begin
            @(negedge clk);command=((command&17)|8)^1;
            wait(!status[6]);wait(status[28]||status[1]);
            @(negedge clk);
            check(!status[1]&&status[2],"valid full SPI wire frame");
            check(observed_frames==frame+1&&sent_frames==frame+1,"one frame from previous bank");
            check(status[23:8]==23568&&bytes==47136,"full SPI word and byte count");
            command=(command^16)&~8;wait(status[6]);
            $display("FULL_WIRE_FRAME_PASS seq=%0d bytes=47136",frame);
        end
        fd=$fopen("radar_full_wire_result.txt","w");
        $fdisplay(fd,"PASS full_wire_frames=2 bytes=94272 C_serialized_vectors=PASS no_decoder_forcing=PASS previous_capture_header_payload=PASS READY_prime=PASS backpressure=PASS checks=%0d",checks);
        $fclose(fd);$display("RADAR_FULL_WIRE_PASS checks=%0d",checks);$finish;
    end
    initial begin #100000000;$fatal(1,"Full wire simulation timeout");end
endmodule
