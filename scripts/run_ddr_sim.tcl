# SPI-only simulation entry; no board access.

set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root spi_capture.xpr]
foreach f [get_files -of_objects [get_filesets sim_1]] {remove_files -fileset sim_1 $f}
add_files -fileset sim_1 -norecurse [list [file join $root sim tb_spi_ddr.v] [file join $root sim spi_slave_model.v]]
set_property top tb_spi_ddr [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sim_1
set result [file join $root spi_capture.sim sim_1 behav xsim ddr_result.txt]
if {[file exists $result]} {file delete $result}
set dir [file join $root spi_capture.sim sim_1 behav xsim]
file mkdir $dir
file copy -force [file join $root sim sine_lut.hex] [file join $dir sine_lut.hex]
launch_simulation -mode behavioral
run all
close_sim
set result [file join $root spi_capture.sim sim_1 behav xsim ddr_result.txt]
if {![file exists $result]} {error "DDR simulation did not finish"}
file copy -force $result [file join $root evidence ddr_result.txt]
close_project
