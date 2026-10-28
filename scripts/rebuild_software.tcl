# ===== 用途与执行边界 =====
# 将software/main.c同步到SDK应用目录，重新编译并复制ELF。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

set root [file normalize [file join [file dirname [info script]] ..]]
file copy -force [file join $root software main.c] [file join $root sdk_workspace adc_uart src main.c]
setws [file join $root sdk_workspace]
# 编译ARM应用；这一步不会下载到开发板
projects -build
file copy -force [file join $root sdk_workspace adc_uart Debug adc_uart.elf] [file join $root hardware adc_uart.elf]
puts {Z100_SOFTWARE_REBUILD_PASS}
exit
