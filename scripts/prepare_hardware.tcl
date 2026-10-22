set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root bd_start.xpr]
open_bd_design [file join $root bd_start.srcs sources_1 bd capture_bd capture_bd.bd]
set ps [get_bd_cells processing_system7_0]
source [file join $root board_reference apply_board_ps.tcl]

if {![llength [get_bd_cells -quiet adc_board_control_0]]} {
    add_files -norecurse [list [file join $root rtl adc_board_control.v]]
    update_compile_order -fileset sources_1
    create_bd_cell -type module -reference adc_board_control adc_board_control_0
    create_bd_cell -type ip -vlnv xilinx.com:ip:axi_gpio:2.0 axi_gpio_0
    set_property -dict [list CONFIG.C_IS_DUAL 1 CONFIG.C_ALL_OUTPUTS 1 \
        CONFIG.C_GPIO_WIDTH 32 CONFIG.C_ALL_INPUTS_2 1 CONFIG.C_GPIO2_WIDTH 32] [get_bd_cells axi_gpio_0]
    # Reuse PS GP0 interconnect: port 0 remains DMA, port 1 controls acquisition.
    set_property CONFIG.NUM_MI 2 [get_bd_cells ps7_0_axi_periph]
    connect_bd_intf_net [get_bd_intf_pins ps7_0_axi_periph/M01_AXI] [get_bd_intf_pins axi_gpio_0/S_AXI]
    foreach pin {axi_gpio_0/s_axi_aclk ps7_0_axi_periph/M01_ACLK adc_board_control_0/axis_clk} {
        connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins $pin]
    }
    foreach pin {axi_gpio_0/s_axi_aresetn ps7_0_axi_periph/M01_ARESETN adc_board_control_0/rst_n} {
        connect_bd_net [get_bd_pins proc_sys_reset_0/peripheral_aresetn] [get_bd_pins $pin]
    }
    connect_bd_net [get_bd_pins axi_gpio_0/gpio_io_o] [get_bd_pins adc_board_control_0/command]
    connect_bd_net [get_bd_pins adc_board_control_0/status] [get_bd_pins axi_gpio_0/gpio2_io_i]
    connect_bd_net [get_bd_ports adc_clk_0] [get_bd_pins adc_board_control_0/adc_clk]

    # Replace simulation-only external controls with PS GPIO and ADC-domain CDC.
    foreach port {start_pulse_0 adc_valid_0 rst_n_0} {
        if {[llength [get_bd_ports -quiet $port]]} {delete_bd_objs [get_bd_ports $port]}
    }
    set capture_reset [get_bd_pins adc_capture_pack_0/rst_n]
    disconnect_bd_net [get_bd_nets -of_objects $capture_reset] $capture_reset
    connect_bd_net [get_bd_pins adc_board_control_0/capture_rst_n] $capture_reset
    connect_bd_net [get_bd_pins xlconstant_1/dout] [get_bd_pins util_vector_logic_1/Op1]
    connect_bd_net [get_bd_pins adc_board_control_0/start_pulse] [get_bd_pins adc_capture_pack_0/start_pulse]
    connect_bd_net [get_bd_pins xlconstant_1/dout] [get_bd_pins adc_capture_pack_0/adc_valid]
    set data_pin [get_bd_pins adc_capture_pack_0/adc_data]
    disconnect_bd_net [get_bd_nets -of_objects $data_pin] $data_pin
    connect_bd_net [get_bd_ports adc_data_0] [get_bd_pins adc_board_control_0/adc_raw]
    connect_bd_net [get_bd_pins adc_board_control_0/adc_sample] $data_pin
    foreach {src dest} {busy capture_busy done capture_done overflow capture_overflow sample_count sample_count} {
        connect_bd_net [get_bd_pins adc_capture_pack_0/$src] [get_bd_pins adc_board_control_0/$dest]
    }
}
assign_bd_address
set gpioseg [get_bd_addr_segs -quiet processing_system7_0/Data/SEG_axi_gpio_0_Reg]
if {[llength $gpioseg]} {set_property offset 0x41200000 $gpioseg}
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -no_script -sync -force
make_wrapper -files [get_files -all */capture_bd.bd] -top
source [file join $root scripts generate_top.tcl]
set_property top z100_adc_top [get_filesets sources_1]
update_compile_order -fileset sources_1
# The previous simulation TB remains in the backup, but targets the old BD ports.
set_property USED_IN_SIMULATION false [get_files -quiet */tb_capture_bd.v]
puts {Z100_HARDWARE_PREPARED}
close_project
