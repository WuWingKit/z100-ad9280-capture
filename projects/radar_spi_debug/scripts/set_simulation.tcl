set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_debug.xpr]
add_files -fileset sim_1 -norecurse [list [file join $root sim tb_radar_ddr.v]]
set_property include_dirs [list [file join $root sim]] [get_filesets sim_1]
set_property top tb_radar_ddr [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sim_1
puts {DEFAULT_DDR_SIMULATION_CONFIGURED}
close_project
