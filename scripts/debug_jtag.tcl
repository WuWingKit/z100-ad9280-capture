# ===== 用途与执行边界 =====
# 实际硬件操作入口：通过物理检查门槛后，经JTAG下载易失bit与DDR ELF；不写Flash。
# 在Vivado Tcl/SDK XSCT中执行，具体命令见README；普通打开文件不会执行脚本。

# Explicit debug-only download: volatile PL configuration + DDR application.
# Never calls program_flash, bootgen, or writes an SD/eMMC/QSPI boot image.
set root [file normalize [file join [file dirname [info script]] ..]]
if {![info exists ::env(Z100_PHYSICAL_CHECKED)] || $::env(Z100_PHYSICAL_CHECKED) ne "YES"} {
    error "Read docs/IMPLEMENTATION.md and confirm the physical board/power/pin checks first. Set Z100_PHYSICAL_CHECKED=YES only after those checks."
}
set bit [file join $root hardware z100_cordic.bit]
set elf [file join $root sdk_workspace adc_uart Debug adc_uart.elf]
foreach f [list $bit $elf] {if {![file exists $f]} {error "Required build output missing: $f"}}
# 以下开始实际JTAG会话；若只阅读代码，不要执行
connect
puts [targets]
if {![string match -nocase *xc7z035* [targets]]} {
    error "Expected XC7Z035 target not found; stop rather than program another part"
}
targets -set -filter {name =~ "*Cortex-A9*#0"}
rst -system
after 1000
source [file join $root sdk_workspace z100_hw ps7_init.tcl]
ps7_init
fpga -file $bit
ps7_post_config
targets -set -filter {name =~ "*Cortex-A9*#0"}
dow $elf
bpadd -addr &main
con
after 500
stop
puts [targets]
puts {DEBUG_READY: application halted at/near main. Inspect registers, then type con to start UART command loop.}

