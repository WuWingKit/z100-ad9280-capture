# Volatile FPGA/PS debug load. No Flash and no automatic SPI transfers.
set root [file normalize [file join [file dirname [info script]] ..]]
connect -url tcp:127.0.0.1:3121
if {[llength [targets -target-properties -filter {name =~ "xc7z035*"}]] != 1} {error "Expected one XC7Z035"}
targets -set -filter {name =~ "ARM*#0"}
stop
rst -system
after 1000
targets -set -filter {name =~ "xc7z035*"}
fpga -file [file join $root hardware spi_only_master.bit]
targets -set -filter {name =~ "ARM*#0"}
source [file join $root hardware ps7_init.tcl]
ps7_init
ps7_post_config
dow [file join $root hardware spi_only_ps.elf]
con
puts SPI_ONLY_MASTER_RUNNING_NO_AUTO_TRANSFER
 disconnect
exit
