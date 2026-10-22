set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root bd_start.xpr]
open_bd_design [file join $root bd_start.srcs sources_1 bd capture_bd capture_bd.bd]
update_compile_order -fileset sources_1
update_module_reference [get_ips capture_bd_adc_board_control_0_0]
if {![llength [get_bd_ports -quiet board_reset_n]]} {
    set pin [get_bd_pins util_vector_logic_1/Op1]
    disconnect_bd_net [get_bd_nets -of_objects $pin] $pin
    create_bd_port -dir I -type rst board_reset_n
    set_property CONFIG.POLARITY ACTIVE_LOW [get_bd_ports board_reset_n]
    connect_bd_net [get_bd_ports board_reset_n] $pin
}
if {![llength [get_bd_ports -quiet ad_otr_0]]} {
    create_bd_port -dir I ad_otr_0
    connect_bd_net [get_bd_ports ad_otr_0] [get_bd_pins adc_board_control_0/adc_otr]
}
set_property CONFIG.FREQ_HZ 10000000 [get_bd_ports adc_clk_0]
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -no_script -sync -force

if {![llength [get_ips -quiet adc_clk_wiz]]} {
    create_ip -name clk_wiz -vendor xilinx.com -library ip -version 6.0 -module_name adc_clk_wiz
    set_property -dict [list CONFIG.PRIM_SOURCE {Differential_clock_capable_pin} \
        CONFIG.PRIM_IN_FREQ 100.000 CONFIG.NUM_OUT_CLKS 2 \
        CONFIG.CLKOUT1_REQUESTED_OUT_FREQ 10.000 CONFIG.CLKOUT2_USED true \
        CONFIG.CLKOUT2_REQUESTED_OUT_FREQ 10.000 CONFIG.CLKOUT2_REQUESTED_PHASE 180.000 \
        CONFIG.USE_RESET false CONFIG.USE_LOCKED true] [get_ips adc_clk_wiz]
}
generate_target all [get_ips adc_clk_wiz]
if {![llength [get_files -quiet *z100_adc.xdc]]} {
    add_files -fileset constrs_1 -norecurse [list [file join $root constraints z100_adc.xdc]]
}
set_property top z100_adc_top [get_filesets sources_1]
update_compile_order -fileset sources_1
write_hwdef -force -file [file join $root hardware system.hdf]
close_project
puts {PHYSICAL_ADC_CONFIGURED}
