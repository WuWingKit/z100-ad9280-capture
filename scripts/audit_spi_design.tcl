set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root spi_capture.xpr]
open_bd_design [get_files -all */capture_bd.bd]
if {[llength [get_bd_cells -quiet cordic*]]} {error "Unused waveform IP remains"}
foreach p {adc_data_0 ad_otr_0 dac_data adc_clk_0} {
    if {[llength [get_bd_ports -quiet $p]]} {error "Legacy parallel port remains: $p"}
}
set f [open [file join $root evidence SPI_ACTIVE_SOURCES.txt] w]
foreach path [get_files -compile_order sources -used_in synthesis] {puts $f $path}
close $f
validate_bd_design
save_bd_design
generate_target all [get_files -all */capture_bd.bd]
write_hwdef -force -file [file join $root hardware system.hdf]
puts {SPI_ONLY_ACTIVE_DESIGN_PASS}
close_project
