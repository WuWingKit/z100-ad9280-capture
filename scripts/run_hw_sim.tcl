set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root bd_start.xpr]
if {![llength [get_files -quiet *tb_z100_hw.v]]} {
    add_files -fileset sim_1 -norecurse [list [file join $root sim tb_z100_hw.v]]
}
set_property top tb_z100_hw [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sim_1
launch_simulation -mode behavioral
log_wave /tb_z100_hw/ad_clk /tb_z100_hw/ad_data /tb_z100_hw/frame_id /tb_z100_hw/sampled /tb_z100_hw/words /tb_z100_hw/stalls /tb_z100_hw/status /tb_z100_hw/gpio_status
run all
close_sim
set result [file join $root bd_start.sim sim_1 behav xsim z100_sim_result.txt]
if {![file exists $result]} {error "Simulation did not produce a PASS result"}
set f [open $result r]
puts [read $f]
close $f
file copy -force $result [file join $root hardware z100_sim_result.txt]
close_project
