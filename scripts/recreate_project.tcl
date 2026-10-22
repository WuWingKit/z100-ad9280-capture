# Vivado 2018.3. Rebuild source project in _rebuild; never programs hardware.
set root [file normalize [file join [file dirname [info script]] ..]]
set dest [file join $root _rebuild]
if {[file exists [file join $dest bd_start.xpr]]} {
    error "_rebuild/bd_start.xpr already exists; open it instead of overwriting"
}
create_project bd_start $dest -part xc7z035ffg900-2
set_property target_language Verilog [current_project]
set_property simulator_language Mixed [current_project]
foreach rel {bd_start.srcs/sources_1/new/adc_capture_pack.v bd_start.srcs/sources_1/new/fifo_to_axis.v rtl/adc_board_control.v rtl/z100_adc_top.v} {
    add_files -norecurse [file join $root $rel]
}
update_compile_order -fileset sources_1
read_ip [file join $root bd_start.srcs sources_1 ip adc_clk_wiz adc_clk_wiz.xci]
add_files -norecurse [file join $root bd_start.srcs sources_1 bd capture_bd capture_bd.bd]
set bd [get_files -all */capture_bd.bd]
open_bd_design $bd
validate_bd_design
generate_target all $bd
add_files -norecurse [make_wrapper -files $bd -top]
add_files -fileset constrs_1 -norecurse [file join $root constraints z100_adc.xdc]
add_files -fileset sim_1 -norecurse [file join $root sim tb_z100_hw.v]
set_property top z100_adc_top [get_filesets sources_1]
set_property top tb_z100_hw [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sources_1
update_compile_order -fileset sim_1
puts {SOURCE_PROJECT_RECREATED}
close_project
