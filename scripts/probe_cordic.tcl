set root [file normalize [file join [file dirname [info script]] ..]]
create_project probe [file join $root _probe] -part xc7z035ffg900-2
create_ip -name cordic -vendor xilinx.com -library ip -version 6.0 -module_name cordic_sincos
set f [open [file join $root evidence cordic_properties.txt] w]
foreach p [list_property [get_ips cordic_sincos]] {
 if {[string match CONFIG.* $p]} {puts $f "$p=[get_property $p [get_ips cordic_sincos]]"}
}
close $f
close_project
