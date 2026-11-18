set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_sync.xpr]
if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Run synthesis first"}
reset_run impl_1
launch_runs impl_1 -to_step route_design -jobs 4
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] ne "100%"} {error "Implementation failed"}
open_run impl_1
report_timing_summary -file [file join $root hardware implemented_timing.rpt]
report_utilization -file [file join $root hardware implemented_utilization.rpt]
report_drc -file [file join $root hardware implemented_drc.rpt]
report_cdc -file [file join $root hardware implemented_cdc.rpt]
set setup_bad [get_timing_paths -quiet -delay_type max -slack_lesser_than 0 -max_paths 1]
set hold_bad [get_timing_paths -quiet -delay_type min -slack_lesser_than 0 -max_paths 1]
if {[llength $setup_bad] || [llength $hold_bad]} {error "Timing violations: no bitstream written"}
if {[llength [get_drc_violations -quiet -filter {SEVERITY == Error}]]} {error "DRC errors: no bitstream written"}
write_bitstream -force [file join $root hardware radar_sync.bit]
write_hwdef -force -file [file join $root hardware radar_sync.hdf]
puts {RADAR_IMPLEMENTATION_AND_BITSTREAM_PASS}
close_project
