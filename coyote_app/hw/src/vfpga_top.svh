`ifdef EN_STRM

localparam integer INPUT_DATA_BITWIDTH = 64;

logic [N_STRM_AXI-1:0][INPUT_DATA_BITWIDTH-1:0]        axis_host_recv_tdata;
logic [N_STRM_AXI-1:0]                          axis_host_recv_tvalid;
logic [N_STRM_AXI-1:0]                          axis_host_recv_tready;
logic [N_STRM_AXI-1:0][INPUT_DATA_BITWIDTH/8-1:0]      axis_host_recv_tkeep;
logic [N_STRM_AXI-1:0]                          axis_host_recv_tlast;
logic [N_STRM_AXI-1:0][INPUT_DATA_BITWIDTH-1:0]         axis_host_recv_tid;



// Data width converter: Coyote is built around 512-bit AXI streams but the encryption block expects 128-bit inputs
// NOTE: Coyote makes no assumptions (or guarantees) about the TID of a stream
// And here, the i-th stream should have TID = i, to match the i-th Coyote Thread in software
// Therefore, pass i here to the s_axis_tid port and it will be propagated all the way to the output through all the other blocks
for (genvar i = 0; i < N_STRM_AXI; i++) begin
    dwidth_input_512_64 inst_dwidth_input (
        .aclk(aclk),
        .aresetn(aresetn),

        .s_axis_tdata(axis_host_recv[i].tdata),
        .s_axis_tvalid(axis_host_recv[i].tvalid),
        .s_axis_tready(axis_host_recv[i].tready),
        .s_axis_tkeep(axis_host_recv[i].tkeep),
        .s_axis_tlast(axis_host_recv[i].tlast),
        .s_axis_tid(i),

        .m_axis_tdata(axis_host_recv_tdata[i]),
        .m_axis_tvalid(axis_host_recv_tvalid[i]),
        .m_axis_tready(axis_host_recv_tready[i]),
        .m_axis_tkeep(axis_host_recv_tkeep[i]),
        .m_axis_tlast(axis_host_recv_tlast[i]),
        .m_axis_tid(axis_host_recv_tid[i])
    );
end

logic [INPUT_DATA_BITWIDTH-1:0]        axis_host_send_tdata;
logic                           axis_host_send_tvalid;
logic                          axis_host_send_tready;
logic [INPUT_DATA_BITWIDTH/8-1:0]      axis_host_send_tkeep;
logic                           axis_host_send_tlast;

dwidth_output_32_512 inst_dwidth_output (
        .aclk(aclk),
        .aresetn(aresetn),

        .s_axis_tdata(axis_host_send_tdata),
        .s_axis_tvalid(axis_host_send_tvalid),
        .s_axis_tready(axis_host_send_tready),
        .s_axis_tkeep(axis_host_send_tkeep),
        .s_axis_tlast(axis_host_send_tlast),
        .s_axis_tid(0),

        .m_axis_tdata(axis_host_send[0].tdata),
        .m_axis_tvalid(axis_host_send[0].tvalid),
        .m_axis_tready(axis_host_send[0].tready),
        .m_axis_tkeep(axis_host_send[0].tkeep),
        .m_axis_tlast(axis_host_send[0].tlast),
        .m_axis_tid(0)
    );



mutual_information_master inst_mutual_information_master(
    .s_input_img_TDATA        (axis_host_recv_tdata[0]),
    .s_input_img_TKEEP        (axis_host_recv_tkeep[0]),
    .s_input_img_TLAST        (axis_host_recv_tlast[0]),
    .s_input_img_TSTRB        (0),
    .s_input_img_TVALID       (axis_host_recv_tvalid[0]),
    .s_input_img_TREADY       (axis_host_recv_tready[0]),

    .s_input_ref_TDATA        (axis_host_recv_tdata[1]),
    .s_input_ref_TKEEP        (axis_host_recv_tkeep[1]),
    .s_input_ref_TLAST        (axis_host_recv_tlast[1]),
    .s_input_ref_TSTRB        (0),
    .s_input_ref_TVALID       (axis_host_recv_tvalid[1]),
    .s_input_ref_TREADY       (axis_host_recv_tready[1]),

    .s_n_couples_TDATA        (axis_host_recv_tdata[2]),
    .s_n_couples_TKEEP        (axis_host_recv_tkeep[2]),
    .s_n_couples_TLAST        (axis_host_recv_tlast[2]),
    .s_n_couples_TSTRB        (0),
    .s_n_couples_TVALID       (axis_host_recv_tvalid[2]),
    .s_n_couples_TREADY       (axis_host_recv_tready[2]),

    .s_mutual_info_TDATA        (axis_host_send_tdata),
    .s_mutual_info_TKEEP        (axis_host_send_tkeep),
    .s_mutual_info_TLAST        (axis_host_send_tlast),
    .s_mutual_info_TSTRB        (),
    .s_mutual_info_TVALID       (axis_host_send_tvalid),
    .s_mutual_info_TREADY       (axis_host_send_tready),

    .ap_clk                 (aclk),
    .ap_rst_n               (aresetn),
    
    .s_axi_control_ARADDR       (axi_ctrl.araddr),
    .s_axi_control_ARVALID      (axi_ctrl.arvalid),
    .s_axi_control_ARREADY      (axi_ctrl.arready),
    .s_axi_control_AWADDR       (axi_ctrl.awaddr),
    .s_axi_control_AWVALID      (axi_ctrl.awvalid),
    .s_axi_control_AWREADY      (axi_ctrl.awready),
    .s_axi_control_RDATA        (axi_ctrl.rdata),
    .s_axi_control_RRESP        (axi_ctrl.rresp),
    .s_axi_control_RVALID       (axi_ctrl.rvalid),
    .s_axi_control_RREADY       (axi_ctrl.rready),
    .s_axi_control_WDATA        (axi_ctrl.wdata),
    .s_axi_control_WSTRB        (axi_ctrl.wstrb),
    .s_axi_control_WVALID       (axi_ctrl.wvalid),
    .s_axi_control_WREADY       (axi_ctrl.wready),
    .s_axi_control_BRESP        (axi_ctrl.bresp),
    .s_axi_control_BVALID       (axi_ctrl.bvalid),
    .s_axi_control_BREADY       (axi_ctrl.bready)
);

// There are two host streams, for both incoming and outgoing signals
// The second outgoing is unused in this example, so tie it off
always_comb axis_host_send[1].tie_off_m();
always_comb axis_host_send[2].tie_off_m();
`endif

// Tie-off unused signals to avoid synthesis problems
always_comb sq_rd.tie_off_m();
always_comb sq_wr.tie_off_m();
always_comb cq_rd.tie_off_s();
always_comb cq_wr.tie_off_s();
always_comb notify.tie_off_m();
// always_comb axi_ctrl.tie_off_s();

/*
NOTE: Due to partial reoncfiguration Coyote renames some the HLS kernels to include a unique ID, for e.g. ```hls_vadd_0```. 
However, this can sometimes cause weird bugs with ILAs. 
Notice how in this example, the ILA IP is called ```ila_vadd``` instead of ```ila_hls_vadd```;
since Coyote iterates through the  files and looks for occurences of ```hls_vadd``` to rename them to ```hls_vadd_0```. 
This can cause ```ila_hls_vadd``` to be changed to ```ila_hls_vadd_0```. 
However, in the IP instantiation (```init_ip.tcl```), the ILA IP is defined as ```ila_hls_vadd``` and 
So the mismatch will cause synthesis errors.
Therefore, whenever possible the HLS kernel name should only be contained in the HLS kernel IP and in no other IP names/instances.
*/
ila_3dmi inst_ila_3dmi (
    .clk(aclk),                             // clock   
 
    .probe0(axis_host_recv[0].tvalid),      // 1
    .probe1(axis_host_recv[0].tready),      // 1
    .probe2(axis_host_recv[0].tlast),       // 1
    .probe3(axis_host_recv[0].tdata),       // 512

    .probe4(axis_host_recv[1].tvalid),      // 1
    .probe5(axis_host_recv[1].tready),      // 1
    .probe6(axis_host_recv[1].tlast),       // 1
    .probe7(axis_host_recv[1].tdata),       // 512

    .probe8(axis_host_recv[2].tvalid),      // 1
    .probe9(axis_host_recv[2].tready),      // 1
    .probe10(axis_host_recv[2].tlast),       // 1
    .probe11(axis_host_recv[2].tdata),       // 512

    .probe12(axis_host_send[0].tvalid),      // 1
    .probe13(axis_host_send[0].tready),      // 1
    .probe14(axis_host_send[0].tlast),      // 1
    .probe15(axis_host_send[0].tdata),       // 512

    .probe16(axi_ctrl.arready),
    .probe17(axi_ctrl.rready),
    .probe18(axi_ctrl.awaddr[17:0]),
    .probe19(axi_ctrl.wdata[31:0]),
    .probe20(axi_ctrl.awvalid),
    .probe21(axi_ctrl.awready),
    .probe22(axi_ctrl.wvalid),
    .probe23(axi_ctrl.wready),
    .probe24(axi_ctrl.araddr[17:0]),
    .probe25(axi_ctrl.rdata[31:0]),
    .probe26(axi_ctrl.wstrb)

);