set root [file normalize [file join [file dirname [info script]] ..]]
create_project source_test [file join $root source_test_project] -part xc7z035ffg900-2 -force
foreach f {spi_slave_rx16.v radar_frame_capture.v radar_fifo_to_axis.v radar_frontend_sim.v} {
    add_files -norecurse [list [file join $root rtl $f]]
}
add_files -fileset sim_1 -norecurse [list [file join $root sim tb_frontend_debug.v]]
set_property include_dirs [list [file join $root sim]] [get_filesets sim_1]
set_property top tb_frontend_debug [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
set directory [file join $root source_test_project source_test.sim sim_1 behav xsim]
foreach full {0 1} {
    set_property generic FULL=$full [get_filesets sim_1]
    set name [expr {$full ? "full_wire_result.txt" : "unit_result.txt"}]
    set result [file join $directory $name]
    if {[file exists $result]} {file delete $result}
    launch_simulation -mode behavioral
    if {!$full} {log_wave -r /tb_frontend_debug/source/* /tb_frontend_debug/core/status /tb_frontend_debug/command}
    run all
    close_sim
    if {![file exists $result]} {error "Source test failed: $name"}
    file copy -force $result [file join $root evidence $name]
}
puts {ALL_SOURCE_TESTS_PASS}
close_project
