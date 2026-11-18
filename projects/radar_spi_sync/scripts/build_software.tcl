set root [file normalize [file join [file dirname [info script]] ..]]
# SDK 2018.3 launches Eclipse with broken quoting for paths containing spaces.
# Only its generated build workspace is placed in a path without spaces.
set workspace E:/FPGAWork/radar_sync_sdk_20261111
setws $workspace
createhw -name radar_hw -hwspec [file join $root hardware radar_sync.hdf]
createbsp -name radar_bsp -hwproject radar_hw -proc ps7_cortexa9_0 -os standalone
configbsp -bsp radar_bsp stdin ps7_uart_0
configbsp -bsp radar_bsp stdout ps7_uart_0
regenbsp -bsp radar_bsp
createapp -name radar_uart -app {Empty Application} -hwproject radar_hw -bsp radar_bsp -proc ps7_cortexa9_0 -os standalone -lang c
importsources -name radar_uart -path [file join $root software]
projects -build
file copy -force [file join $workspace radar_uart Debug radar_uart.elf] [file join $root hardware radar_uart.elf]
puts {RADAR_SYNC_SOFTWARE_BUILT}
exit
