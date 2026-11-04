# SPI-only simulation entry; no board access.

set root [file normalize [file join [file dirname [info script]] ..]]
create_project simcheck [file join $root sim_project] -part xc7z035ffg900-2 -force
add_files -norecurse [list [file join $root rtl spi_capture_core.v] [file join $root rtl fifo_to_axis.v] [file join $root rtl spi_master_burst8.v]]
create_ip -name fifo_generator -vendor xilinx.com -library ip -version 13.2 -module_name sim_fifo
set_property -dict [list CONFIG.Fifo_Implementation Independent_Clocks_Builtin_FIFO \
 CONFIG.Input_Data_Width 32 CONFIG.Output_Data_Width 32 CONFIG.Input_Depth 1024 \
 CONFIG.Performance_Options First_Word_Fall_Through] [get_ips sim_fifo]
generate_target all [get_ips]
add_files -fileset sim_1 -norecurse [list [file join $root sim tb_spi_capture.v] [file join $root sim spi_slave_model.v]]
set_property top tb_spi_capture [get_filesets sim_1]
set_property xsim.simulate.runtime 0ns [get_filesets sim_1]
update_compile_order -fileset sim_1
set dir [file join $root sim_project simcheck.sim sim_1 behav xsim]
foreach f {spi_capture_result.txt loopback_simulated.csv} {
 if {[file exists [file join $dir $f]]} {file delete [file join $dir $f]}
}
file mkdir $dir
file copy -force [file join $root sim sine_lut.hex] [file join $dir sine_lut.hex]
launch_simulation -mode behavioral
run all
close_sim
set dir [file join $root sim_project simcheck.sim sim_1 behav xsim]
foreach f {spi_capture_result.txt loopback_simulated.csv} {
 if {![file exists [file join $dir $f]]} {error "Simulation missing $f"}
 file copy -force [file join $dir $f] [file join $root evidence $f]
}
puts {MEASUREMENT_SIMULATION_COMPLETE}
close_project
