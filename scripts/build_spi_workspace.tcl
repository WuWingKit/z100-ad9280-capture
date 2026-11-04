set root [file normalize [file join [file dirname [info script]] ..]]
setws [file join $root sdk_spi_workspace]
createhw -name z100_hw -hwspec [file join $root hardware system.hdf]
createbsp -name spi_bsp -hwproject z100_hw -proc ps7_cortexa9_0 -os standalone
configbsp -bsp spi_bsp stdin ps7_uart_0
configbsp -bsp spi_bsp stdout ps7_uart_0
regenbsp -bsp spi_bsp
createapp -name spi_uart -app {Empty Application} -hwproject z100_hw -bsp spi_bsp -proc ps7_cortexa9_0 -os standalone -lang c
importsources -name spi_uart -path [file join $root software]
projects -build
file copy -force [file join $root sdk_spi_workspace spi_uart Debug spi_uart.elf] [file join $root hardware spi_uart.elf]
puts {CLEAN_SPI_SOFTWARE_PASS}
exit
