# MCU SYNC前端副本

当前不是原f1_0自动采集版本。已接入FPGA SYNC上升沿触发、P12 READY、SPI0整帧输出以及A/B乒乓。

打开05_application/firmware_app/keil_project/firmware_app.uvprojx，选择radar_sync_selftest或radar_sync_mcu。

通用逻辑直接引用上一级mcu_sync/radar_sync_pipeline.c。真实硬件绑定在source/src/app_sync_frontend.c；厂商库与原版权保留。旧docs/F1_0_*.md是上游参考说明，不能作为新SYNC协议依据；以根项目docs/PROTOCOL.md及docs/上板接线与调试.md为准。

MCU自检版不调用采集/RF开启回调；真实版等待SYNC后才开始采集。首次只填A，后续采另一块并发送上一块。P0为CS、P1为输入SYNC、P3为SCLK、P6为MOSI、P12为READY。移除底板UART0跳线后接线；无自动Flash烧写步骤。
