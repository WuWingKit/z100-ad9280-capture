# Usage: vivado -mode batch -source scripts/capture_ila.tcl -tclargs arm|read
# Does not reprogram FPGA and does not touch PS memory or Flash.
set root [file normalize [file join [file dirname [info script]] ..]]
open_hw
connect_hw_server -url localhost:3121
open_hw_target
set device [get_hw_devices -filter {PART == xc7z035}]
current_hw_device $device
set_property PROBES.FILE [file join $root hardware radar_debug.ltx] $device
refresh_hw_device $device
set ila [get_hw_ilas -of_objects $device]
if {[llength $ila]!=1} {error "Expected exactly one debug ILA"}
if {[lindex $argv 0] eq "arm"} {
    set probe [get_hw_probes -of_objects $ila -filter {NAME =~ *spi_cs_n_1}]
    set_property TRIGGER_COMPARE_VALUE eq1'bF $probe
    set_property CONTROL.TRIGGER_POSITION 512 $ila
    run_hw_ila $ila
    puts {ILA_ARMED_CS_FALL}
} else {
    upload_hw_ila_data $ila
    set data [get_hw_ila_data -of_objects $ila]
    write_hw_ila_data -force -csv_file [file join $root evidence board_spi_ila.csv] $data
    write_hw_ila_data -force [file join $root evidence board_spi_ila.ila] $data
    puts {ILA_REAL_CAPTURE_SAVED}
}
close_hw
exit
