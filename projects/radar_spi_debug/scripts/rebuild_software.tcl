set root [file normalize [file join [file dirname [info script]] ..]]
set workspace E:/FPGAWork/radar_spi_debug_sdk_20261112
setws $workspace
updatehw -hw debug_hw -newhwspec [file join $root hardware radar_debug.hdf]
regenbsp -bsp debug_bsp
file copy -force [file join $root software main.c] [file join $workspace radar_debug_uart src main.c]
projects -build
file copy -force [file join $workspace radar_debug_uart Debug radar_debug_uart.elf] [file join $root hardware radar_debug_uart.elf]
puts {FPGA_ONLY_DEBUG_SOFTWARE_PASS}
exit
