# SYNC、SPI帧与寄存器协议

更新：2026-11-12。MCU是SPI主机，FPGA是SPI从机；SYNC由FPGA输出，READY由MCU输出。本版五根信号加共地，物理位置见上板文档。

## 1. 一轮事件

MCU初始化成功、空闲且无故障时READY=1。PS只有在FPGA空闲、READY=1、CS高时才可请求SYNC。

- 第一次p：FPGA输出10us SYNC；MCU的P1上升沿ISR拉低P12 READY并记录事件，主循环采A、填头，结束后READY高；没有上一帧可发。FPGA必须看到READY低后高才置primed。
- 之后c：PS先武装47136B DMA，再SYNC。MCU采B并填头，然后发上一轮A；下一次采A发B。
- 每轮ADC启动后100us预写下一波束，等待ADC DMA完成并关闭RF，再LOAD下一波束，随后发送上一帧。不是ADC和SPI物理同时工作。
- FPGA先缓存并验证整帧，再送FIFO/DMA。PS同时确认AXIS结束和DMA写DDR结束，检查CPU读取结果后ACK，才释放下一次请求。

READY是硬件往返确认，替代旧版的固定prime等待。prime_wait_cycles这个历史端口名保留，但含义改为ACK最大等待时间，默认50ms；到时没有READY往返就报错，不能当成功。

## 2. MCU缓冲区与所有权

A=0x20008000，B=0x20014000，各49152B。ADC采集24×512×4B。发送起点为缓冲基址+2016，先32B元信息，再后23个chirp的47104B采样，合计47136B。

状态：FREE→CAPTURING→READY→SENDING→FREE。帧头和有效载荷同在一个缓冲区，填头在ADC DMA完成后。ISR不切换缓冲区、不发送SPI。

采集时快照seq/beam/angle/resolution；发送上一缓冲区时不按当前波束重写头。默认beam9=0°；扫描开启时依次10、11…18、0…。

```text
SYNC0: A <- seq0/beam9；无发送
SYNC1: B <- seq1/beam10；发送A(seq0/beam9)
SYNC2: A <- seq2/beam11；发送B(seq1/beam10)
```

扫描关闭时角度一直0°，序号仍逐帧递增。SPI真正Busy=0、CS高后才释放旧缓冲。错误锁定，防止覆盖待发数据。

## 3. SPI与帧头

Mode0（CPOL=0，第一上升沿采样）、MSB先发、16bit word。MCU APB保持50MHz，显式SCR=0、CPSR=4，目标12.5MHz。CS在整帧低，发23568个16bit word；帧内允许停钟。无需MISO。

| Word | 定义 |
|---|---|
| 0 | Magic=0xA610 |
| 1 | Version=3；接收器兼容Version2 |
| 2 | 头16个word |
| 3 | 采集时波束index，0..18 |
| 4 | int16角度，0.01°单位，-4500+index×500 |
| 5 | 总波束19 |
| 6 | 每chirp 512点 |
| 7 | 发送23 chirp |
| 8 | 实际采集24 chirp |
| 9 | 每复数点4字节 |
| 10/11 | 理论距离分辨率µm，低16/高16；1400MHz当前配置为107142µm |
| 12 | 总帧长47136B |
| 13 | 头长度32B |
| 14 | V3采集序号，低16位回卷；V2保留0 |
| 15 | Word0..14 XOR16 |

序号第一帧建立基准，后续检查+1。V2不能证明序号连续。本地头XOR与PS的local_payload_crc都不构成源端载荷CRC验证。

ADC FIFO每个32bit字为{Q[15:0],I[15:0]}，当前采集配置选择ADC0。SPI顺序发RAM中的16bit word；FPGA配对为{second_word,first_word}，DDR按小端放字节。Magic在DDR为10 A6，不是A6 10。16bit是容器位宽，不表示ADC有效精度必然16bit。

帧头没有ADC采样率字段。后续FFT时间/频率轴必须来自真实ADC配置，不能用SPI速率、UART波特率或帧到达间隔代替ADC采样率。

## 4. GPIO寄存器

| 地址 | 方向/含义 |
|---|---|
| 0x41200000 | PS写command |
| 0x41200008 | PS读status |
| 0x41210000 | prime_wait_cycles，100MHz周期数；0使用默认5000000（50ms ACK超时） |
| 0x41210008 | timeout_cycles；0使用默认20000000（200ms接收/流转超时） |

command位：0请求翻转位；1首次prime；2软复位高有效；3表示PS已武装DMA；4是PS确认DDR完成的ACK翻转位。bit3是软件承诺，不是FPGA自行读取DMA状态。

status：

| 位 | 含义 |
|---|---|
| 0 | FPGA事务忙 |
| 1 | 错误 |
| 2 | 完整SPI帧有效 |
| 3 | 已处理的request翻转值 |
| 4 | 请求被拒绝 |
| 5 | 心跳 |
| 6 | 可请求：FPGA空闲、SYNC低且MCU READY高 |
| 7 | 已完成首次READY往返确认 |
| 23:8 | 正常时为收到的16bit SPI字数 |
| 27:24 | 状态：0 IDLE，1 PRIME，2 RECEIVE，3 FETCH，4 PUT，5 DRAIN，6 WAIT_ACK，7 FAULT |
| 28 | 最后一个AXIS字实际握手 |
| 29 | 回显DMA armed |
| 30 | 回显prime命令 |
| 31 | 三级同步后的真实MCU READY |

FAULT时bit23:8改为{error_code, received_words[7:0]}，只能恢复字数低8位，不能当完整字数读。

错误：01非法请求（含MCU未就绪/未prime/未武装DMA）；02头字段/XOR；04序号不连续；08超长；10短帧/非完整16bit/无有效头；20接收窗口外CS；40 READY确认、接收或流转超时。忙时请求只记拒绝并继续原事务，不输出新SYNC或丢原SPI字。

## 5. DMA与复位

AXI DMA基址0x40400000，S2MM控制+0x30、状态+0x34、目的地址+0x48、长度+0x58。简单模式，S2MM启用，长度16位。每帧11784个32bit握手，TKEEP=0xF，最后一次TLAST。TREADY低时TDATA/TLAST保持。

WAIT_ACK不设置接收超时，允许PS在DDR完成后停断点。MCU停止响应或中途断点仍可能触发接收超时，属于合理失败，不能伪报完成。

任何一端复位都不会自动清另一端状态。重新prime要重置MCU和FPGA两端会话；当前没有跨板RESET线或SPI重同步命令。禁止只复位FPGA后直接当作MCU也是第一轮。

## 6. 自检版格式

Keil radar_sync_selftest的头与真实版相同。采集RAM的第k个32bit样本：I容器=k&0x7FFF，Q容器=(0x8000|采集序号)&0xFFFF。第一个chirp丢弃，因此线上第一点I=512，之后递增到12287。PS的v逐点比较11776个有效样本。

这不是雷达回波、不是正弦波，不能作为FFT性能测试或目标检测结果。真实版必须用c/d和实际回波分析，不应拿v图案比较作为通过标准。
