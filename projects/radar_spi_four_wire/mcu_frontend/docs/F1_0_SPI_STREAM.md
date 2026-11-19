# f1_0 SPI0 原始 ADC 数据流

## 电气/外设配置

- SPI0 Master
- 12.5 MHz SCLK（实测时钟源下 SPI 分频 `/4`）
- 16-bit frame
- CPOL=0，CPHA=1Edge（沿用工程 SPI driver 默认）
- CLK：P3
- MOSI：P6
- `/CS`：P0（原 UART0_TX），低有效
- P4（板载 SPI Flash `/CS`）：固定高
- MISO/P5：发送链路不使用

## 每帧格式

`/CS` 的一次低电平区间就是一帧。RAM 中每帧第一个 chirp 不作为 ADC 数据输出，只在该 chirp 末尾写入 32 B 元信息；发送时固件顺序写 SPI0 TX FIFO 输出这 16 个元信息 word 和后续 23 个原始 ADC chirp。

每帧大小：

```text
32 bytes metadata + 23 chirps × 512 samples/chirp × 4 bytes/complex sample
= 47136 bytes = 23568 × 16-bit SPI words
```

元信息位于第一个 chirp 末尾，地址偏移为 `2016 B`，按 little-endian `uint16` 数组解释：

| Word | 含义 |
|---:|---|
| 0 | Magic：`0xA610` |
| 1 | 元信息版本：`2` |
| 2 | 元信息 word 数：`16` |
| 3 | 波束序号：`0..18` |
| 4 | 波束角度，单位 0.01°，有符号 int16：`-4500..4500` |
| 5 | 总波束数：`19` |
| 6 | 每 chirp 采样点数：`512` |
| 7 | SPI 有效 ADC chirp 数：`23` |
| 8 | ADC 实际采集 chirp 数：`24` |
| 9 | 每个复数采样字节数：`4` |
| 10 | 距离分辨率低 16 bit，单位 um |
| 11 | 距离分辨率高 16 bit，单位 um；`resolution_um = word10 \| (word11 << 16)` |
| 12 | SPI 线上帧长，单位 byte：`47136` |
| 13 | 元信息长度，单位 byte：`32` |
| 14 | 保留：`0` |
| 15 | 元信息 XOR16 校验：word0..word14 逐 word 异或 |

有效 ADC payload 长度可由 word12 - word13 得到，即 `47104 B`。

ADC0 DMA 保持原 32-bit IQ 存储格式：低 16 bit 为 I，高 16 bit 为 Q。因此 SPI 以 16-bit word 顺序读取 RAM 时，线上字序为：

```text
metadata word0, metadata word1, ... metadata word15,
I(chirp1,sample0), Q(chirp1,sample0),
I(chirp1,sample1), Q(chirp1,sample1), ...,
I(chirp23,sample511), Q(chirp23,sample511)
```

所有值均为原始有符号 16-bit ADC 分量；f1_0 不做平均、FFT、相位、距离或其它算法处理。

## 波束序列

每个 `/CS` 帧对应一个波束，固定循环：

```text
-45, -40, -35, -30, -25, -20, -15, -10, -5,
  0,  +5, +10, +15, +20, +25, +30, +35, +40, +45 deg
```

上电后的第一帧就是 `-45°`，之后按上述顺序循环；接收端也可直接使用元信息区里的波束序号和角度。

## Ping-pong

- A：`0x20008000`，48 KiB
- B：`0x20014000`，48 KiB

稳态时，SPI 只读上一帧缓冲区，ADC DMA1 只写另一块缓冲区；两块在 SPI 发送完成后交换角色。

12.5 MHz 下发送 47136 B 的理想线时长约 30.17 ms。默认运行周期为 50 ms，可通过 UART0 `0x2A` 命令修改；实际周期由采集、SPI 发送和等待边界共同约束。
