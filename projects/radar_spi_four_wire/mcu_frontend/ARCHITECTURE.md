# f1_0 firmware architecture

`f1_0` 的目标是让 MCU 只完成“配置 + 波束控制 + ADC 采集 + SPI 原始数据转发”。

## 分层

| Layer | f1_0 职责 |
|---|---|
| `00_vendor_hal` | ADT6101P CMSIS/寄存器/HAL |
| `01_mcu_driver` | ADC、DMA、GPIO、SPI、UART、时钟、低功耗等板级驱动 |
| `02_mcu_library` | 当前为空 |
| `03_peripheral_driver` | ADT2011 驱动 |
| `04_internal_library` | 通用字节流协议解析器 |
| `05_application` | 固定 19 波束调度、带宽命令、ADC/SPI ping-pong、ISR 组合 |
| `06_third_party` | 仅保留与固件烧录相关资产；雷达算法库已移除 |

依赖方向保持为高层调用低层。应用不再依赖相位、绝对距离、FFT 后处理、槽位或结果 FIFO。

## 数据通路

```text
UART0 RX(P1) ──> 0x29 带宽命令 ──> PLL/FMCW 重配置
        └──────> 0x2A 帧周期命令 ──> 调度周期更新

固定19波束 ──> ADT2011 PSR/LOAD ──> FMCW
                                      │
                                      v
                                  ADC0 FIFO
                                      │ DMA1
                         ┌────────────┴────────────┐
                         │                         │
                       RAM A                     RAM B
                         │                         │
                         └──── ping-pong ──────────┘
                                      │ CPU writes SPI0 TX FIFO
                                      v
                                  SPI0 TX
                         CLK=P3 MOSI=P6 /CS=P0
```

每轮开始时，ADC DMA 写当前采集块，上一帧 SPI 输出源先预配置但暂不发送。ADC DMA 完成后先关闭 ADT2011 RF 并 LOAD 下一波束，再发送上一帧 SPI 数据，因此 CPU 顺序写 SPI0 TX FIFO 时 RF 已经关闭。A/B 两块 RAM 仍保持一块采集、一块待发送的 ping-pong 关系。

## 波束流水

启动阶段生成第一帧 `-45°`。稳态中，当前采集波束为 `b` 时，ADC 运行 100 us 后按 TX→RX 顺序预写 `b+1` PSR；随后主循环轮询 ADC DMA 完成，确认采集结束后关闭 ADT2011 RF，再按 TX→RX 顺序 LOAD `b+1`；最后发送上一帧 `b-1`。ADC DMA DONE 中断不参与业务处理；SPI 发送完成仍是本轮调度的完成条件。
