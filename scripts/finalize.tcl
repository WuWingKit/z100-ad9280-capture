set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root bd_start.xpr]
open_bd_design [get_files -all */capture_bd.bd]
foreach n [get_bd_nets] {
 if {[llength [get_bd_pins -quiet -of_objects $n]]==0 && [llength [get_bd_ports -quiet -of_objects $n]]==0} {delete_bd_objs $n}
}
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -no_script -sync -force
write_hwdef -force -file [file join $root hardware system.hdf]
close_project
