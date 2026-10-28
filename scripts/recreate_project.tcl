# Recreate from a source-only Git checkout. Vivado 2018.3, no board access.
set root [file normalize [file join [file dirname [info script]] ..]]
create_project bd_start [file join $root _rebuild_cordic] -part xc7z035ffg900-2 -force
set_property target_language Verilog [current_project]
add_files -norecurse [list [file join $root rtl measurement_core.v] [file join $root rtl fifo_to_axis.v] [file join $root rtl z100_adc_top.v]]
add_files -norecurse [file join $root bd_start.srcs sources_1 ip adc_clk_wiz adc_clk_wiz.xci]
add_files -norecurse [file join $root bd_start.srcs sources_1 bd capture_bd capture_bd.bd]
add_files -norecurse [file join $root bd_start.srcs sources_1 bd capture_bd hdl capture_bd_wrapper.v]
add_files -fileset constrs_1 -norecurse [file join $root constraints z100_adc.xdc]
set_property top z100_adc_top [get_filesets sources_1]
open_bd_design [get_files -all */capture_bd.bd]
validate_bd_design
generate_target all [get_files -all */capture_bd.bd]
generate_target all [get_ips adc_clk_wiz]
add_files -fileset sim_1 -norecurse [file join $root sim tb_loop_ddr.v]
set_property top tb_loop_ddr [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
puts {CORDIC_SOURCE_RECREATE_PASS}
close_project
