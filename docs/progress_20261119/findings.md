# 报告图像素材

## 01_real_iq_first_chirp.png

真实采集第0帧的第一条保留chirp。按16位补码解释I/Q；横轴为样点，纵轴为码值，未经电压或距离标定。

## 02_ten_frame_statistics.png

10帧I/Q均值与标准差趋势，来自每帧11,776点统计；不据此推断目标运动、信噪比或射频质量。

## 03_board_test_summary.png

本轮四线SPI真实采集、DDR保存及UART导出的验证指标；最后一帧另经JTAG逐字节核对。

## 04_ps_crc_read_source.png

PS真实源码的带行号节录，展示请求、响应与CRC16校验；不是IDE屏幕截图。

## 05_mcu_lease_source.png

MCU真实源码的带行号节录，展示缓存租约、触发拒绝、块读取与ACK释放；不是IDE屏幕截图。

## 06_log_evidence_excerpt.png

从保存日志及JSON生成的证据节录图；完整原文和载荷保存在证据包，不是串口软件屏幕截图。

## 07_historical_radar_board_photo.jpg

用户此前提供的区控板接口照片，原样复制；仅说明实物接口，不能证明本次接线或测试场景。

图像按真实源码、日志和BIN生成，数据来源可追溯。未生成FFT、距离、角度或实验场景的虚假图像。
