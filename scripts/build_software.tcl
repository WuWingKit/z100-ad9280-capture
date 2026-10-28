# ===== 用途与执行边界 =====
# 仅用于空SDK工作区首次创建平台/BSP/应用；已有工作区应使用rebuild_software.tcl。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

set root [file normalize [file join [file dirname [info script]] ..]]
setws [file join $root sdk_workspace]
createhw -name z100_hw -hwspec [file join $root hardware system.hdf]
createbsp -name adc_bsp -hwproject z100_hw -proc ps7_cortexa9_0 -os standalone
configbsp -bsp adc_bsp stdin ps7_uart_0
configbsp -bsp adc_bsp stdout ps7_uart_0
regenbsp -bsp adc_bsp
createapp -name adc_uart -app {Empty Application} -hwproject z100_hw -bsp adc_bsp -proc ps7_cortexa9_0 -os standalone -lang c
importsources -name adc_uart -path [file join $root software]
# 编译ARM应用；这一步不会下载到开发板
projects -build
if {![file exists [file join $root sdk_workspace adc_uart Debug adc_uart.elf]]} {error "Application ELF missing"}
puts {Z100_SOFTWARE_BUILD_PASS}
exit
