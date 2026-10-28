# ===== 用途与执行边界 =====
# 开发辅助：查询CORDIC可配置属性；不是测量主流程。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

set root [file normalize [file join [file dirname [info script]] ..]]
# 1. 创建独立工程目录
create_project probe [file join $root _probe] -part xc7z035ffg900-2
create_ip -name cordic -vendor xilinx.com -library ip -version 6.0 -module_name cordic_sincos
set f [open [file join $root evidence cordic_properties.txt] w]
foreach p [list_property [get_ips cordic_sincos]] {
 if {[string match CONFIG.* $p]} {puts $f "$p=[get_property $p [get_ips cordic_sincos]]"}
}
close $f
close_project
