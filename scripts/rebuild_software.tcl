set root [file normalize [file join [file dirname [info script]] ..]]
setws [file join $root sdk_workspace]
file copy -force [file join $root software main.c] [file join $root sdk_workspace adc_uart src main.c]
projects -build -type app -name adc_uart
exit
