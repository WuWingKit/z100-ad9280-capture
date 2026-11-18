set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_sync.xpr]
open_bd_design [get_files -all */capture_bd.bd]
# Interfaces are refreshed by configure_hardware.tcl when ports change.
# A simulation rerun must not unnecessarily invalidate implemented hardware.
validate_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -sync -force -quiet
foreach f [get_files -of_objects [get_filesets sim_1]] {remove_files -fileset sim_1 $f}
add_files -fileset sim_1 -norecurse [list [file join $root sim tb_radar_ddr.v] [file join $root sim mcu_pingpong_model.v]]
set_property top tb_radar_ddr [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sim_1
set result [file join $root radar_sync.sim sim_1 behav xsim radar_ddr_result.txt]
if {[file exists $result]} {file delete $result}
file mkdir [file dirname $result]
file copy -force [file join $root evidence mcu_reference_words.hex] [file join [file dirname $result] mcu_reference_words.hex]
launch_simulation -mode behavioral
run all
close_sim
if {![file exists $result]} {error "DDR simulation did not finish"}
file copy -force $result [file join $root evidence radar_ddr_result.txt]
puts {RADAR_DDR_COMPLETE}
close_project


