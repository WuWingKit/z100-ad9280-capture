set topfile [file join $root rtl z100_adc_top.v]
if {![llength [get_files -quiet *z100_adc_top.v]]} {add_files -norecurse [list $topfile]}
