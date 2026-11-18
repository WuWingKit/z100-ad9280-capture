set root [file normalize [file join [file dirname [info script]] ..]]
set workspace E:/FPGAWork/radar_sync_sdk_20261111
setws $workspace
updatehw -hw radar_hw -newhwspec [file join $root hardware radar_sync.hdf]
regenbsp -bsp radar_bsp
file copy -force [file join $root software main.c] [file join $workspace radar_uart src main.c]
projects -build
file copy -force [file join $workspace radar_uart Debug radar_uart.elf] [file join $root hardware radar_uart.elf]
puts {RADAR_LATEST_HDF_AND_ELF_BUILT}
exit
