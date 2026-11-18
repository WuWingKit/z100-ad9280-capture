set root [file normalize [file join [file dirname [info script]] ..]]
if {[file exists [file join $root radar_sync.xpr]]} {
    open_project [file join $root radar_sync.xpr]
} else {
    create_project radar_sync $root -part xc7z035ffg900-2
}
set_property target_language Verilog [current_project]
foreach f {spi_master_burst8.v fifo_to_axis.v spi_capture_core.v} {
    add_files -norecurse [list [file join $root seed rtl $f]]
}
foreach f {spi_slave_rx16.v radar_frame_capture.v radar_fifo_to_axis.v z100_radar_top.v} {
    add_files -norecurse [list [file join $root rtl $f]]
}
set bd_dir [file join $root radar_sync.srcs sources_1 bd capture_bd]
file mkdir $bd_dir
if {![file exists [file join $bd_dir capture_bd.bd]]} {file copy [file join $root seed capture_bd.bd] [file join $bd_dir capture_bd.bd]}
add_files -norecurse [list [file join $bd_dir capture_bd.bd]]
open_bd_design [get_files -all */capture_bd.bd]
delete_bd_objs [get_bd_cells {spi_capture_core_0 fifo_to_axis_0}]
delete_bd_objs [get_bd_ports spi_clk16]
set_property CONFIG.PCW_SPI0_PERIPHERAL_ENABLE {0} [get_bd_cells processing_system7_0]
set_property CONFIG.c_sg_length_width {16} [get_bd_cells axi_dma_0]
create_bd_cell -type module -reference radar_frame_capture radar_frame_capture_0
create_bd_cell -type module -reference radar_fifo_to_axis radar_fifo_to_axis_0
foreach pair {
 {radar_frame_capture_0/axis_clk processing_system7_0/FCLK_CLK0}
 {radar_fifo_to_axis_0/axis_clk processing_system7_0/FCLK_CLK0}
 {fifo_generator_0/wr_clk processing_system7_0/FCLK_CLK0}
 {fifo_generator_0/rd_clk processing_system7_0/FCLK_CLK0}
 {radar_frame_capture_0/rst_n proc_sys_reset_0/peripheral_aresetn}
 {radar_frame_capture_0/command axi_gpio_0/gpio_io_o}
 {radar_frame_capture_0/status axi_gpio_0/gpio2_io_i}
 {radar_frame_capture_0/prime_wait_cycles axi_gpio_config/gpio_io_o}
 {radar_frame_capture_0/timeout_cycles axi_gpio_config/gpio2_io_o}
 {radar_frame_capture_0/fifo_full fifo_generator_0/full}
 {radar_frame_capture_0/fifo_din fifo_generator_0/din}
 {radar_frame_capture_0/fifo_wr_en fifo_generator_0/wr_en}
 {radar_frame_capture_0/fifo_reset fifo_generator_0/rst}
 {radar_frame_capture_0/axis_reset_n radar_fifo_to_axis_0/rst_n}
 {radar_frame_capture_0/frame_bytes_axis radar_fifo_to_axis_0/frame_bytes}
 {radar_frame_capture_0/stream_done radar_fifo_to_axis_0/stream_done}
 {radar_fifo_to_axis_0/fifo_dout fifo_generator_0/dout}
 {radar_fifo_to_axis_0/fifo_empty fifo_generator_0/empty}
 {radar_fifo_to_axis_0/fifo_rd_en fifo_generator_0/rd_en}
} {
    set pa [get_bd_pins [lindex $pair 0]]
    set pb [get_bd_pins [lindex $pair 1]]
    set na [get_bd_nets -quiet -of_objects $pa]
    set nb [get_bd_nets -quiet -of_objects $pb]
    if {[llength $na] && [llength $nb] && $na eq $nb} {continue}
    if {[llength $na] && [llength $nb]} {disconnect_bd_net $na $pa}
    if {[llength $nb]} {connect_bd_net -net $nb $pa $pb} else {connect_bd_net $pa $pb}
}
connect_bd_intf_net [get_bd_intf_pins radar_fifo_to_axis_0/m_axis] [get_bd_intf_pins axi_dma_0/S_AXIS_S2MM]
foreach name {spi_sclk spi_cs_n spi_mosi mcu_ready} {
    create_bd_port -dir I $name
    connect_bd_net [get_bd_ports $name] [get_bd_pins radar_frame_capture_0/$name]
}
create_bd_port -dir O radar_sync
connect_bd_net [get_bd_ports radar_sync] [get_bd_pins radar_frame_capture_0/radar_sync]
foreach f {spi_master_burst8.v fifo_to_axis.v spi_capture_core.v} {
    remove_files [get_files -all [list [file join $root seed rtl $f]]]
}
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -sync -force -quiet
make_wrapper -files [get_files -all */capture_bd.bd] -top
add_files -norecurse [list [file join $bd_dir hdl capture_bd_wrapper.v]]
add_files -fileset constrs_1 -norecurse [list [file join $root constraints board_base.xdc]]
add_files -fileset constrs_1 -norecurse [list [file join $root constraints radar_j37.xdc]]
set_property top z100_radar_top [get_filesets sources_1]
update_compile_order -fileset sources_1
write_hwdef -force -file [file join $root hardware radar_sync.hdf]
write_bd_tcl -force [file join $root scripts capture_bd_export.tcl]
puts {RADAR_SYNC_PROJECT_CREATED}
close_project
