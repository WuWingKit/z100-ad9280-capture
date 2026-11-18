# MCU模块集成状态（2026-11-12）

本目录是唯一的通用乒乓源码，已直接加入mcu_frontend的Keil工程，不再是待接入模块。

板级绑定：mcu_frontend/05_application/firmware_app/source/src/app_sync_frontend.c。main调用Init/Process，GPIO IRQ转入GpioIsr；P1上升沿仅记录事件，P12 READY确认忙闲；采集、RF关闭、波束预写/LOAD和SPI发送回调均绑定原驱动。

Keil提供真实采集和RAM自检两个目标，均使用同一缓冲区所有权状态机和Version3帧头。原参考工程在根工作区仍然保留未修改。

详见根项目docs/代码逐段讲解.md、docs/上板接线与调试.md及docs/PROTOCOL.md。未进行真实硬件下载或Flash烧写。
