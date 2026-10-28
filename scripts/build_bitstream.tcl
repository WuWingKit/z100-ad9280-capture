set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root bd_start.xpr]
reset_run synth_1
launch_runs synth_1 -jobs 4
wait_on_run synth_1
if {[get_property PROGRESS [get_runs synth_1]] ne "100%"} {error "Synthesis failed"}
launch_runs impl_1 -to_step route_design -jobs 4
wait_on_run impl_1
if {[get_property PROGRESS [get_runs impl_1]] ne "100%"} {error "Implementation failed"}
open_run impl_1
report_timing_summary -report_unconstrained -file [file join $root hardware timing_routed.rpt]
report_drc -file [file join $root hardware drc_routed.rpt]
report_cdc -details -file [file join $root hardware cdc_routed.rpt]
report_io -file [file join $root hardware io_routed.rpt]
foreach typ {max min} {
    set paths [get_timing_paths -delay_type $typ -max_paths 1]
    if {![llength $paths]} {error "No timing paths available for $typ check"}
    set slack [get_property SLACK [lindex $paths 0]]
    puts "Z100_TIMING_$typ slack=$slack"
    if {$slack < 0} {error "Timing failed; bitstream generation stopped"}
}
close_design
launch_runs impl_1 -to_step write_bitstream -jobs 4
wait_on_run impl_1
set bit [file join $root bd_start.runs impl_1 z100_adc_top.bit]
if {![file exists $bit]} {error "Bitstream generation failed"}
file copy -force $bit [file join $root hardware z100_cordic.bit]
write_sysdef -hwdef [file join $root hardware system.hdf] -bitfile $bit -file [file join $root hardware z100_cordic.hdf] -force
puts {Z100_BITSTREAM_PASS}
close_project

