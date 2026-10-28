# 阅读顺序：1.时钟/复位引脚 2.AD输入映射与输入时序 3.异步时钟域 4.DA输出映射与输出时序。
# 本次仅补注释，封装引脚、电平和时序数值保持不变。
# Official 27_hs_ad_da example / J37 AD input mapping, LVCMOS33.
set_property PACKAGE_PIN D9 [get_ports sys_clk_p]
set_property PACKAGE_PIN D8 [get_ports sys_clk_n]
set_property IOSTANDARD DIFF_SSTL135 [get_ports {sys_clk_p sys_clk_n}]
# Clocking Wizard supplies the 100 MHz primary clock constraint on sys_clk_p.
set_property -dict {PACKAGE_PIN AA25 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]
set_property -dict {PACKAGE_PIN AA17 IOSTANDARD LVCMOS33} [get_ports ad_clk]
set_property -dict {PACKAGE_PIN AE18 IOSTANDARD LVCMOS33} [get_ports ad_otr]
set_property -dict {PACKAGE_PIN AD13 IOSTANDARD LVCMOS33} [get_ports {ad_data[0]}]
set_property -dict {PACKAGE_PIN AC16 IOSTANDARD LVCMOS33} [get_ports {ad_data[1]}]
set_property -dict {PACKAGE_PIN AD14 IOSTANDARD LVCMOS33} [get_ports {ad_data[2]}]
set_property -dict {PACKAGE_PIN AC17 IOSTANDARD LVCMOS33} [get_ports {ad_data[3]}]
set_property -dict {PACKAGE_PIN AB16 IOSTANDARD LVCMOS33} [get_ports {ad_data[4]}]
set_property -dict {PACKAGE_PIN AB14 IOSTANDARD LVCMOS33} [get_ports {ad_data[5]}]
set_property -dict {PACKAGE_PIN AB17 IOSTANDARD LVCMOS33} [get_ports {ad_data[6]}]
set_property -dict {PACKAGE_PIN AB15 IOSTANDARD LVCMOS33} [get_ports {ad_data[7]}]

# Bring-up: 10 MHz launch clock, sample 180 degrees later (50 ns).
# Budget 0..35 ns from the forwarded ADC clock to returned data, including
# the ADC's documented 25 ns valid delay plus board/connector allowance.
# This is a bring-up timing budget, not a measured guarantee for jumper wires.
create_generated_clock -name ad_forward_clk -source [get_pins u_adc_clk_out/C] -divide_by 1 [get_ports ad_clk]
set_input_delay -clock ad_forward_clk -max 35.000 [get_ports {ad_data[*] ad_otr}]
set_input_delay -clock ad_forward_clk -min 0.000 [get_ports {ad_data[*] ad_otr}]

# PS FCLK and the independent PL oscillator have no fixed phase relationship.
set_clock_groups -asynchronous \
    -group [get_clocks -include_generated_clocks -of_objects [get_ports sys_clk_p]] \
    -group [get_clocks -of_objects [get_pins u_bd/capture_bd_i/processing_system7_0/FCLK_CLK0]]

# AD9708 setup 2ns/hold1.5ns + conservative board allowance. Validate compatible silicon on board.
set_property -dict {PACKAGE_PIN AA13 IOSTANDARD LVCMOS33} [get_ports {da_clk}]
set_property -dict {PACKAGE_PIN AK25 IOSTANDARD LVCMOS33} [get_ports {da_data[0]}]
set_property -dict {PACKAGE_PIN AC22 IOSTANDARD LVCMOS33} [get_ports {da_data[1]}]
set_property -dict {PACKAGE_PIN AJ25 IOSTANDARD LVCMOS33} [get_ports {da_data[2]}]
set_property -dict {PACKAGE_PIN AC23 IOSTANDARD LVCMOS33} [get_ports {da_data[3]}]
set_property -dict {PACKAGE_PIN AF17 IOSTANDARD LVCMOS33} [get_ports {da_data[4]}]
set_property -dict {PACKAGE_PIN W23 IOSTANDARD LVCMOS33} [get_ports {da_data[5]}]
set_property -dict {PACKAGE_PIN AF18 IOSTANDARD LVCMOS33} [get_ports {da_data[6]}]
set_property -dict {PACKAGE_PIN AC21 IOSTANDARD LVCMOS33} [get_ports {da_data[7]}]
create_generated_clock -name da_forward_clk -source [get_pins u_dac_clk_out/C] -invert -divide_by 1 [get_ports da_clk]
set_output_delay -clock da_forward_clk -max 3.000 [get_ports {da_data[*]}]
set_output_delay -clock da_forward_clk -min -2.500 [get_ports {da_data[*]}]
