`timescale 1ns/1ps
`define BD dut.u_bd.capture_bd_i
`define PS `BD.processing_system7_0.inst
module tb_loop_ddr;
 reg clk=0,reset_n=0,ps_clk=0;
 always #5 clk=~clk;
 always #15 ps_clk=~ps_clk;
 wire ps_clock_pin=ps_clk,ps_por_pin=reset_n,ps_srst_pin=reset_n;
 wire ad_clk,da_clk;wire [7:0] da_data;
 reg [7:0] ad_data=128,a=128,b=128,c=128,d=128;
 integer frame=0,n=0,checks=0,i,j,polls,fd;
 reg [7:0] expected[0:4095];
 reg [31:0] cmd=0,status;
 reg [2047:0] gp;
 reg [1023:0] mem;
 reg [1:0] response;
 z100_adc_top dut(.sys_clk_p(clk),.sys_clk_n(~clk),.sys_rst_n(reset_n),
 .FIXED_IO_0_ps_clk(ps_clock_pin),.FIXED_IO_0_ps_porb(ps_por_pin),.FIXED_IO_0_ps_srstb(ps_srst_pin),
 .ad_data(ad_data),.ad_otr(1'b0),.ad_clk(ad_clk),.da_data(da_data),.da_clk(da_clk));
 always @(posedge da_clk) a<=#2 (256-da_data);
 always @(posedge ad_clk)begin b<=a;c<=b;d<=c;ad_data<=#25 d;end
 task check;input ok;input [511:0] msg;
 begin checks=checks+1;if(ok!==1'b1)begin $display("DDR_FAIL frame=%0d %s",frame,msg);$fatal;end end endtask
 task wr;input [31:0] addr,value;begin `PS.write_data(addr,4,value,response);check(response==0,"GP write");end endtask
 task rd;input [31:0] addr;output [31:0] value;
 begin `PS.read_data(addr,4,gp,response);value=gp[31:0];check(response==0,"GP read");end endtask
 always @(posedge `BD.adc_clk_0)
 if(`BD.measurement_core_0.inst.state==4 && !`BD.fifo_generator_0.full)begin
  expected[n]=`BD.measurement_core_0.inst.test_mode?n[7:0]:ad_data;n=n+1;
 end
 initial begin
  #1000;reset_n=1;#1000;`PS.fpga_soft_reset(0);
  wait(`BD.proc_sys_reset_0_peripheral_aresetn===1);repeat(100)@(negedge ad_clk);
  wr('h41200000,4);repeat(20)@(negedge ad_clk);wr('h41200000,0);
  repeat(30)@(negedge ad_clk);wr('h40400030,4);polls=0;status=4;
  while(status[2] && polls<100)begin rd('h40400030,status);polls=polls+1;end
  check(!status[2],"DMA reset completes");
  for(frame=0;frame<2;frame=frame+1)begin
   n=0;
   for(j=0;j<3;j=j+1)`PS.write_mem({128{8'hA5}},'h01000000+j*128,128);
   wr('h41210000,42949673);wr('h41210008,256|(3<<13)|(24<<21));
   repeat(20)@(negedge ad_clk);
   wr('h40400034,'h1000);wr('h40400030,1);wr('h40400048,'h01000000);wr('h40400058,256);
   cmd=(cmd&17)|8|(frame==1?2:0);cmd=cmd^1;wr('h41200000,cmd);
   status=0;polls=0;
   while(!status[12] && polls<500)begin rd('h40400034,status);check((status&'h4770)==0,"DMA no error");#100;polls=polls+1;end
   check(status[12],"DMA IOC after DDR write");check(n==256,"exact 256 samples");
   repeat(10)@(negedge ad_clk);rd('h41200008,status);
   check(status[2] && status[25] && !status[1] && !status[4],"frame + stream completion");
   check(status[20:8]==256,"count via GPIO");
   for(j=0;j<3;j=j+1)begin
    `PS.read_mem('h01000000+j*128,128,mem);
    for(i=0;i<128;i=i+1)
     if(j<2)check(mem[i*8+:8]===expected[j*128+i],"DDR equals ADC accepted bytes");
     else check(mem[i*8+:8]===8'hA5,"DDR guard intact");
   end
   if(frame==1)for(i=0;i<256;i=i+1)check(expected[i]===i[7:0],"RAMP integrity");
   cmd=(cmd^16)&~8;wr('h41200000,cmd);repeat(20)@(negedge ad_clk);
   $display("DDR_FRAME_PASS mode=%0d n=%0d",frame,n);
  end
  fd=$fopen("ddr_result.txt","w");$fdisplay(fd,"PASS frames=2 samples=512 DDR_bytes=512 guards=256 checks=%0d",checks);$fclose(fd);
  $display("DDR_INTEGRATION_PASS checks=%0d",checks);$finish;
 end
 initial begin #1000000;$fatal(1,"DDR simulation timeout");end
endmodule
`undef BD
`undef PS

