# Read-only verification of the current ELF data[] buffer; resume CPU after read.
set root [file normalize [file join [file dirname [info script]] ..]]
connect -url tcp:127.0.0.1:3121
if {[llength [targets -target-properties -filter {name =~ "xc7z035*"}]] != 1} {error "Expected one XC7Z035"}
targets -set -filter {name =~ "ARM*#0"}
stop
set code [catch {mrd -bin -file [file join $root evidence spi_only_real_20261119_154059 last_frame_ddr.bin] 0x00108100 11784} message]
con
disconnect
if {$code} {error $message}
puts LAST_FRAME_DDR_READ_DONE_CPU_RESUMED
exit
