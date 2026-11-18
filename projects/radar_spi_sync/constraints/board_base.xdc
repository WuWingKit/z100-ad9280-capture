# Known Z100 board reset only. External radar pins/electrical levels are TBD.
set_property -dict {PACKAGE_PIN AA25 IOSTANDARD LVCMOS33} [get_ports sys_rst_n]
# DO NOT assign guessed pins or bypass UCIO/NSTD checks to generate a bitstream.
# Before implementation, constrain external SPI SCLK/MOSI/CS and radar_sync
# using the confirmed connector map, bank voltage and timing requirements.
