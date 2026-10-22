set root [file normalize [file join [file dirname [info script]] ..]]
setws [file join $root sdk_workspace]
createhw -name z100_hw -hwspec [file join $root hardware system.hdf]
createbsp -name adc_bsp -hwproject z100_hw -proc ps7_cortexa9_0 -os standalone
configbsp -bsp adc_bsp stdin ps7_uart_0
configbsp -bsp adc_bsp stdout ps7_uart_0
regenbsp -bsp adc_bsp
createapp -name adc_uart -app {Empty Application} -hwproject z100_hw -bsp adc_bsp -proc ps7_cortexa9_0 -os standalone -lang c
importsources -name adc_uart -path [file join $root software]
projects -build
if {![file exists [file join $root sdk_workspace adc_uart Debug adc_uart.elf]]} {error "Application ELF missing"}
puts {Z100_SOFTWARE_BUILD_PASS}
exit
