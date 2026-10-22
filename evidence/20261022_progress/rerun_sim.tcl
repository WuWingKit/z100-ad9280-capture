set root E:/FPGAWork/z100_adc_hw_20261022
set result [file join $root bd_start.sim sim_1 behav xsim z100_sim_result.txt]
if {[file exists $result]} {file delete -force $result}
source [file join $root scripts run_hw_sim.tcl]
set f [open $result r]
set result_text [read $f]
close $f
if {![string match {PASS frames=2 samples=200 DDR_bytes=200 checks=2676*} [string trim $result_text]]} {error "Unexpected simulation result"}
file copy -force $result [file join $root evidence 20261022_progress simulation_rerun.txt]
puts {PROGRESS_SIMULATION_RERUN_PASS}
