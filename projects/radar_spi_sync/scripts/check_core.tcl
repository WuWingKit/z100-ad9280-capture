set root [file normalize [file join [file dirname [info script]] ..]]
read_verilog [list [file join $root rtl spi_slave_rx16.v] [file join $root rtl radar_frame_capture.v]]
synth_design -top radar_frame_capture -part xc7z035ffg900-2 -mode out_of_context
report_utilization -file [file join $root evidence core_utilization.rpt]
if {[llength [get_cells -hier -filter {REF_NAME =~ RAMB*}]]==0} {error "Frame RAM was not mapped to block RAM"}
puts {RADAR_CORE_BRAM_INFERENCE_PASS}
