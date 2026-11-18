# MCU SYNC上升沿与A/B乒乓模块接入

本目录是平台无关C模块，尚未绑定具体GPIO/Keil工程。原始老师工程未修改。

1. 在老师f1_0的开发副本中加入`radar_sync_pipeline.c/h`，通过`RadarSyncPorts`连接已有硬件接口。
2. `capture_start`调用`adt_scan_acquire_start(addr)`；`capture_finish`调用`adt_scan_acquire_wait()`，必须完成DMA并关闭RF。
3. `prepare_beam`、`load_beam`连接已有波束函数。初始化固定索引9=0°；`scan=0`首轮先固定波束，`scan=1`才轮转。
4. `send_frame`按顺序调用`AppSpiStream_Prepare(addr)`、`Trigger()`、`Wait()`。必须判断每一步返回值，并在失败时Abort/关闭RF，不要提前释放缓冲区。
5. `write_word`实现为`*(volatile uint16_t *)addr=value`。仅在ADC DMA完成后填头。
6. `enter_critical/leave_critical`使用CMSIS的PRIMASK保存、禁中断和恢复，防止SYNC ISR与主循环事件交接竞态。
7. 初始化ADC/RF、SPI外设及发送模块、Pipeline成功后，才启用实际SYNC GPIO的上升沿中断。ISR清硬件pending后只调用`RadarSync_OnRisingEdge()`；主循环轮询`RadarSync_Process()`。RF与SPI阻塞操作不得放入ISR。
8. SYNC GPIO尚未提供，禁止猜测引脚。本目录不生成前端可烧写固件；GPIO复用、沿触发和中断向量仍需按硬件图绑定。

## 必须处理的参考工程差异

- `adt6101P_init.c`现在主动关闭SPI0；需恢复Mode0、16bit、主机及目标12.5MHz配置。HAL说明“CPHA_1Edge”是第一跳沿采样；不要误配置成Mode1。
- 当前P0/P1是同步输出，P0要改为SPI低有效CS独占。`main.c`的`app_hw_capture_wait()`有同时拉低P0/P1的语句，接入时必须移除对P0的写入，否则首次只采集时也可能产生假SPI帧边界。
- 改用SYNC调度后，不再自动循环调用旧`AppFrontend_RunCycle()`，也不在启动时额外采集隐藏预热帧。第一枚FPGA SYNC明确只填A，不发送；第二枚采B发A。
- 50ms周期由FPGA/PS请求节奏控制。MCU忙时拒绝新增SYNC并累计`rejected_sync`，不能覆盖上一事件。
- 本版使用32B版本3帧头，Word14是16位帧号，Word15为头部XOR；不含载荷CRC。第一chirp不发送，后23chirp为有效载荷。
- 初次“prime等待”是FPGA可配置的定时等待，并非真实MCU-ready反馈。实际等待时间须大于前端首次采集耗时；若需要确定握手，应增加确认GPIO或准备状态协议。
- 软复位FPGA不会自动清空MCU的A/B状态。重新预填充必须同时重置两端协议状态，或另行实现显式重同步命令。

必须先确认外部GPIO电平、SPI/SYNC接线和RF时序，再编译前端固件和上板。纯C逻辑检查、MCU行为模型仿真不替代真实MCU固件验证。
