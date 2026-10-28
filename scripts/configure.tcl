set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root bd_start.xpr]
foreach f [get_files -quiet *adc_board_control.v] {remove_files $f}
foreach f [get_files -quiet *adc_capture_pack.v] {remove_files $f}
foreach f [get_files -quiet *fifo_to_axis.v] {remove_files $f}
add_files -norecurse [list [file join $root rtl measurement_core.v] [file join $root rtl fifo_to_axis.v]]
update_compile_order -fileset sources_1
open_bd_design [get_files -all */capture_bd.bd]
delete_bd_objs [get_bd_cells adc_board_control_0] [get_bd_cells adc_capture_pack_0]
foreach p {busy_0 done_0 overflow_0 sample_count_0 stream_done_0} {delete_bd_objs [get_bd_ports $p]}
update_module_reference [get_ips capture_bd_fifo_to_axis_0_0]
create_bd_cell -type module -reference measurement_core measurement_core_0
create_bd_cell -type ip -vlnv xilinx.com:ip:cordic:6.0 cordic_sincos_0
set_property -dict [list CONFIG.Functional_Selection Sin_and_Cos CONFIG.Phase_Format Scaled_Radians \
 CONFIG.Input_Width 16 CONFIG.Output_Width 16 CONFIG.ARESETN true CONFIG.Coarse_Rotation true \
 CONFIG.Architectural_Configuration Parallel CONFIG.Pipelining_Mode Maximum \
 CONFIG.flow_control NonBlocking CONFIG.Round_Mode Nearest_Even] [get_bd_cells cordic_sincos_0]
create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_config
set_property -dict [list CONFIG.C_IS_DUAL 1 CONFIG.C_ALL_OUTPUTS 1 CONFIG.C_ALL_OUTPUTS_2 1 \
 CONFIG.C_GPIO_WIDTH 32 CONFIG.C_GPIO2_WIDTH 32] [get_bd_cells axi_gpio_config]
set_property CONFIG.NUM_MI 3 [get_bd_cells ps7_0_axi_periph]
connect_bd_intf_net [get_bd_intf_pins ps7_0_axi_periph/M02_AXI] [get_bd_intf_pins axi_gpio_config/S_AXI]
foreach pin {measurement_core_0/axis_clk axi_gpio_config/s_axi_aclk ps7_0_axi_periph/M02_ACLK} {
 connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins $pin]
}
foreach pin {measurement_core_0/rst_n axi_gpio_config/s_axi_aresetn ps7_0_axi_periph/M02_ARESETN} {
 connect_bd_net [get_bd_pins proc_sys_reset_0/peripheral_aresetn] [get_bd_pins $pin]
}
foreach pin {measurement_core_0/adc_clk cordic_sincos_0/aclk} {
 connect_bd_net [get_bd_ports adc_clk_0] [get_bd_pins $pin]
}
foreach {src dst} {
 axi_gpio_0/gpio_io_o measurement_core_0/command
 axi_gpio_config/gpio_io_o measurement_core_0/phase_word
 axi_gpio_config/gpio2_io_o measurement_core_0/config_word
 measurement_core_0/status axi_gpio_0/gpio2_io_i
 measurement_core_0/fifo_din fifo_generator_0/din
 measurement_core_0/fifo_wr_en fifo_generator_0/wr_en
 fifo_generator_0/full measurement_core_0/fifo_full
 fifo_to_axis_0/stream_done measurement_core_0/stream_done
 measurement_core_0/frame_samples_axis fifo_to_axis_0/frame_samples
 measurement_core_0/phase_data cordic_sincos_0/s_axis_phase_tdata
 measurement_core_0/phase_valid cordic_sincos_0/s_axis_phase_tvalid
 measurement_core_0/cordic_reset_n cordic_sincos_0/aresetn
 cordic_sincos_0/m_axis_dout_tdata measurement_core_0/cordic_data
 cordic_sincos_0/m_axis_dout_tvalid measurement_core_0/cordic_valid
} {connect_bd_net [get_bd_pins $src] [get_bd_pins $dst]}
connect_bd_net [get_bd_ports adc_data_0] [get_bd_pins measurement_core_0/adc_raw]
connect_bd_net [get_bd_ports ad_otr_0] [get_bd_pins measurement_core_0/adc_otr]
foreach {target src} {fifo_generator_0/rst measurement_core_0/fifo_reset fifo_to_axis_0/rst_n measurement_core_0/axis_reset_n} {
 set p [get_bd_pins $target]
 disconnect_bd_net [get_bd_nets -of_objects $p] $p
 connect_bd_net [get_bd_pins $src] $p
}
create_bd_port -dir O -from 7 -to 0 dac_data
connect_bd_net [get_bd_pins measurement_core_0/dac_data] [get_bd_ports dac_data]
assign_bd_address
set_property offset 0x41210000 [get_bd_addr_segs processing_system7_0/Data/SEG_axi_gpio_config_Reg]
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
add_files -norecurse [make_wrapper -files [get_files -all */capture_bd.bd] -top]
set_property top z100_adc_top [get_filesets sources_1]
update_compile_order -fileset sources_1
write_hwdef -force -file [file join $root hardware system.hdf]
puts {CORDIC_LOOP_CONFIGURED}
close_project
