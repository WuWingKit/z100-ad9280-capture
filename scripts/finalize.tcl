# ===== 用途与执行边界 =====
# 清理孤立BD网络，校验设计并导出IP与硬件描述，不连接硬件。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

set root [file normalize [file join [file dirname [info script]] ..]]
# 1. 打开现有副本工程
open_project [file join $root bd_start.xpr]
open_bd_design [get_files -all */capture_bd.bd]
foreach n [get_bd_nets] {
 if {[llength [get_bd_pins -quiet -of_objects $n]]==0 && [llength [get_bd_ports -quiet -of_objects $n]]==0} {delete_bd_objs $n}
}
# 检查BD接口/地址/时钟连接；不代表真实模拟电路已验证
validate_bd_design
save_bd_design
# 再生成厂商IP仿真/综合文件，不手工修改这些自动生成文件
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -no_script -sync -force
write_hwdef -force -file [file join $root hardware system.hdf]
close_project
