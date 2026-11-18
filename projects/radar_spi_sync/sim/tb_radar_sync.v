`timescale 1ns/1ps
module tb_radar_sync;
    reg clk=0,rst_n=0;
    always #5 clk=~clk;
    reg [31:0] command=0;
    reg [3:0] fault=0;
    wire sync,sclk,cs,mosi,busy;
    wire [31:0] din,dout,status;
    wire wr,rd,full,empty,freset,areset,last,valid,done;
    wire [31:0] tdata;
    wire [3:0] keep;
    wire [15:0] bytes;
    reg ready=0;
    wire [31:0] sync_count,sent_frames;
    reg [1:0] ready_override=0;
    wire mcu_ready=ready_override==1 ? 1'b1 : ready_override==2 ? 1'b0 : (rst_n && !busy);
    radar_frame_capture #(.SAMPLE_POINTS(4),.CAPTURE_CHIRPS(3),.PAYLOAD_CHIRPS(2),.SYNC_PULSE_CYCLES(100))
        dut(clk,rst_n,command,32'd2000,32'd200000,sclk,cs,mosi,mcu_ready,sync,
            full,done,din,wr,freset,areset,bytes,status);
    mcu_pingpong_model #(.SAMPLE_POINTS(4),.CAPTURE_CHIRPS(3),.PAYLOAD_CHIRPS(2))
        model(sync,rst_n,fault,sclk,cs,mosi,busy,sync_count,sent_frames);
    radar_fifo_to_axis axis(clk,areset,dout,empty,rd,tdata,keep,valid,ready,last,done,bytes);
    // Small synchronous FWFT model forces FIFO-full/backpressure paths.
    reg [31:0] mem[0:3];
    reg [1:0] wp=0,rp=0;
    reg [2:0] used=0;
    assign full=used==4;assign empty=used==0;assign dout=mem[rp];
    always @(posedge clk or posedge freset)begin
        if(freset)begin wp<=0;rp<=0;used<=0;end
        else begin
            if(wr && !full)begin mem[wp]<=din;wp<=wp+1'b1;end
            if(rd && !empty)rp<=rp+1'b1;
            case({wr&&!full,rd&&!empty})2'b10:used<=used+1'b1;2'b01:used<=used-1'b1;endcase
        end
    end
    integer expected_seq=0,beats=0,frames=0,checks=0,stalls=0,cycle=0,fd;
    reg monitor=0,backpressure=0,held=0;
    reg [31:0] held_data;
    reg held_last;
    task check;
        input condition;input [511:0] why;
        begin checks=checks+1;if(!condition)$fatal(1,"SYNC_UNIT_FAIL %s status=%h",why,status);end
    endtask
    always @(posedge clk)begin
        if(!rst_n)begin held<=0;cycle<=0;end
        else begin
            cycle<=cycle+1;
            if(backpressure)ready<=cycle%7!=0 && cycle%7!=1;
            if(held)begin
                check(valid && tdata===held_data && last===held_last,"AXIS stable while stalled");
            end
            held<=valid&&!ready;
            if(valid&&!ready)begin held_data<=tdata;held_last<=last;stalls=stalls+1;end
            if(valid&&ready)begin
                check(monitor,"bad frame must never reach DMA");
                check(tdata==={model.make_word(expected_seq,beats*2+1),model.make_word(expected_seq,beats*2)},"data/header tied to prior MCU frame");
                check(keep==15,"TKEEP all bytes");
                check(last==(beats==15),"TLAST second-to-last boundaries");
                if(last)begin beats=0;frames=frames+1;end else beats=beats+1;
            end
        end
    end
    task reset_all;
        begin
            rst_n=0;command=0;ready=0;backpressure=0;monitor=0;fault=0;beats=0;expected_seq=0;ready_override=0;
            #123;rst_n=1;repeat(20)@(negedge clk);
        end
    endtask
    task request;
        input prime;
        begin
            @(negedge clk);command=(command&17)|(prime ? 2 : 8);command=command^1;repeat(2)@(negedge clk);
        end
    endtask
    task prime_once;
        begin request(1);wait(status[6]);check(status[7],"prime READY falling/rising acknowledgement completed");check(sent_frames==0,"first SYNC no stale SPI frame");end
    endtask
    task wait_frame;
        integer n;
        begin n=0;while(!status[28] && !status[1] && n<500000)begin @(negedge clk);n=n+1;end
            check(!status[1] && status[2] && status[28],"complete frame and stream");
        end
    endtask
    task ack;
        begin @(negedge clk);command=(command^16)&~8;wait(status[6]);end
    endtask
    integer test_case,before_frames,before_sync;
    initial begin
        reset_all();
        for(test_case=0;test_case<19;test_case=test_case+1)
            check($signed(dut.beam_angle(test_case))==(-4500+test_case*500),"all 19 beam angle table entries");
        ready_override=2;repeat(8)@(negedge clk);request(1);repeat(20)@(negedge clk);
        check(status[1] && sync_count==0,"MCU not ready: no SYNC emitted");
        reset_all();ready_override=1;request(1);wait(status[27:24]==7);
        check(status[1] && !status[7],"stuck-high READY cannot fake prime completion");
        reset_all();
        request(0);repeat(20)@(negedge clk);
        check(status[1] && sync_count==0,"reject capture without prime, no unwanted SYNC");
        reset_all();prime_once();
        for(test_case=0;test_case<3;test_case=test_case+1)begin
            expected_seq=test_case;monitor=1;before_frames=frames;
            request(0);
            wait(dut.received_words==8 && dut.rx_valid);
            before_sync=sync_count;request(0);
            check(sync_count==before_sync,"busy request during SPI does not retrigger or drop a word");
            wait(full);repeat(100)@(negedge clk);
            check(!status[1],"FIFO full holds RAM drain without loss");
            backpressure=1;wait_frame();
            check(frames==before_frames+1,"exactly one frame per request");
            check(sent_frames==test_case+1,"MCU sends exactly previous capture");
            before_sync=sync_count;request(0);repeat(20)@(negedge clk);
            check(sync_count==before_sync,"no retrigger before PS DDR ACK");
            ack();ready=0;backpressure=0;
        end
        for(test_case=1;test_case<=8;test_case=test_case+1)begin
            if(test_case!=4)begin
                reset_all();prime_once();before_frames=frames;
                fault=test_case;monitor=0;ready=1;request(0);
                wait(status[27:24]==7);wait(!busy);repeat(20)@(negedge clk);
                check(frames==before_frames && empty,"invalid/timeout frame never leaks to AXIS");
            end
        end
        // Establish sequence then repeat/alter the next sequence with valid XOR.
        reset_all();prime_once();monitor=1;ready=1;request(0);wait_frame();ack();
        monitor=0;fault=4;before_frames=frames;request(0);wait(status[27:24]==7);
        repeat(20)@(negedge clk);check(frames==before_frames,"reject duplicated/nonconsecutive v3 sequence");
        fd=$fopen("radar_sync_unit_result.txt","w");
        $fdisplay(fd,"PASS valid_frames=4 checks=%0d stalls=%0d prime_no_spi=PASS prior_frame_beam=PASS no_early_sync=PASS header_length_partial_timeout_faults=PASS",checks,stalls);
        $fclose(fd);$display("RADAR_SYNC_UNIT_PASS checks=%0d",checks);$finish;
    end
    initial begin #20000000;$fatal(1,"Unit test timeout");end
endmodule
