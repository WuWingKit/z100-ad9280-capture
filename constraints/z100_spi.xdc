# Board oscillator and reset only. SPI uses internal PS EMIO, not connector pins.
set_property -dict {PACKAGE_PIN D9 IOSTANDARD DIFF_SSTL135} [get_ports sys_clk_p]
set_property -dict {PACKAGE_PIN D8 IOSTANDARD DIFF_SSTL135} [get_ports sys_clk_n]
set_property -dict {PACKAGE_PIN AA25 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]
# Primary input period is supplied by the Clocking Wizard IP constraints.
set_clock_groups -asynchronous \
    -group [get_clocks -include_generated_clocks -of_objects [get_ports sys_clk_p]] \
    -group [get_clocks -of_objects [get_pins u_bd/capture_bd_i/processing_system7_0/FCLK_CLK0]]
