# Run with the debug BD open. Native mode avoids automatic AXI probe expansion.
if {[llength [get_bd_cells -quiet debug_ila_0]]} {delete_bd_objs [get_bd_cells debug_ila_0]}
if {1} {
    create_bd_cell -type ip -vlnv xilinx.com:ip:ila:6.2 debug_ila_0
    set_property -dict [list CONFIG.C_MONITOR_TYPE {Native} CONFIG.C_NUM_OF_PROBES {14} CONFIG.C_DATA_DEPTH {4096}] [get_bd_cells debug_ila_0]
    set widths {1 1 1 1 1 32 1 1 1 1 1 32 16 16}
    set sources {radar_frame_capture_0/radar_sync frontend_sim_0/ready frontend_sim_0/spi_sclk frontend_sim_0/spi_cs_n frontend_sim_0/spi_mosi radar_frame_capture_0/status radar_frame_capture_0/fifo_wr_en radar_fifo_to_axis_0/fifo_rd_en radar_fifo_to_axis_0/m_axis_tvalid radar_fifo_to_axis_0/m_axis_tready radar_fifo_to_axis_0/m_axis_tlast radar_fifo_to_axis_0/m_axis_tdata frontend_sim_0/next_capture_seq frontend_sim_0/last_tx_seq}
    connect_bd_net [get_bd_pins processing_system7_0/FCLK_CLK0] [get_bd_pins debug_ila_0/clk]
    for {set i 0} {$i<14} {incr i} {
        set_property CONFIG.C_PROBE${i}_WIDTH [lindex $widths $i] [get_bd_cells debug_ila_0]
        connect_bd_net [get_bd_pins [lindex $sources $i]] [get_bd_pins debug_ila_0/probe$i]
    }
    # Tapping an interface member makes it a manual connection in Vivado 2018.3.
    # Explicitly join BOTH endpoints so probing never disconnects DMA handshakes.
    foreach signal {tdata tkeep tvalid tready tlast} {
        connect_bd_net [get_bd_pins radar_fifo_to_axis_0/m_axis_$signal] [get_bd_pins axi_dma_0/s_axis_s2mm_$signal]
    }
}
