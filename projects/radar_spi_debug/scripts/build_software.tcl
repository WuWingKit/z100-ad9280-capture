set root [file normalize [file join [file dirname [info script]] ..]]
set workspace E:/FPGAWork/radar_spi_debug_sdk_20261112
setws $workspace
createhw -name debug_hw -hwspec [file join $root hardware radar_debug.hdf]
createbsp -name debug_bsp -hwproject debug_hw -proc ps7_cortexa9_0 -os standalone
configbsp -bsp debug_bsp stdin ps7_uart_0
configbsp -bsp debug_bsp stdout ps7_uart_0
regenbsp -bsp debug_bsp
createapp -name radar_debug_uart -app {Empty Application} -hwproject debug_hw -bsp debug_bsp -proc ps7_cortexa9_0 -os standalone -lang c
importsources -name radar_debug_uart -path [file join $root software]
projects -build
file copy -force [file join $workspace radar_debug_uart Debug radar_debug_uart.elf] [file join $root hardware radar_debug_uart.elf]
puts {FPGA_ONLY_DEBUG_SOFTWARE_PASS}
exit
