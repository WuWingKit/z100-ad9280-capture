# SP328B / W25X40CL 外置Flash烧录说明

日期：2026-11-18。此目录只生成文件，未执行烧录或擦除。

## 文件来源与验证范围

使用现有已编译的hardware/radar_sync_selftest.bin及radar_sync_mcu.bin，调用原工程的make_andar_flash_image.ps1，增加Boot V3的16字节头。本次没有修改MCU业务代码，也没有重新验证真实MCU上电启动。

Python独立复核了原参考镜像和新镜像的头部、长度、偏移、CRC32、应用向量栈地址及Thumb复位入口；新镜像的载荷与输入应用BIN逐字节一致。完整记录见verification.json，文件校验见SHA256SUMS.txt。

## 选择文件

首次测试选择 radar_sync_selftest_W25X40CL_512KiB.bin。它不启动真实雷达采集/RF，仍使用真实MCU GPIO、SPI和乒乓缓存。链路通过后再换 radar_sync_mcu_W25X40CL_512KiB.bin。

512KiB文件包含：启动头+应用+0xFF填充，恰为524288字节；适用于已确认的W25X40CL（4Mbit）整片写入。较短的*_andar_flash.bin内容等价但不包含末尾FF填充，不能与原始hardware/*.bin混淆。

## 操作步骤

1. 整片擦除前，用烧录器读取当前Flash，保存完整524288字节备份，并确认能正常读回；保留原SP32工程及原始可用镜像。整片写入不会保留Flash末尾原有内容。
2. 芯片型号保持Winbond W25X40CL / 4M-bits；不要按Keil自带的8Mbit算法名称修改烧录器芯片型号。
3. 加载本目录的 radar_sync_selftest_W25X40CL_512KiB.bin。加载至缓冲区0，写入外置Flash物理地址0。MCU运行地址0x8000已编码在程序中，不是烧录器应填写的Flash偏移。
4. 烧录区域选A:FLASH（4M-bits），不选Status Register；操作为自动擦除→编程→校验。12MHz和X1是用户已有配置，本次未做适配器实测；若仍出现E041“适配器模块IO异常”，先检查当前适配器、接触和供电，不能只凭旧的成功日志继续判断本次成功。
5. 完成后建议再次读取整片，与加载的512KiB文件逐字节比较，保存日志。烧录器校验成功证明存储一致，不代表雷达固件已经启动或SPI链路通过。
6. 将Flash装回原板，保持原来可从Flash启动的启动配置。断电接线、分别供电、共地；按docs/上板接线与调试.md核对SPI/SYNC/READY及移除UART0跳线冲突。不要将芯片烧录电压直接当作Z100信号电压。
7. Z100必须使用外部前端版hardware/radar_sync.bit和radar_uart.elf，不能使用radar_spi_debug_20261112的内部模拟版。串口应显示Z100 RADAR EXTERNAL SPI + SYNC。
8. MCU启动后等待SYNC，不会自行发送完整数据帧；s确认READY后，p首轮预热，c收上一帧，v检查自检样点。通过后重置双方，再烧真实采集版，用p/c/d；真实回波不应使用v的固定模式作为通过标准。

## 范围限制

这是按工程现有Boot V3规范封装的待上板验证镜像。没有执行SP328烧写、断电启动、真实MCU SPI或射频测试。本目录不改变此前“FPGA内部模拟板测通过”和“真实MCU尚待联调”的边界。
