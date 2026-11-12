set root [file normalize [file join [file dirname [info script]] ..]]
open_hw
connect_hw_server -url localhost:3121
open_hw_target
set device [get_hw_devices -filter {PART == xc7z035}]
current_hw_device $device
set_property PROBES.FILE [file join $root hardware radar_debug.ltx] $device
refresh_hw_device $device
puts "DEBUG_ILAS [get_hw_ilas]"
puts "DEBUG_PROBES [get_hw_probes]"
help wait_on_hw_ila
help write_hw_ila_data
report_property [lindex [get_hw_probes -filter {NAME =~ *spi_cs_n_1}] 0]
close_hw
exit
