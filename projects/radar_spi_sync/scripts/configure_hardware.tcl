set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_sync.xpr]
open_bd_design [get_files -all */capture_bd.bd]
update_module_reference [get_ips -all *radar_frame_capture*]
if {![llength [get_bd_ports -quiet mcu_ready]]} {create_bd_port -dir I mcu_ready}
if {![llength [get_bd_nets -quiet -of_objects [get_bd_pins radar_frame_capture_0/mcu_ready]]]} {
    connect_bd_net [get_bd_ports mcu_ready] [get_bd_pins radar_frame_capture_0/mcu_ready]
}
set xdc [file join $root constraints radar_j37.xdc]
if {![llength [get_files -quiet -all [list $xdc]]]} {add_files -fileset constrs_1 -norecurse [list $xdc]}
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -sync -force -quiet
make_wrapper -files [get_files -all */capture_bd.bd] -top
update_compile_order -fileset sources_1
write_hwdef -force -file [file join $root hardware radar_sync.hdf]
write_bd_tcl -force [file join $root scripts capture_bd_export.tcl]
puts {RADAR_J37_READY_CONFIGURED}
close_project
