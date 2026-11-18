set root [file normalize [file join [file dirname [info script]] ..]]
set workspace E:/FPGAWork/radar_sync_sdk_20261111
setws $workspace
file copy -force [file join $root software main.c] [file join $workspace radar_uart src main.c]
projects -build -type app -name radar_uart
file copy -force [file join $workspace radar_uart Debug radar_uart.elf] [file join $root hardware radar_uart.elf]
puts {RADAR_SYNC_SOFTWARE_REBUILT}
exit
