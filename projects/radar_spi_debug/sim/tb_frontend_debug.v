`timescale 1ns/1ps
// Actual synthesizable source -> actual SPI decoder/parser/BRAM/AXIS.
// FULL=1 checks production-sized frames; FULL=0 also tests source faults/reset.
module tb_frontend_debug;
    parameter FULL=0;
    localparam POINTS=FULL ? 512 : 16,CHIRPS=FULL ? 24 : 3;
    localparam BYTES=32+(CHIRPS-1)*POINTS*4;
    reg clk=0,rst_n=0;always #5 clk=~clk;
    reg [31:0] command=0;
    wire sync,source_ready,sclk,cs,mosi,wr,rd,full,empty,freset,areset;
    wire [31:0] status,din,dout,tdata,events,rejects;
    wire [15:0] bytes,next_seq,tx_seq;
    wire [3:0] keep;wire valid,last,done;
    reg [3:0] wp=0,rp=0;reg [4:0] used=0;reg [31:0] fifo[0:15];
    integer cycle=0,beat=0,frame=0,checks=0,frames=0,cs_edges=0,fd;
    integer expected_seq=0;reg accept_output=1;
    reg held=0;reg [31:0] held_data;reg held_last;
    wire ready=(cycle%11)>2;
    `include "expected_word.vh"
    radar_frame_capture #(.SAMPLE_POINTS(POINTS),.CAPTURE_CHIRPS(CHIRPS),
        .PAYLOAD_CHIRPS(CHIRPS-1),.SYNC_PULSE_CYCLES(20)) core(
        clk,rst_n,command,32'd100000,32'd10000000,
        sclk,cs,mosi,source_ready,sync,full,done,din,wr,freset,areset,bytes,status);
    radar_frontend_sim #(.SAMPLE_POINTS(POINTS),.CAPTURE_CHIRPS(CHIRPS)) source(
        clk,areset,sync,command,source_ready,sclk,cs,mosi,next_seq,tx_seq,events,rejects);
    radar_fifo_to_axis axis(clk,areset,dout,empty,rd,tdata,keep,valid,ready,last,done,bytes);
    assign full=used==16;assign empty=used==0;assign dout=fifo[rp];
    always @(negedge cs)cs_edges=cs_edges+1;
    always @(posedge clk or posedge freset)begin
        if(freset)begin wp<=0;rp<=0;used<=0;end
        else begin
            if(wr&&!full)begin fifo[wp]<=din;wp<=wp+1'b1;end
            if(rd&&!empty)rp<=rp+1'b1;
            case({wr&&!full,rd&&!empty})2'b10:used<=used+1'b1;2'b01:used<=used-1'b1;endcase
        end
    end
    task check;input ok;input [511:0] why;
        begin checks=checks+1;if(ok!==1'b1)$fatal(1,"DEBUG_FAIL %s status=%h beat=%0d",why,status,beat);end
    endtask
    always @(posedge clk)begin
        cycle<=cycle+1;
        if(!areset)held<=0;
        else begin
            if(held)check(valid&&tdata===held_data&&last===held_last,"AXIS stable under backpressure");
            held<=valid&&!ready;held_data<=tdata;held_last<=last;
            if(valid&&ready)begin
                check(accept_output,"invalid frame must never reach FIFO/AXIS");
                check(tdata==={expected_word(expected_seq,2*beat+1),expected_word(expected_seq,2*beat)},"previous capture header/payload");
                check(keep==15&&last==(beat==BYTES/4-1),"TKEEP/TLAST");
                if(last)begin frames=frames+1;beat=0;end else beat=beat+1;
            end
        end
    end
    task reset_both;
        begin
            @(negedge clk);command=4;repeat(20)@(negedge clk);
            command=0;beat=0;expected_seq=0;wait(status[6]);
            check(cs&&next_seq==0,"shared reset aborts SPI and clears both banks session");
        end
    endtask
    task prime;
        integer edges_before;
        begin
            edges_before=cs_edges;
            @(negedge clk);command=3;
            wait(!status[6]);wait(status[6]);
            check(status[7]&&events==1&&cs_edges==edges_before,"prime captures only, no SPI transfer");
        end
    endtask
    task request;input [31:0] faults;
        begin @(negedge clk);command=(((command&17)|8)^1)|faults;wait(!status[6]);end
    endtask
    task bad;input [31:0] fault_bits;input [7:0] code;
        integer frames_before;
        begin
            reset_both();prime();frames_before=frames;accept_output=0;
            request(fault_bits);wait(status[27:24]==7);repeat(20)@(negedge clk);
            check(status[27:24]==7&&(status[23:16]&code)!=0,"fault detected and locked");
            check(frames==frames_before&&empty,"fault releases no frame data");
            accept_output=1;
        end
    endtask
    initial begin
        #123;rst_n=1;wait(status[6]);prime();
        for(frame=0;frame<(FULL?3:21);frame=frame+1)begin
            expected_seq=frame;request(0);wait(status[28]||status[1]);@(negedge clk);
            check(!status[1]&&status[2]&&tx_seq==frame,"previous bank selected, header valid");
            check(frames==frame+1&&bytes==BYTES&&status[23:8]==BYTES/2,"exact frame size");
            command=(command^16)&~8;wait(status[6]);
        end
        if(!FULL)begin
            bad(32'h100,8'h02);bad(32'h200,8'h10);
            // Shorten only the receiver timeout, not the source wire timing.
            force core.timeout_limit=32'd50000;
            bad(32'h400,8'h40);release core.timeout_limit;
            reset_both();prime();request(0);wait(!cs);
            reset_both();prime();expected_seq=0;request(0);
            wait(status[28]||status[1]);check(!status[1],"recovery after reset during SPI");
        end
        fd=$fopen(FULL?"full_wire_result.txt":"unit_result.txt","w");
        $fdisplay(fd,"PASS full=%0d frame_bytes=%0d good_frames=%0d checks=%0d real_synthesizable_source_SPI=PASS previous_header_payload_beam=PASS AXIS_backpressure=PASS",FULL,BYTES,frames,checks);
        $fclose(fd);$display("FPGA_SOURCE_TEST_PASS full=%0d checks=%0d",FULL,checks);$finish;
    end
    initial begin #150000000;$fatal(1,"Source test timeout");end
endmodule
