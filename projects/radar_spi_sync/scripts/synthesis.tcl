set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_sync.xpr]
open_bd_design [get_files -all */capture_bd.bd]
update_module_reference [get_ips -all *radar_frame_capture*]
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
write_hwdef -force -file [file join $root hardware radar_sync.hdf]
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Synthesis failed"}
open_run synth_1
report_utilization -file [file join $root hardware synthesis_utilization.rpt]
report_timing_summary -file [file join $root hardware synthesis_timing.rpt]
puts {RADAR_SYNC_SYNTHESIS_PASS_NO_BITSTREAM}
close_project


