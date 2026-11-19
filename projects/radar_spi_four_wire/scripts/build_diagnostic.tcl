set root [file normalize [file join [file dirname [info script]] ..]]
file mkdir [file join $root hardware]
file mkdir [file join $root evidence diagnostic_20261119]
open_project [file join $root radar_sync.xpr]
set_property ip_output_repo [file join $root ip_cache] [current_project]
open_bd_design [get_files -all */capture_bd.bd]
update_module_reference [get_ips -all *radar_frame_capture*]
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Synthesis failed"}
launch_runs impl_1 -to_step route_design -jobs 4
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] ne "100%"} {error "Implementation failed"}
open_run impl_1
report_timing_summary -file [file join $root evidence diagnostic_20261119 timing.rpt]
report_drc -file [file join $root evidence diagnostic_20261119 drc.rpt]
if {[llength [get_timing_paths -quiet -delay_type max -slack_lesser_than 0 -max_paths 1]] || [llength [get_timing_paths -quiet -delay_type min -slack_lesser_than 0 -max_paths 1]]} {error "Timing failed"}
if {[llength [get_drc_violations -quiet -filter {SEVERITY == Error}]]} {error "DRC failed"}
write_bitstream -force [file join $root hardware spi_only_master.bit]
puts DIAGNOSTIC_BIT_PASS
close_project
