# ===== 用途与执行边界 =====
# 从Git源码创建_rebuild_cordic工程并再生IP；不依赖提交生成的IP网表。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

# Recreate from a source-only Git checkout. Vivado 2018.3, no board access.
set root [file normalize [file join [file dirname [info script]] ..]]
# 1. 创建独立工程目录
create_project bd_start [file join $root _rebuild_cordic] -part xc7z035ffg900-2 -force
set_property target_language Verilog [current_project]
add_files -norecurse [list [file join $root rtl measurement_core.v] [file join $root rtl fifo_to_axis.v] [file join $root rtl z100_adc_top.v]]
add_files -norecurse [file join $root bd_start.srcs sources_1 ip adc_clk_wiz adc_clk_wiz.xci]
add_files -norecurse [file join $root bd_start.srcs sources_1 bd capture_bd capture_bd.bd]
add_files -norecurse [file join $root bd_start.srcs sources_1 bd capture_bd hdl capture_bd_wrapper.v]
add_files -fileset constrs_1 -norecurse [file join $root constraints z100_adc.xdc]
set_property top z100_adc_top [get_filesets sources_1]
open_bd_design [get_files -all */capture_bd.bd]
# 检查BD接口/地址/时钟连接；不代表真实模拟电路已验证
validate_bd_design
# 再生成厂商IP仿真/综合文件，不手工修改这些自动生成文件
generate_target all [get_files -all */capture_bd.bd]
generate_target all [get_ips adc_clk_wiz]
add_files -fileset sim_1 -norecurse [file join $root sim tb_loop_ddr.v]
set_property top tb_loop_ddr [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
puts {CORDIC_SOURCE_RECREATE_PASS}
close_project
