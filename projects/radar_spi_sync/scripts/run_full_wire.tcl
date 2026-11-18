set root [file normalize [file join [file dirname [info script]] ..]]
create_project full_wire [file join $root full_wire_project] -part xc7z035ffg900-2 -force
foreach f {spi_slave_rx16.v radar_frame_capture.v radar_fifo_to_axis.v} {
    add_files -norecurse [list [file join $root rtl $f]]
}
add_files -fileset sim_1 -norecurse [list [file join $root sim mcu_pingpong_model.v] [file join $root sim tb_radar_full_wire.v]]
set_property top tb_radar_full_wire [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
set result [file join $root full_wire_project full_wire.sim sim_1 behav xsim radar_full_wire_result.txt]
if {[file exists $result]} {file delete $result}
file mkdir [file dirname $result]
file copy -force [file join $root evidence mcu_reference_words.hex] [file join [file dirname $result] mcu_reference_words.hex]
launch_simulation -mode behavioral
run all
close_sim
if {![file exists $result]} {error "Full SPI wire simulation did not finish"}
file copy -force $result [file join $root evidence radar_full_wire_result.txt]
puts {RADAR_FULL_WIRE_COMPLETE}
close_project
