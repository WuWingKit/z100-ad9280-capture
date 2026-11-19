set root [file normalize [file join [file dirname [info script]] ..]]
open_project [file join $root radar_sync.xpr]
open_run impl_1
write_hwdef -force -file [file join $root hardware spi_only_hw.hdf]
puts SPI_ONLY_HWDEF_EXPORTED
close_project
