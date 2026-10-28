# 代码阅读指南：跟着一帧数据走

建议同时打开本文件和代码。每个主要文件都有编号分段；第一次先读分段标题，再读关键条件，最后跟寄存器在时钟沿的变化。

## 1. 先区分三个执行位置

| 语言/位置 | 负责什么 | 入口 |
|---|---|---|
| Verilog / FPGA PL | 每个时钟沿发波、采样、打包、握手 | `rtl/z100_adc_top.v`，内部BD |
| C / Zynq ARM PS | 初始化、收命令、配寄存器、准备DMA、检查DDR、回串口 | `software/main.c::main()` |
| JavaScript / 电脑浏览器 | 连接串口、解析帧、FFT、绘图 | `web/index.html`加载`signal.js`与`app.js` |

Tcl是Vivado/SDK自动化脚本，不是下载到FPGA执行的业务逻辑。Python是可选的电脑采集/分析工具及打包/校验工具。

Verilog多个`always`并行工作；`<=`的右侧读旧值，本时钟沿之后才更新左侧。C/JS函数中的语句一般顺序执行，串口读取/用户点击则通过事件或等待推进。

## 2. 例子：100 kHz、4096点、delay=3、amp=8

网页发送：`m 100000 4096 3 8`加换行。

1. PS计算`K=round(100000×2^32/10000000)=42949673`。
2. 写新GPIO：channel1=K，channel2打包N/delay/amp。等待1 ms，保证稳定总线先于触发跨域。
3. 把DDR缓冲和保护区填0xa5并flush Cache，提交DMA接收地址和**4096字节**长度。
4. 设置`dma_armed`，翻转START位。ADC域同步后锁存参数。
5. PL清零相位、复位CORDIC，等待启动和首个有效输出，再经过delay阶段。
6. 连续采4096个8位点，每4点写一个32位FIFO字，共1024字。
7. AXIS握手把1024字交给DMA，最后一字携带TLAST。
8. PS等DMA实际完成，Invalidate Cache，读DDR检查计数/保护区，ACK使PL停波并回空闲。
9. 串口发送BEGIN、4096个十六进制字节、END统计/CRC和成功标志。
10. 浏览器检查完整性后FFT/绘图。下一帧由用户再次点击，不能把打印间隔当成连续采样间隔。

## 3. `z100_adc_top.v`：板级接线图

文件分为端口、时钟/复位、AD转发时钟、DA转发时钟、BD封装五段。

- `sys_clk_p/n`：板载100 MHz差分时钟，经Clocking Wizard生成驱动/采样时钟。
- `ad_clk`：送给AD模块的时钟；`adc_sample_clk`用于FPGA采数据，相差半个周期。
- `da_data`：8位DA码；DA时钟上升沿相对数据更新延后半周期，为建立时间留余量。
- `board_reset_n = sys_rst_n & adc_locked`：时钟未锁定时保持复位。
- DDR/FIXED_IO：PS固定接口，不是你逐点手写驱动的DDR并行逻辑。

封装引脚与时序数值在`constraints/z100_adc.xdc`。代码里的8位向量并不能说明接哪个排针，必须同时看XDC与板卡原理图。

## 4. `measurement_core.v`：测量主状态机

### 4.1 控制/状态与跨域

`axis_clk=100 MHz`，`adc_clk=10 MHz`，没有固定相位关系。START/ACK用toggle加两级同步，避免一个AXI短脉冲被慢时钟漏掉。K和参数先保持稳定，再触发START；两级同步不能替代任意多位总线的握手协议。

| command位 | 含义 | PS做法 |
|---|---|---|
| bit0 | START toggle | 每次新任务异或1 |
| bit1 | RAMP模式 | 1为递增码，0为真实AD |
| bit2 | 软件复位电平 | 整条控制/FIFO路径复位 |
| bit3 | DMA已提交接收 | DMA准备成功后置1 |
| bit4 | DMA完成ACK toggle | DDR完成后异或16 |

配置参数是`N[12:0]、delay[20:13]、amplitude[27:21]`。状态字段完整表见[IMPLEMENTATION.md](IMPLEMENTATION.md)。

### 4.2 相位与定点码

相位每个有效输入沿执行`phase <= phase + increment`，32位溢出自然取模。K越大，一周走得越快。输入相位转换成CORDIC的16位Q3.13 π单位，不能直接把原始32位累加器当成IP输入。

CORDIC高16位正弦采用Q2.14，`16384`对应数值1。乘数字幅度后算术右移14位，再围绕128转换成DA码并限幅。这里的减号用于配套模块反相补偿，实际增益/极性仍需测量。

不要把CORDIC和FFT混淆：CORDIC根据相位算正弦；FFT分析已经采到的一段数据里有什么频率。

### 4.3 状态如何走

| 状态 | 本状态做什么 | 何时离开 |
|---|---|---|
| IDLE | DA保持128，等待START | 已armed且参数合法 |
| PREP | 清相位、复位CORDIC | 四个准备周期结束 |
| WAIT_ZERO | 等首个有效正弦输出 | cordic_valid有效 |
| DELAY | 等可配置整数周期补偿 | 延迟计数结束 |
| CAPTURE | 连续采N点并组字 | 第N点完成，或FIFO满进入错误 |
| WAIT_DMA | 采样已结束，继续等实际DDR完成确认 | PS翻转ACK |
| ERROR | 停止本次任务，记录原因 | 软件整条路径复位 |

`cordic_warm`在释放复位后再等3拍才送相位输入，确保第一笔相位0不被跳过。真正启动时机以有效信号为准，不凭猜测写死CORDIC流水延迟。

实际第一笔采样在首个零相位DA码更新后的`delay+2`个采样周期。ADC流水、模拟滤波和线缆延迟都影响实物结果。整数delay的步进是100 ns，不能提供任意精细的模拟相位调节。

### 4.4 四点怎样打包

```text
第1点 s0 → pack[7:0]
第2点 s1 → pack[15:8]
第3点 s2 → pack[23:16]
第4点 s3 → 本拍fifo_din={s3,pack}，同时wr_en=1

32位字： [31:24] [23:16] [15:8] [7:0]
             s3      s2      s1     s0
```

输入`00 01 02 03`得到`0x03020100`；在本工程小端ARM DDR缓冲中按字节读回仍是`00 01 02 03`。本拍时序逻辑清空pack，不会改变FIFO在同一沿已经采入的数据；非阻塞赋值读旧值是关键。

FIFO满时中止并上报，不能暂停模拟正弦、等DMA准备好后再假装采样仍连续。

## 5. `fifo_to_axis.v`：握手不是“发一个脉冲”

FIFO是32位入/32位出、1024深度、独立时钟、FWFT。**非空时dout已经是队首数据**，所以可以直接接TDATA。若改成Standard FIFO，必须重写读延迟适配，不能只改IP选项。

| TVALID | TREADY | 结果 |
|---|---|---|
| 0 | 任意 | 没有有效传输，不读FIFO |
| 1 | 0 | 保持数据/TLAST，不能移动读指针 |
| 1 | 1 | 本时钟沿成功传一字，rd_en=1、计数前进 |

TKEEP=F表示4个字节都有效；第`N/4-1`个字携带TLAST。word_count只按成功握手计数，而不是按经过的时钟数计数。

几个完成条件必须分开：

| 标志 | 只证明什么 |
|---|---|
| frame_done | PL已经采到第N个样本 |
| stream_done | 最后一个AXIS字已握手交给下游 |
| DMA IOC且不忙 | DMA已经完成本次DDR写入 |
| PS ACK | 软件核对完成，允许控制器结束本次任务 |

## 6. `software/main.c`：顺着编号读

1. BSP头文件与FS/MAX_N：这些驱动在SDK BSP提供，不是普通电脑gcc自带。
2. 驱动实例：control GPIO0通道1命令/通道2状态；config GPIO配置K和参数。
3. timeout/CRC/status：统一的辅助工具。
4. reset_path：复位PL/FIFO与DMA，释放后等待稳定。
5. measure：本文件主流程，按5.1～5.9的注释追踪一帧。
6. handle：将m/t/s/r/q等字符串转为函数调用。
7. main：初始化UART/GPIO/DMA，再逐字符组行；它不自动测量。

特别看三件事：

- **缓存一致性**：DMA读写DDR不自动等价于CPU Cache内容；先flush，完成后invalidate再读。
- **先准备DMA再START**：CPU不是精确采样时钟，采样边沿由PL状态机决定。
- **保留原始错误**：errors、保护区、超时、DMA错误不能忽略；失败后须复位。

`cmd & 17`保留bit0/bit4，`|8`设置armed，`^1`翻转START，`^16`翻转ACK。读到源码里的数字时，可对照前面的位表理解，避免把它们误认成任意常数。

采集轮询期间只识别q，其它输入不排队；不要连续发送多条测量命令。打印前已经ACK并停波，但打印本身仍需要时间。

## 7. 网页两份JS如何配合

### `signal.js`是纯算法

- `crc32`：与PS一致的校验。
- `parsePacket`：BEGIN→数据区→END→成功标志，验证后才返回样本。
- `parseCSV`：只验证格式，没有板端来源CRC。
- `fft`：位反转重排后逐级蝶形运算，复杂度约N log2 N。
- `fitPhase`：按已知参考频率拟合`A sin + B cos + DC`，相位是atan2(B,A)。
- `analyze`：去DC→Hann窗→FFT→单边幅度→峰值插值→相位拟合。

这些函数不访问DOM/串口，因此同一份代码能直接在Node里测试。

### `app.js`负责事件与状态

按13个编号分段阅读：页面状态→按钮互锁→send→start→receive→readLoop→连接/断开→控件事件→render→离线数据→Canvas绘图→导出→首次加载。

`read()`得到的是任意大小的字节片段，**不是完整一行**。`lineBuffer`先拼接、按换行切开，`packet`再把多行拼成一帧。只有成功标志到达并通过CRC/参数核对，才调用render。

来源标签分“串口实测/仿真演示/CSV导入”，不能因为图画出来了就统一标成实测。忙时禁止改参数，pending记录发出去的请求，避免把别的帧误当本次结果。

Canvas的sx/sy只做数值到像素的映射，不修改原始samples。CSV始终导出全帧，不受屏幕“前三周期”选项影响。

## 8. 仿真在证明什么

- `tb_measurement.v`：实际CORDIC/FIFO IP、数学sin对照、打包记分牌、背压/重复帧/非法参数/溢出恢复。
- `tb_loop_ddr.v`：完整BD，通过PS VIP读写配置与DMA寄存器，读回DDR逐字节比较及检查保护区。
- `pc/test_analysis.py`、`web/test_signal.cjs`：频率/相位估计、合法帧及故意篡改的负例。

PS VIP发AXI事务，不执行ARM C指令；数字AD模型不等于实际ADC/模拟滤波器。仿真PASS与SDK编译PASS不能代替真实板上程序运行。

## 9. 第一次修改建议

先只在网页改变频率、N、幅度、delay，保存结果并理解变化。之后可在已有测试里增加一个频率/点数场景。改RTL后依次刷新BD、仿真、构建；改PS后同步main.c并重编译ELF。不要一开始同时改变引脚、时钟、打包顺序和协议，这样错误很难定位。

完整使用流程见[README](../README.md)，实际测试见[BOARD_TEST.md](BOARD_TEST.md)。
