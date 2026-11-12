# Volatile JTAG load only. This script never programs QSPI/Flash.
set root [file normalize [file join [file dirname [info script]] ..]]
foreach name {radar_debug.bit radar_debug_uart.elf} {
    if {![file exists [file join $root hardware $name]]} {error "Missing $name"}
}
set workspace E:/FPGAWork/radar_spi_debug_sdk_20261112
set init [file join $workspace debug_hw ps7_init.tcl]
if {![file exists $init]} {error "Missing matching PS initialization script"}
connect -url tcp:127.0.0.1:3121
targets -set -filter {name =~ "ARM*#0"}
stop
rst -system
after 1000
targets -set -filter {name =~ "xc7z035*"}
fpga -file [file join $root hardware radar_debug.bit]
targets -set -filter {name =~ "ARM*#0"}
source $init
ps7_init
ps7_post_config
dow [file join $root hardware radar_debug_uart.elf]
con
puts {DEBUG_PROGRAM_RUNNING_NO_FLASH}
disconnect
exit
