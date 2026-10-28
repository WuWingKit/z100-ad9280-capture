# CI入口：只核对源码配置和已存证据，不运行Vivado、不连接开发板。
"""Portable source baseline checks; no original-machine paths or hardware access."""
from pathlib import Path
import json,re,xml.etree.ElementTree as ET

root=Path(__file__).resolve().parents[1]
# ===== 1. 核对器件、FIFO、DMA和引脚参数 =====
bd=json.loads((root/'bd_start.srcs/sources_1/bd/capture_bd/capture_bd.bd').read_text())['design']
assert bd['design_info']['device']=='xc7z035ffg900-2', 'Review device changes explicitly'
fifo=bd['components']['fifo_generator_0']['parameters']
assert fifo['Input_Data_Width']['value']=='32'
assert fifo['Performance_Options']['value']=='First_Word_Fall_Through'
assert fifo['Fifo_Implementation']['value']=='Independent_Clocks_Builtin_FIFO'
dma=bd['components']['axi_dma_0']['parameters']
assert dma['c_include_mm2s']['value']=='0' and dma['c_include_sg']['value']=='0'
xdc=(root/'constraints/z100_adc.xdc').read_text()
for i,pin in enumerate(['AD13','AC16','AD14','AC17','AB16','AB14','AB17','AB15']):
    assert re.search(r'PACKAGE_PIN '+pin+r' IOSTANDARD LVCMOS33.*ad_data\['+str(i)+r'\]',xdc)
assert 'PACKAGE_PIN AA17 IOSTANDARD LVCMOS33' in xdc
ET.parse(root/'bd_start.srcs/sources_1/ip/adc_clk_wiz/adc_clk_wiz.xci')
for path in ['rtl/z100_adc_top.v','rtl/adc_board_control.v','software/main.c','sim/tb_z100_hw.v','POWER_CHECK.md','CONTRIBUTING.md']:
    assert (root/path).is_file(), path
# ===== 2. 历史板上递增码证据，用于追溯原版本 =====
data=(root/'evidence/20261022_progress/ramp_screenshot_transcript.txt').read_text()
samples=[int(v,16) for line in data.splitlines() if re.fullmatch(r'[0-9A-F]{2}( [0-9A-F]{2})*',line) for v in line.split()]
assert samples==list(range(100)) and sum(samples)==4950
print('REPOSITORY_BASELINE_PASS: BD/XCI/XDC inputs and archived evidence consistent')
print('Not a Vivado build, physical ADC test, or electrical safety certification.')

# ===== 3. 当前CORDIC版本配置、关键文件和离线验证记录 =====
cordic=bd['components']['cordic_sincos_0']['parameters']
for k,v in {'Functional_Selection':'Sin_and_Cos','Phase_Format':'Scaled_Radians',
            'Input_Width':'16','Output_Width':'16','ARESETN':'true'}.items():
    assert cordic[k]['value']==v,(k,cordic[k])
# Depth 1024 is the FIFO Generator 13.2 default and may be omitted in BD JSON.
assert fifo.get('Input_Depth',{'value':'1024'})['value']=='1024'
for path in ['rtl/measurement_core.v','rtl/fifo_to_axis.v','sim/tb_measurement.v',
             'sim/tb_loop_ddr.v','pc/capture_analyze.py','docs/CDC_REVIEW.md']:
    assert (root/path).is_file(),path
verification=json.loads((root/'evidence/verification.json').read_text())
assert verification['hardware_operations_performed'] is False
assert verification['timing_met'] is True
assert 'PASS frames=5' in verification['core_simulation']
assert 'PASS frames=2' in verification['bd_dma_ddr_simulation']
print('CORDIC_SOURCE_CHECK_PASS: current configuration and archived offline evidence consistent')

