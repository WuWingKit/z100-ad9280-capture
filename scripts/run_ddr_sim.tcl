# ===== 用途与执行边界 =====
# 主工程完整BD仿真：PS VIP配置GPIO和DMA，检查实际DDR模型读回结果。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

set root [file normalize [file join [file dirname [info script]] ..]]
# 1. 打开现有副本工程
open_project [file join $root bd_start.xpr]
foreach f [get_files -of_objects [get_filesets sim_1]] {remove_files -fileset sim_1 $f}
add_files -fileset sim_1 -norecurse [file join $root sim tb_loop_ddr.v]
set_property top tb_loop_ddr [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sim_1
set result [file join $root bd_start.sim sim_1 behav xsim ddr_result.txt]
if {[file exists $result]} {file delete $result}
# 启动XSim；run all执行testbench直至完成或失败
launch_simulation -mode behavioral
run all
close_sim
set result [file join $root bd_start.sim sim_1 behav xsim ddr_result.txt]
if {![file exists $result]} {error "DDR simulation did not finish"}
file copy -force $result [file join $root evidence ddr_result.txt]
close_project
