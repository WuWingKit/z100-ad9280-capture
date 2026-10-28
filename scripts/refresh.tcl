set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root bd_start.xpr]
open_bd_design [get_files -all */capture_bd.bd]
update_module_reference [get_ips capture_bd_measurement_core_0_0]
if {[llength [get_bd_cells -quiet util_vector_logic_0]]} {delete_bd_objs [get_bd_cells util_vector_logic_0]}
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -no_script -sync -force
write_hwdef -force -file [file join $root hardware system.hdf]
close_project

