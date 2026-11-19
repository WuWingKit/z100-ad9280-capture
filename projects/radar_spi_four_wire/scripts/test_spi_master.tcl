set root [file normalize [file join [file dirname [info script]] ..]]
create_project spi_master [file join $root evidence diagnostic_20261119 sim_project] -part xc7z035ffg900-2 -force
add_files [list [file join $root rtl radar_frame_capture.v] [file join $root rtl spi_slave_rx16.v]]
add_files -fileset sim_1 [list [file join $root sim tb_spi_master.v]]
set_property top tb_spi_master [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
launch_simulation -mode behavioral
run all
close_sim
close_project

