set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root bd_start.xpr]
open_bd_design [file join $root bd_start.srcs sources_1 bd capture_bd capture_bd.bd]
write_hwdef -force -file [file join $root hardware system.hdf]
puts {Z100_HDF_EXPORTED}
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Synthesis failed"}
open_run synth_1
report_utilization -file [file join $root hardware utilization_synth.rpt]
report_drc -file [file join $root hardware drc_synth.rpt]
puts {Z100_SYNTHESIS_PASS}
close_project
