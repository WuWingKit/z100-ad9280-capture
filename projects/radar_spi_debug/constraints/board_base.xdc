# FPGA-only debug: no connector SPI/SYNC/READY signals are exported.
# Z100 reset button is active low on AA25, same as the verified board design.
set_property -dict {PACKAGE_PIN AA25 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]
# Reset assertion is asynchronous; release is synchronized by proc_sys_reset.
set_false_path -from [get_ports sys_rst_n]
