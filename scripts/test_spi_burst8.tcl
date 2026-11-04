set root [file normalize [file join [file dirname [info script]] ..]]
create_project spi_burst [file join $root spi_burst] -part xc7z035ffg900-2 -force
add_files [file join $root rtl spi_master_burst8.v]
add_files -fileset sim_1 [list [file join $root sim spi_slave_model.v] [file join $root sim tb_spi_burst8.v]]
set_property top tb_spi_burst8 [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
launch_simulation -mode behavioral
open_vcd [file join $root evidence spi_burst8_timing.vcd]
log_vcd [get_objects /tb_spi_burst8/dut/*]
run all
close_vcd
close_sim
file copy -force [file join $root spi_burst spi_burst.sim sim_1 behav xsim spi_burst8_result.txt] [file join $root evidence spi_burst8_result.txt]
close_project
