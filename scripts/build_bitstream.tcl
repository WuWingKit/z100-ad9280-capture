# ===== 用途与执行边界 =====
# 重新综合/实现并检查建立和保持时序；通过后生成bit与HDF，不操作开发板。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

set root [file normalize [file join [file dirname [info script]] ..]]
# 1. 打开现有副本工程
open_project [file join $root bd_start.xpr]
# 2. 清除旧综合结果，避免把旧bit误当成本次源码的结果
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Synthesis failed"}
# 3. 布局布线后才能检查实际物理路径时序
launch_runs impl_1 -to_step route_design -jobs 4
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] ne "100%"} {error "Implementation failed"}
open_run impl_1
# 4. 保存时序/DRC/CDC报告；CDC需结合稳定参数和复位协议审阅
report_timing_summary -report_unconstrained -file [file join $root hardware timing_routed.rpt]
report_drc -file [file join $root hardware drc_routed.rpt]
report_cdc -details -file [file join $root hardware cdc_routed.rpt]
report_io -file [file join $root hardware io_routed.rpt]
foreach typ {max min} {
    set paths [get_timing_paths -delay_type $typ -max_paths 1]
    if {![llength $paths]} {error "No timing paths available for $typ check"}
    set slack [get_property SLACK [lindex $paths 0]]
    puts "Z100_TIMING_$typ slack=$slack"
    if {$slack < 0} {error "Timing failed; bitstream generation stopped"}
}
close_design
# 5. 时序通过后生成最终配置文件
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1
set bit [file join $root bd_start.runs impl_1 z100_adc_top.bit]
if {![file exists $bit]} {error "Bitstream generation failed"}
file copy -force $bit [file join $root hardware z100_cordic.bit]
write_sysdef -hwdef [file join $root hardware system.hdf] -bitfile $bit -file [file join $root hardware z100_cordic.hdf] -force
puts {Z100_BITSTREAM_PASS}
close_project

