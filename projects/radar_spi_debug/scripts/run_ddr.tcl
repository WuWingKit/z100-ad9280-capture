set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_debug.xpr]
# Testbench defparams change only simulation elaboration, not production BD.
foreach f [get_files -of_objects [get_filesets sim_1]] {remove_files -fileset sim_1 $f}
add_files -fileset sim_1 -norecurse [list [file join $root sim tb_radar_ddr.v]]
set_property include_dirs [list [file join $root sim]] [get_filesets sim_1]
set_property top tb_radar_ddr [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sim_1
set result [file join $root radar_debug.sim sim_1 behav xsim radar_ddr_result.txt]
if {[file exists $result]} {file delete $result}
launch_simulation -mode behavioral
run all
close_sim
if {![file exists $result]} {error "DDR simulation did not finish"}
file copy -force $result [file join $root evidence radar_ddr_result.txt]
puts {REAL_SOURCE_DDR_INTEGRATION_PASS}
close_project
