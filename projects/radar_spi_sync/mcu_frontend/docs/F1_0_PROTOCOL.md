# f1_0 UART0 协议

f1_0 保留 **FMCW 带宽修改** 和 **帧周期修改**，不再支持查询、槽位、保存配置、ADC 查询、频谱查询、距离/位移查询等命令，也不发送 UART0 应答；P0/TX0 已作为 SPI 数据流片选。

## 串口

- UART：UART0
- RX：P1
- 波特率：115200 bit/s
- 仅 RX

## 帧格式

```text
A0 55 LL HH CMD PAYLOAD... XOR
```

- `LL HH`：payload 长度，16 bit little-endian。
- `XOR`：从 `A0` 起到 payload 最后一个字节的逐字节 XOR。
- f1_0 最大 payload 为 2 B。

## 命令 0x29：修改带宽

```text
A0 55 02 00 29 BW_L BW_H XOR
```

`BW_L/BW_H` 为 MHz，little-endian `uint16`。

有效范围：80..4000 MHz。默认值：1400 MHz。收到有效且与当前值不同的配置后立即执行 PLL/FMCW 重配置；**不写 Flash**，复位后恢复默认 1400 MHz。

例：1400 MHz = `0x0578`，待发送前 7 字节为：

```text
A0 55 02 00 29 78 05
```

其 XOR 校验为 `A3`，完整帧：

```text
A0 55 02 00 29 78 05 A3
```

其它 CMD 或错误长度/校验/范围会被静默忽略。

## 命令 0x2A：修改帧周期

```text
A0 55 02 00 2A MS_L MS_H XOR
```

`MS_L/MS_H` 为帧周期，单位 ms，little-endian `uint16`。

有效范围：1..60000 ms。默认值：50 ms。收到有效配置后从后续调度周期开始生效；**不写 Flash**，复位后恢复默认 50 ms。

例：50 ms = `0x0032`，待发送前 7 字节为：

```text
A0 55 02 00 2A 32 00
```

其 XOR 校验为 `EF`，完整帧：

```text
A0 55 02 00 2A 32 00 EF
```

## SPI0 ADC 输出帧

SPI0 每帧固定输出 `PROJECT_SPI_STREAM_BYTES = 47136 B`，16 bit word 传输。RAM 中每帧第一个 chirp 不作为 ADC 数据输出，只在该 chirp 末尾写入 32 B 元信息；固件从这 32 B 元信息处开始顺序写 SPI0 TX FIFO，后续 `23 chirp × 512 sample × 4 B = 47104 B` 为原始复数 ADC 数据。

元信息区按 little-endian `uint16` 数组解释：

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
