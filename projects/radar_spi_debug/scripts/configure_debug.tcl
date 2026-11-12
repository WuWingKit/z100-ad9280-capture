set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_debug.xpr]
open_bd_design [get_files -all */capture_bd.bd]
source [file join $root scripts add_debug_ila.tcl]
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -sync -force -quiet
write_hwdef -force -file [file join $root hardware radar_debug.hdf]
write_bd_tcl -force [file join $root scripts capture_bd_export.tcl]
puts {DEBUG_ILA_CONFIGURED}
close_project
