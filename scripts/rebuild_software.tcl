set root [file normalize [file join [file dirname [info script]] ..]]
file copy -force [file join $root software main.c] [file join $root sdk_workspace adc_uart src main.c]
setws [file join $root sdk_workspace]
projects -build
file copy -force [file join $root sdk_workspace adc_uart Debug adc_uart.elf] [file join $root hardware adc_uart.elf]
puts {Z100_SOFTWARE_REBUILD_PASS}
exit
