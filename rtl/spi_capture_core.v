`timescale 1ns/1ps
// SPI-only acquisition controller. PS generates source bytes through SPI0 slave.
// command: bit0 START toggle, bit1 source-mode tag, bit2 reset, bit3 DMA armed,
// bit4 DDR-complete ACK toggle. Stable config is held until ACK.
module spi_capture_core(
    input wire axis_clk, spi_clk16, rst_n,
    input wire [31:0] command, phase_word, config_word,
    input wire fifo_full, stream_done, spi_miso,
    output wire spi_sclk, spi_cs_n, spi_mosi,
    output wire [31:0] fifo_din,
    output wire fifo_wr_en, fifo_reset, axis_reset_n,
    output wire [12:0] frame_samples_axis,
    output wire [31:0] status
);
    localparam IDLE=0, PREP=1, DELAY=3, CAPTURE=4, WAIT_DMA=5, ERROR=6;
    wire logic_rst_n=rst_n && !command[2];
    (* ASYNC_REG="TRUE" *) reg [1:0] ar,xr,req_sync,ack_sync,mode_sync,armed_sync;
    (* ASYNC_REG="TRUE" *) reg [31:0] cfg_meta,cfg_sync;
    wire spi_reset_n=ar[1];
    reg [3:0] state;
    reg req_seen,ack_seen,test_mode,rejected,overflow,frame_done;
    reg [12:0] count,length;
    reg [1:0] prep_count;
    reg [23:0] pack;
    reg [15:0] heartbeat_count;
    reg heartbeat;
    wire [7:0] sample;
    wire spi_busy,sample_valid,spi_frame_done;
    (* MARK_DEBUG="TRUE",KEEP="TRUE" *) wire [63:0] spi_parallel64;
    wire spi_start=spi_reset_n && state==CAPTURE && count==0 &&
                   !spi_busy && !sample_valid && !fifo_full;
    spi_master_burst8 u_spi_master(
        .clk16(spi_clk16),.rst_n(spi_reset_n),.start(spi_start),.miso(spi_miso),
        .sclk(spi_sclk),.cs_n(spi_cs_n),.mosi(spi_mosi),.busy(spi_busy),
        .rx_data(sample),.rx_valid(sample_valid),
        .frame_data(spi_parallel64),.frame_done(spi_frame_done));
    // Earliest byte goes into bits 7:0. Exactly 8 bytes produce two FIFO words.
    assign fifo_din={sample,pack};
    assign fifo_wr_en=spi_reset_n && state==CAPTURE && sample_valid &&
                      count<8 && !fifo_full && count[1:0]==3;
    assign fifo_reset=!logic_rst_n;
    assign axis_reset_n=xr[1];
    always @(posedge spi_clk16 or negedge logic_rst_n)
        if(!logic_rst_n) ar<=0; else ar<={ar[0],1'b1};
    always @(posedge axis_clk or negedge logic_rst_n)
        if(!logic_rst_n) xr<=0; else xr<={xr[0],1'b1};
    always @(posedge spi_clk16 or negedge spi_reset_n) begin
        if(!spi_reset_n) begin
            req_sync<=0;ack_sync<=0;mode_sync<=0;armed_sync<=0;
            cfg_meta<=0;cfg_sync<=0;state<=IDLE;req_seen<=0;ack_seen<=0;
            test_mode<=0;rejected<=0;overflow<=0;frame_done<=0;
            count<=0;length<=8;prep_count<=0;pack<=0;
            heartbeat_count<=0;heartbeat<=0;
        end else begin
            req_sync<={req_sync[0],command[0]};ack_sync<={ack_sync[0],command[4]};
            mode_sync<={mode_sync[0],command[1]};armed_sync<={armed_sync[0],command[3]};
            cfg_meta<=config_word;cfg_sync<=cfg_meta;
            heartbeat_count<=heartbeat_count+1'b1;
            if(&heartbeat_count) heartbeat<=~heartbeat;
            if(req_sync[1]!=req_seen) begin
                req_seen<=req_sync[1];
                if(state!=IDLE || !armed_sync[1] || cfg_sync[12:0]!=8) begin
                    rejected<=1;
                    if(state==IDLE) state<=ERROR;
                end else begin
                    test_mode<=mode_sync[1];rejected<=0;overflow<=0;frame_done<=0;
                    count<=0;length<=8;pack<=0;prep_count<=0;
                    ack_seen<=ack_sync[1];state<=PREP;
                end
            end else case(state)
                PREP: if(prep_count==3) state<=DELAY; else prep_count<=prep_count+1'b1;
                DELAY: state<=CAPTURE;
                CAPTURE: begin
                    if(fifo_full) begin overflow<=1;state<=ERROR;end
                    else if(sample_valid && count<8) begin
                        case(count[1:0])
                            0:pack[7:0]<=sample;
                            1:pack[15:8]<=sample;
                            2:pack[23:16]<=sample;
                            3:pack<=0;
                        endcase
                        count<=count+1'b1;
                        if(count==7) begin frame_done<=1;state<=WAIT_DMA;end
                    end
                end
                WAIT_DMA: if(ack_sync[1]!=ack_seen) begin ack_seen<=ack_sync[1];state<=IDLE;end
                default: begin end
            endcase
        end
    end
    // Retain the existing PS status layout; bit7 is reserved zero.
    wire [31:0] spi_status_next={4'd0,(state==WAIT_DMA),1'b0,1'b0,state,count,
        1'b0,spi_reset_n,heartbeat,rejected,req_seen,frame_done,overflow,
        (state!=IDLE && state!=ERROR)};
    reg [31:0] spi_status;
    always @(posedge spi_clk16 or negedge spi_reset_n)
        if(!spi_reset_n) spi_status<=0; else spi_status<=spi_status_next;
    (* ASYNC_REG="TRUE" *) reg [31:0] smeta,ssync;
    reg streamed,req_axis;
    always @(posedge axis_clk or negedge axis_reset_n)
        if(!axis_reset_n) begin smeta<=0;ssync<=0;streamed<=0;req_axis<=0;end
        else begin
            smeta<=spi_status;ssync<=smeta;req_axis<=command[0];
            if(command[0]!=req_axis) streamed<=0;
            else if(stream_done) streamed<=1;
        end
    assign frame_samples_axis=13'd8;
    assign status=(ssync & 32'hfdffffff) | ({31'd0,streamed}<<25);
    // phase_word is retained as a PS register-interface compatibility field;
    // it does not synthesize a waveform here. Source generation is in PS C.
endmodule
