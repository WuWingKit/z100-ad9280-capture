set root [file normalize [file join [file dirname [info script]] ..]]
create_project sync_unit [file join $root unit_project] -part xc7z035ffg900-2 -force
foreach f {spi_slave_rx16.v radar_frame_capture.v radar_fifo_to_axis.v} {
    add_files -norecurse [list [file join $root rtl $f]]
}
add_files -fileset sim_1 -norecurse [list [file join $root sim mcu_pingpong_model.v] [file join $root sim tb_radar_sync.v]]
set_property top tb_radar_sync [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
set result [file join $root unit_project sync_unit.sim sim_1 behav xsim radar_sync_unit_result.txt]
if {[file exists $result]} {file delete $result}
launch_simulation -mode behavioral
log_wave -r /
open_vcd [file join $root evidence radar_sync_unit.vcd]
log_vcd [get_objects /tb_radar_sync/dut/*]
run all
close_vcd
close_sim
set result [file join $root unit_project sync_unit.sim sim_1 behav xsim radar_sync_unit_result.txt]
if {![file exists $result]} {error "Unit simulation did not finish"}
file copy -force $result [file join $root evidence radar_sync_unit_result.txt]
puts {RADAR_SYNC_UNIT_COMPLETE}
close_project
