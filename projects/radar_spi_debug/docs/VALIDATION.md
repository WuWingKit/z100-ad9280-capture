# 验证层次与结果

2026-11-12，所有下列项目均已执行；详细板测另见 [BOARD_TEST_REPORT.md](BOARD_TEST_REPORT.md)。

|层次|实际覆盖|结果文件|
|---|---|---|
|小帧源/接收仿真|可综合A/B源→真实SPI解码→帧检查→模型FIFO→真实AXIS；21连续帧跨波束回绕，故障和传输中复位恢复，共22个正常帧；3,090项检查|`evidence/unit_result.txt`|
|完整帧串行仿真|同样的源/解码/帧BRAM/AXIS，三帧各47,136字节，共141,408字节；反压下数据稳定，上一帧头部/载荷关联；122,136项检查|`evidence/full_wire_result.txt`|
|真实BD DDR仿真|源→串行SPI→帧检查→真实FIFO→真实DMA→HP0→PS VIP DDR模型；3帧各160字节，共480字节；逐字节和前后保护区检查，1,211项检查；无解码信号强制注入|`evidence/radar_ddr_result.txt`|
|布线与生成|100MHz时序通过，WNS +4.041ns、WHS +0.044ns；包含ILA，生成bit/LTX/HDF|`hardware/implemented_*.rpt`|
|PS软件|SDK2018.3编译成功，最终硬件HDF同步至debug_hw|`evidence/software_build.txt`|
|真实Z100|JTAG下载、COM11、连续22完整帧、三类故障及恢复、PC独立整帧核对|`evidence/board_20261112_170218/`|
|板内ILA|4096点，实际解码27个SPI16位字，包括完整头部；匹配期望|`evidence/board_ila_result.json`|

DDR仿真的160字节通过testbench层次参数覆盖，仅用于降低PS VIP仿真耗时；不修改生产BD。生产bit和真实板测均为47,136字节帧。仿真未运行真实ARM ELF；真实ELF已通过板上测试验证。

原外部雷达工程未修改；本副本的 `spi_slave_rx16.v`、`radar_frame_capture.v`、`radar_fifo_to_axis.v` 与其保持逐字节一致。新增内容为可综合源、ILA、独立顶层、PS自检/故障命令与测试脚本。基准哈希见 `evidence/external_baseline.json`，最终交付检查见 `evidence/delivery_manifest.json`。
