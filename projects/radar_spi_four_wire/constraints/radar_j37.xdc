# Z100 XC7Z035FFG900 J37, official base schematic sheet 18 (PDF page 19).
# SPI-only slave P4=CS, P3=SCK, P6=MOSI, P5=MISO.
# mcu_ready is the legacy INTERNAL FPGA port name for MISO on AA24.
# radar_sync is unused: do not connect its AG25 pin.
# J37 is unused, as confirmed by the user. Never connect J37 pin1 (+5V).
# Both boards have their own power supplies; connect GND using J37 pin2 or4.
set_property -dict {PACKAGE_PIN AC24 IOSTANDARD LVCMOS33 PULLDOWN TRUE} [get_ports spi_sclk]
set_property -dict {PACKAGE_PIN AD24 IOSTANDARD LVCMOS33 PULLUP TRUE} [get_ports spi_cs_n]
set_property -dict {PACKAGE_PIN AG24 IOSTANDARD LVCMOS33 PULLDOWN TRUE} [get_ports spi_mosi]
set_property -dict {PACKAGE_PIN AG25 IOSTANDARD LVCMOS33 DRIVE 4 SLEW SLOW} [get_ports radar_sync]
set_property -dict {PACKAGE_PIN AA24 IOSTANDARD LVCMOS33 PULLDOWN TRUE} [get_ports mcu_ready]

# These pins enter dedicated three-stage ASYNC_REG synchronizers. SCLK is DATA
# sampled by FCLK0 (100MHz), not a clock routed into the fabric clock network.
# Only asynchronous port-to-first-register paths are excluded. Register-to-
# register synchronizer paths remain timed at 100MHz.
set_false_path -from [get_ports mcu_ready]
# Board reset asserts asynchronously; reset controllers synchronize release.
set_false_path -from [get_ports sys_rst_n]
# SYNC is an asynchronous event into MCU GPIO, with a controlled 10us pulse.
# There is no synchronous source/capture clock relation for this output.
set_false_path -to [get_ports radar_sync]
