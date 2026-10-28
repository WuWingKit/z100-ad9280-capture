`timescale 1ns / 1ps
// J37 physical AD9280 interface. ADC drive=10 MHz; capture=10 MHz/180deg.
// ===== 1. 板级端口 =====
// DDR/FIXED_IO 是 Zynq PS 固定接口；AD/DA 引脚来自配套 J37 示例。
// 具体封装引脚在 constraints/z100_adc.xdc，本文件负责信号连接。
module z100_adc_top
    (inout wire [14 : 0] DDR_0_addr,
     inout wire [2 : 0] DDR_0_ba,
     inout wire DDR_0_cas_n,
     inout wire DDR_0_ck_n,
     inout wire DDR_0_ck_p,
     inout wire DDR_0_cke,
     inout wire DDR_0_cs_n,
     inout wire [3 : 0] DDR_0_dm,
     inout wire [31 : 0] DDR_0_dq,
     inout wire [3 : 0] DDR_0_dqs_n,
     inout wire [3 : 0] DDR_0_dqs_p,
     inout wire DDR_0_odt,
     inout wire DDR_0_ras_n,
     inout wire DDR_0_reset_n,
     inout wire DDR_0_we_n,
     inout wire FIXED_IO_0_ddr_vrn,
     inout wire FIXED_IO_0_ddr_vrp,
     inout wire [53 : 0] FIXED_IO_0_mio,
     inout wire FIXED_IO_0_ps_clk,
     inout wire FIXED_IO_0_ps_porb,
     inout wire FIXED_IO_0_ps_srstb,
     input wire sys_clk_p,
     input wire sys_clk_n,
     input wire sys_rst_n,
     input wire [7 : 0] ad_data,
     input wire ad_otr,
     output wire [7 : 0] da_data,
     output wire da_clk,
     output wire ad_clk);
    // ===== 2. 时钟与复位 =====
    // 板载差分 100 MHz 经 Clocking Wizard 生成 AD 驱动与采样时钟。
    // adc_sample_clk 相对驱动时钟延后半个周期，在 ADC 数据稳定后采样。
    // sys_rst_n 或 MMCM 未锁定，都会使板级复位保持有效。
    wire adc_drive_clk, adc_sample_clk, adc_locked;
    wire board_reset_n = sys_rst_n & adc_locked;
    adc_clk_wiz u_adc_clocking(.clk_in1_p(sys_clk_p),
                               .clk_in1_n(sys_clk_n),
                               .clk_out1(adc_drive_clk),
                               .clk_out2(adc_sample_clk),
                               .locked(adc_locked));
    // ===== 3. 使用 ODDR 转发 AD 时钟 =====
    // ODDR 在两种边沿输出 1/0，避免用普通组合逻辑拼接时钟。
    ODDR #(.DDR_CLK_EDGE("SAME_EDGE"),
           .INIT(1'b0),
           .SRTYPE("SYNC"))
        u_adc_clk_out(.C(adc_drive_clk),
                      .CE(1'b1),
                      .D1(1'b1),
                      .D2(1'b0),
                      .R(1'b0),
                      .S(1'b0),
                      .Q(ad_clk));
    // ===== 4. DA 转发时钟 =====
    // D1=0/D2=1 使 DA 上升沿位于数据更新沿之后半周期，留出建立时间。
    ODDR #(.DDR_CLK_EDGE("SAME_EDGE"),
           .INIT(1'b0),
           .SRTYPE("SYNC"))
        u_dac_clk_out(.C(adc_sample_clk),
                      .CE(1'b1),
                      .D1(1'b0),
                      .D2(1'b1),
                      .R(1'b0),
                      .S(1'b0),
                      .Q(da_clk));
    // ===== 5. BD 封装连接 =====
    // 自定义核心、FIFO、CORDIC、DMA、GPIO 和 PS 都在 capture_bd 内连接。
    capture_bd_wrapper u_bd(.dac_data(da_data),
                            .DDR_0_addr(DDR_0_addr),
                            .DDR_0_ba(DDR_0_ba),
                            .DDR_0_cas_n(DDR_0_cas_n),
                            .DDR_0_ck_n(DDR_0_ck_n),
                            .DDR_0_ck_p(DDR_0_ck_p),
                            .DDR_0_cke(DDR_0_cke),
                            .DDR_0_cs_n(DDR_0_cs_n),
                            .DDR_0_dm(DDR_0_dm),
                            .DDR_0_dq(DDR_0_dq),
                            .DDR_0_dqs_n(DDR_0_dqs_n),
                            .DDR_0_dqs_p(DDR_0_dqs_p),
                            .DDR_0_odt(DDR_0_odt),
                            .DDR_0_ras_n(DDR_0_ras_n),
                            .DDR_0_reset_n(DDR_0_reset_n),
                            .DDR_0_we_n(DDR_0_we_n),
                            .FIXED_IO_0_ddr_vrn(FIXED_IO_0_ddr_vrn),
                            .FIXED_IO_0_ddr_vrp(FIXED_IO_0_ddr_vrp),
                            .FIXED_IO_0_mio(FIXED_IO_0_mio),
                            .FIXED_IO_0_ps_clk(FIXED_IO_0_ps_clk),
                            .FIXED_IO_0_ps_porb(FIXED_IO_0_ps_porb),
                            .FIXED_IO_0_ps_srstb(FIXED_IO_0_ps_srstb),
                            .adc_clk_0(adc_sample_clk),
                            .adc_data_0(ad_data),
                            .ad_otr_0(ad_otr),
                            .board_reset_n(board_reset_n));
endmodule
