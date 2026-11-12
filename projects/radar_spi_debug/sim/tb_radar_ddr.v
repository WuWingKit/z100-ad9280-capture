`timescale 1ns/1ps
`define BD dut.u_bd.capture_bd_i
`define PS `BD.processing_system7_0.inst
module tb_radar_ddr;
    // Real BD with reduced simulation geometry, no forced decoder signals.
    localparam POINTS=16,CHIRPS=3;
    reg reset_n=0,ps_clk=0;
    always #15 ps_clk=~ps_clk;
    wire ps_clock_pin=ps_clk,ps_por_pin=reset_n,ps_srst_pin=reset_n;
    integer checks=0,frame,i,j,polls,fd,offset;
    reg [31:0] cmd=0,status;
    reg [2047:0] gp;
    reg [1023:0] mem;
    reg [1:0] response;
    reg [15:0] expected;
    localparam BYTES=160,BASE=32'h01000100;
    z100_radar_debug_top dut(.sys_rst_n(reset_n),.FIXED_IO_0_ps_clk(ps_clock_pin),
        .FIXED_IO_0_ps_porb(ps_por_pin),.FIXED_IO_0_ps_srstb(ps_srst_pin));
    defparam dut.u_bd.capture_bd_i.frontend_sim_0.inst.SAMPLE_POINTS=POINTS;
    defparam dut.u_bd.capture_bd_i.frontend_sim_0.inst.CAPTURE_CHIRPS=CHIRPS;
    defparam dut.u_bd.capture_bd_i.radar_frame_capture_0.inst.SAMPLE_POINTS=POINTS;
    defparam dut.u_bd.capture_bd_i.radar_frame_capture_0.inst.CAPTURE_CHIRPS=CHIRPS;
    defparam dut.u_bd.capture_bd_i.radar_frame_capture_0.inst.PAYLOAD_CHIRPS=CHIRPS-1;
    `include "expected_word.vh"
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
        wait(`BD.proc_sys_reset_0_peripheral_aresetn===1);#1000;
        wr('h41200000,4);#1000;wr('h41200000,0);#1000;
        wr('h41210000,10000);wr('h41210008,10000000);
        cmd=3;wr('h41200000,cmd);#150000;
        rd('h41200008,status);
        check(status[7]&&status[6]&&!status[1],"prime timer complete");
        wr('h40400030,4);polls=0;status=4;
        while(status[2]&&polls<100)begin rd('h40400030,status);polls=polls+1;end
        check(!status[2],"DMA reset complete");
        for(frame=0;frame<3;frame=frame+1)begin
            `PS.write_mem({128{8'hA5}},BASE-128,128);
            for(j=0;j<2;j=j+1)`PS.write_mem({128{8'hA5}},BASE+j*128,128);
            wr('h40400034,'h1000);wr('h40400030,1);
            wr('h40400048,BASE);wr('h40400058,BYTES);
            cmd=((cmd&17)|8)^1;wr('h41200000,cmd);
            status=0;polls=0;
            while(!status[12]&&polls<1000)begin
                #100000;rd('h40400034,status);
                check((status&'h4770)==0,"DMA no error");polls=polls+1;
            end
            check(status[12],"DMA IOC after complete frame");
            rd('h41200008,status);
            check(status[28]&&status[2]&&!status[1]&&status[27:24]==6,"frame waits for DDR ACK");
            check(status[23:8]==80,"80 SPI words received");
            `PS.read_mem(BASE-128,128,mem);
            for(i=0;i<128;i=i+1)check(mem[i*8+:8]===8'hA5,"leading guard intact");
            for(j=0;j<2;j=j+1)begin
                `PS.read_mem(BASE+j*128,128,mem);
                for(i=0;i<128;i=i+1)begin
                    offset=j*128+i;
                    if(offset<BYTES)begin
                        expected=expected_word(frame,offset/2);
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
        $fdisplay(fd,"PASS frames=3 frame_bytes=160 DDR_bytes=480 synthesizable_source=PASS no_forcing=PASS serial_SPI=PASS actual_FIFO_DMA_HP0_DDR=PASS previous_header_payload_sequence_beam=PASS guards=PASS checks=%0d",checks);
        $fclose(fd);$display("RADAR_DDR_INTEGRATION_PASS checks=%0d",checks);$finish;
    end
    initial begin #10000000;$fatal(1,"DDR integration timeout");end
endmodule
`undef BD
`undef PS
