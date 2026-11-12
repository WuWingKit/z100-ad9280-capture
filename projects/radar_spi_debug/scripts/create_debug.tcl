set root [file normalize [file join [file dirname [info script]] ..]]
if {[file exists [file join $root radar_debug.xpr]]} {
    open_project [file join $root radar_debug.xpr]
} else {create_project radar_debug $root -part xc7z035ffg900-2}
set_property target_language Verilog [current_project]
foreach f {spi_slave_rx16.v radar_frame_capture.v radar_fifo_to_axis.v radar_frontend_sim.v z100_radar_debug_top.v} {
    add_files -norecurse [list [file join $root rtl $f]]
}
set bd_dir [file join $root radar_debug.srcs sources_1 bd capture_bd]
file mkdir $bd_dir
if {![file exists [file join $bd_dir capture_bd.bd]]} {file copy [file join $root seed capture_bd.bd] [file join $bd_dir capture_bd.bd]}
add_files -norecurse [list [file join $bd_dir capture_bd.bd]]
open_bd_design [get_files -all */capture_bd.bd]
update_module_reference [get_ips -all *radar_frame_capture*]
update_module_reference [get_ips -all *radar_fifo_to_axis*]
create_bd_cell -type module -reference radar_frontend_sim frontend_sim_0
foreach port {spi_sclk spi_cs_n spi_mosi mcu_ready radar_sync} {delete_bd_objs [get_bd_ports $port]}
foreach pair {
 {frontend_sim_0/clk processing_system7_0/FCLK_CLK0}
 {frontend_sim_0/rst_n radar_frame_capture_0/axis_reset_n}
 {frontend_sim_0/sync radar_frame_capture_0/radar_sync}
 {frontend_sim_0/command axi_gpio_0/gpio_io_o}
 {frontend_sim_0/ready radar_frame_capture_0/mcu_ready}
 {frontend_sim_0/spi_sclk radar_frame_capture_0/spi_sclk}
 {frontend_sim_0/spi_cs_n radar_frame_capture_0/spi_cs_n}
 {frontend_sim_0/spi_mosi radar_frame_capture_0/spi_mosi}
} {
    connect_bd_net [get_bd_pins [lindex $pair 0]] [get_bd_pins [lindex $pair 1]]
}
source [file join $root scripts add_debug_ila.tcl]
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -sync -force -quiet
make_wrapper -files [get_files -all */capture_bd.bd] -top
add_files -norecurse [list [file join $bd_dir hdl capture_bd_wrapper.v]]
add_files -fileset constrs_1 -norecurse [list [file join $root constraints board_base.xdc]]
set_property top z100_radar_debug_top [get_filesets sources_1]
update_compile_order -fileset sources_1
write_hwdef -force -file [file join $root hardware radar_debug.hdf]
write_bd_tcl -force [file join $root scripts capture_bd_export.tcl]
puts {FPGA_ONLY_DEBUG_PROJECT_CREATED}
close_project
