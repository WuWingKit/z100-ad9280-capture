`timescale 1ns/1ps
`define BD dut.u_bd.capture_bd_i
`define PS `BD.processing_system7_0.inst
module tb_radar_ddr;
    // Set WIRE_MODE=1 for the full 30ms-per-frame wire simulation (very slow
    // with the PS7 VIP). Default injects decoded words at the receiver boundary;
    // actual asynchronous SPI timing is covered separately by tb_radar_sync.
    parameter WIRE_MODE=0;
    reg reset_n=0,ps_clk=0;
    always #15 ps_clk=~ps_clk;
    wire ps_clock_pin=ps_clk,ps_por_pin=reset_n,ps_srst_pin=reset_n;
    wire sclk,cs_n,mosi,sync,mcu_busy;
    reg ready_inject=0;
    wire mcu_ready=WIRE_MODE ? (reset_n && !mcu_busy) : ready_inject;
    wire [31:0] sync_count,sent_frames;
    integer checks=0,frame,i,j,polls,fd,offset;
    reg [31:0] cmd=0,status;
    reg [2047:0] gp;
    reg [1023:0] mem;
    reg [1:0] response;
    reg [15:0] expected;
    localparam BYTES=47136,BASE=32'h01000100;
    z100_radar_top dut(.sys_rst_n(reset_n),.spi_sclk(sclk),.spi_cs_n(cs_n),
        .spi_mosi(mosi),.mcu_ready(mcu_ready),.radar_sync(sync),.FIXED_IO_0_ps_clk(ps_clock_pin),
        .FIXED_IO_0_ps_porb(ps_por_pin),.FIXED_IO_0_ps_srstb(ps_srst_pin));
    mcu_pingpong_model #(.C_VECTORS(1)) model(WIRE_MODE ? sync : 1'b0,reset_n,4'd0,sclk,cs_n,mosi,mcu_busy,sync_count,sent_frames);
    reg inj_start=0,inj_end=0,inj_valid=0;
    reg [15:0] inj_word=0;
    integer w;
    always @(posedge sync)if(!WIRE_MODE)begin
        ready_inject=0;
        if(cmd[1])begin #5000;ready_inject=1;end
    end
    initial if(!WIRE_MODE)begin
        force `BD.radar_frame_capture_0.inst.rx_start=inj_start;
        force `BD.radar_frame_capture_0.inst.rx_end=inj_end;
        force `BD.radar_frame_capture_0.inst.rx_valid=inj_valid;
        force `BD.radar_frame_capture_0.inst.rx_word=inj_word;
        force `BD.radar_frame_capture_0.inst.rx_partial=1'b0;
        force `BD.radar_frame_capture_0.inst.rx_selected=1'b0;
    end
    task inject_frame;
        input integer seq;
        begin
            @(negedge `BD.processing_system7_0_FCLK_CLK0);inj_start=1;
            @(negedge `BD.processing_system7_0_FCLK_CLK0);inj_start=0;
            for(w=0;w<BYTES/2;w=w+1)begin
                inj_word=model.make_word(seq,w);inj_valid=1;
                @(negedge `BD.processing_system7_0_FCLK_CLK0);
            end
            inj_valid=0;
            @(negedge `BD.processing_system7_0_FCLK_CLK0);inj_end=1;
            @(negedge `BD.processing_system7_0_FCLK_CLK0);inj_end=0;
            ready_inject=1;
        end
    endtask
    task check;
        input ok;input [511:0] msg;
        begin checks=checks+1;if(ok!==1'b1)begin
            $display("DDR_FAIL frame=%0d status=%h %s",frame,status,msg);$fatal;
        end end
    endtask
    task wr;
        input [31:0] addr,value;
        begin `PS.write_data(addr,4,value,response);check(response==0,"GP write");end
    endtask
    task rd;
        input [31:0] addr;output [31:0] value;
        begin `PS.read_data(addr,4,gp,response);value=gp[31:0];check(response==0,"GP read");end
    endtask
    initial begin
        #1000;reset_n=1;#1000;`PS.fpga_soft_reset(0);
        wait(`BD.proc_sys_reset_0_peripheral_aresetn===1);ready_inject=1;#1000;
        wr('h41200000,4);#1000;wr('h41200000,0);#1000;
        wr('h41210000,10000);wr('h41210008,10000000);
        cmd=3;wr('h41200000,cmd);#150000;
        rd('h41200008,status);
        check(status[7]&&status[6]&&!status[1],"prime timer complete");
        if(WIRE_MODE)check(sync_count==1&&sent_frames==0,"first SYNC captures A without transmission");
        wr('h40400030,4);polls=0;status=4;
        while(status[2]&&polls<100)begin rd('h40400030,status);polls=polls+1;end
        check(!status[2],"DMA reset complete");
        for(frame=0;frame<2;frame=frame+1)begin
            `PS.write_mem({128{8'hA5}},BASE-128,128);
            for(j=0;j<370;j=j+1)`PS.write_mem({128{8'hA5}},BASE+j*128,128);
            wr('h40400034,'h1000);wr('h40400030,1);
            wr('h40400048,BASE);wr('h40400058,BYTES);
            cmd=((cmd&17)|8)^1;wr('h41200000,cmd);
            if(!WIRE_MODE)inject_frame(frame);
            status=0;polls=0;
            while(!status[12]&&polls<1000)begin
                #100000;rd('h40400034,status);
                check((status&'h4770)==0,"DMA no error");polls=polls+1;
            end
            check(status[12],"DMA IOC after complete frame");
            rd('h41200008,status);
            check(status[28]&&status[2]&&!status[1]&&status[27:24]==6,"frame waits for DDR ACK");
            check(status[23:8]==23568,"23568 SPI words received");
            if(WIRE_MODE)check(sent_frames==frame+1&&sync_count==frame+2,"one-frame delayed MCU ping-pong");
            `PS.read_mem(BASE-128,128,mem);
            for(i=0;i<128;i=i+1)check(mem[i*8+:8]===8'hA5,"leading guard intact");
            for(j=0;j<370;j=j+1)begin
                `PS.read_mem(BASE+j*128,128,mem);
                for(i=0;i<128;i=i+1)begin
                    offset=j*128+i;
                    if(offset<BYTES)begin
                        expected=model.make_word(frame,offset/2);
                        if(offset%2==0)check(mem[i*8+:8]===expected[7:0],"DDR low byte matches previous capture");
                        else check(mem[i*8+:8]===expected[15:8],"DDR high byte matches previous capture");
                    end else check(mem[i*8+:8]===8'hA5,"trailing guard intact");
                end
            end
            cmd=(cmd^16)&~8;wr('h41200000,cmd);#1000;
            rd('h41200008,status);check(status[6],"DDR ACK releases next SYNC");
            $display("RADAR_DDR_FRAME_PASS frame=%0d bytes=%0d",frame,BYTES);
        end
        fd=$fopen("radar_ddr_result.txt","w");
        $fdisplay(fd,"PASS frames=2 frame_bytes=47136 DDR_bytes=94272 C_serialized_vectors=PASS header_payload_sequence_beam=PASS guards=PASS WIRE_MODE=%0d checks=%0d",WIRE_MODE,checks);
        $fclose(fd);$display("RADAR_DDR_INTEGRATION_PASS checks=%0d",checks);$finish;
    end
    initial begin #200000000;$fatal(1,"DDR integration timeout");end
endmodule
`undef BD
`undef PS
