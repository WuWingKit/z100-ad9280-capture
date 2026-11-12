set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_debug.xpr]
open_bd_design [get_files -all */capture_bd.bd]
update_module_reference [get_ips -all *frontend_sim*]
validate_bd_design
save_bd_design
# Regenerate SmartConnect children after copying/updating the BD. Its cached
# parent can otherwise retain references to missing generated child XDC files.
reset_target all [get_files -all */capture_bd.bd]
generate_target all [get_files -all */capture_bd.bd]
export_ip_user_files -of_objects [get_files -all */capture_bd.bd] -sync -force -quiet
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Synthesis failed"}
launch_runs impl_1 -to_step route_design -jobs 4
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] ne "100%"} {error "Implementation failed"}
open_run impl_1
report_timing_summary -file [file join $root hardware implemented_timing.rpt]
report_utilization -file [file join $root hardware implemented_utilization.rpt]
report_drc -file [file join $root hardware implemented_drc.rpt]
report_cdc -file [file join $root hardware implemented_cdc.rpt]
if {[llength [get_timing_paths -quiet -delay_type max -slack_lesser_than 0 -max_paths 1]] || [llength [get_timing_paths -quiet -delay_type min -slack_lesser_than 0 -max_paths 1]]} {error "Timing failed; no bitstream"}
if {[llength [get_drc_violations -quiet -filter {SEVERITY == Error}]]} {error "DRC errors; no bitstream"}
write_debug_probes -force [file join $root hardware radar_debug.ltx]
write_bitstream -force [file join $root hardware radar_debug.bit]
write_hwdef -force -file [file join $root hardware radar_debug.hdf]
puts {FPGA_ONLY_DEBUG_HARDWARE_PASS}
close_project
