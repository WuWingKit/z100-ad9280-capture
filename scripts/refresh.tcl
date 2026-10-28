# ===== 用途与执行边界 =====
# 修改自定义RTL后刷新BD模块引用、校验连接并重新生成输出产品。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

set root [file normalize [file join [file dirname [info script]] ..]]
# 1. 打开现有副本工程
open_project [file join $root bd_start.xpr]
open_bd_design [get_files -all */capture_bd.bd]
update_module_reference [get_ips capture_bd_measurement_core_0_0]
if {[llength [get_bd_cells -quiet util_vector_logic_0]]} {delete_bd_objs [get_bd_cells util_vector_logic_0]}
# 检查BD接口/地址/时钟连接；不代表真实模拟电路已验证
validate_bd_design
save_bd_design
# 再生成厂商IP仿真/综合文件，不手工修改这些自动生成文件
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -no_script -sync -force
write_hwdef -force -file [file join $root hardware system.hdf]
close_project

