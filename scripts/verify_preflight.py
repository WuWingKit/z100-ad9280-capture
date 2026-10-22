"""Offline checks only. Does not connect to hardware or certify physical wiring."""
from pathlib import Path
import hashlib, json, re

root = Path(__file__).resolve().parents[1]
bd = json.loads((root/'bd_start.srcs/sources_1/bd/capture_bd/capture_bd.bd').read_text())['design']
report = {'offline_checks': [], 'physical_checks': 'NOT VERIFIED', 'board_test': 'NOT RUN'}
def check(ok, description):
    if not ok: raise RuntimeError(description)
    report['offline_checks'].append(description)

check(bd['design_info']['device'] == 'xc7z035ffg900-2', 'Project device is XC7Z035FFG900-2')
xdc = (root/'constraints/z100_adc.xdc').read_text()
vendor = (root/'board_reference/vendor_hs_ad_da.xdc').read_text(errors='replace')
vendor_pins = {port.strip('{}'):pin for pin,port in re.findall(r'^set_property PACKAGE_PIN (\w+) \[get_ports (.+)\]\s*$', vendor, re.M)}
expected = {'sys_clk_p':'D9','sys_clk_n':'D8','sys_rst_n':'AA25','ad_clk':'AA17','ad_otr':'AE18',
            'ad_data[0]':'AD13','ad_data[1]':'AC16','ad_data[2]':'AD14','ad_data[3]':'AC17',
            'ad_data[4]':'AB16','ad_data[5]':'AB14','ad_data[6]':'AB17','ad_data[7]':'AB15'}
for port,pin in expected.items():
    if port != 'sys_clk_n':
        check(vendor_pins.get(port) == pin, 'Vendor example agrees: '+port+' -> '+pin)
    if port.startswith('ad_'):
        check(bool(re.search(r'PACKAGE_PIN\s+'+pin+r'\s+IOSTANDARD LVCMOS33\}\s+\[get_ports\s+\{?'+re.escape(port)+r'\}?\]',xdc)), '3.3V constraint '+port+' -> '+pin)
    else: check(pin in xdc, 'Board oscillator/reset pin '+port+' -> '+pin)
top=(root/'rtl/z100_adc_top.v').read_text()
check('input wire [7:0] ad_data' in top and 'input wire ad_otr' in top, 'ADC data and OTR are inputs, never FPGA outputs')
check('output wire ad_clk' in top, 'Only ADC drive clock is an AD-facing FPGA output')
check('da_clk' not in top and 'da_data' not in top, 'DAC output logic is not enabled')
ps=bd['components']['processing_system7_0']['parameters']
for key,value in {'PCW_UART0_PERIPHERAL_ENABLE':'1','PCW_UART0_UART0_IO':'MIO 14 .. 15',
                  'PCW_UIPARAM_DDR_BUS_WIDTH':'32 Bit','PCW_UIPARAM_DDR_PARTNO':'MT41K256M16 RE-125'}.items():
    check(ps.get(key,{}).get('value') == value, 'PS '+key+' = '+value)
check(ps.get('PCW_PRESET_BANK0_VOLTAGE',{}).get('value') == 'LVCMOS 3.3V', 'PS MIO UART bank is configured for 3.3V')
refs=json.loads((root/'original_sha256.json').read_text())
for p,digest in refs.items():
    check(hashlib.sha256(Path(p).read_bytes()).hexdigest() == digest, 'Original source unchanged: '+Path(p).name)
check('program_flash' not in '\n'.join(line for line in (root/'scripts/debug_jtag.tcl').read_text().splitlines() if not line.lstrip().startswith('#')), 'Debug script contains no Flash programming command')
report['result']='OFFLINE_STATIC_PASS'
(root/'hardware/offline_preflight.json').write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
print('OFFLINE_STATIC_PASS checks='+str(len(report['offline_checks'])))
