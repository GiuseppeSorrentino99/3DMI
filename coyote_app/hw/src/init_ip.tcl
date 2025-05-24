create_ip -name ila -vendor xilinx.com -library ip -version 6.2 -module_name ila_3dmi
set_property -dict [list CONFIG.C_NUM_OF_PROBES {27} CONFIG.C_PROBE3_WIDTH {512} CONFIG.C_PROBE7_WIDTH {512} CONFIG.C_PROBE11_WIDTH {512} CONFIG.C_PROBE15_WIDTH {512} CONFIG.C_PROBE18_WIDTH {18} CONFIG.C_PROBE19_WIDTH {32} CONFIG.C_PROBE24_WIDTH {18} CONFIG.C_PROBE25_WIDTH {32} CONFIG.C_EN_STRG_QUAL {1} CONFIG.ALL_PROBE_SAME_MU_CNT {2} ] [get_ips ila_3dmi]

# Data width converters
create_ip -name axis_dwidth_converter -vendor xilinx.com -library ip -version 1.1 -module_name dwidth_input_512_64
set_property -dict [list CONFIG.S_TDATA_NUM_BYTES {64} CONFIG.M_TDATA_NUM_BYTES {8} CONFIG.TID_WIDTH {6} CONFIG.HAS_TLAST {1} CONFIG.HAS_TKEEP {1} CONFIG.Component_Name {dwidth_input_512_64}] [get_ips dwidth_input_512_64]

create_ip -name axis_dwidth_converter -vendor xilinx.com -library ip -version 1.1 -module_name dwidth_output_32_512
set_property -dict [list CONFIG.S_TDATA_NUM_BYTES {4} CONFIG.M_TDATA_NUM_BYTES {64} CONFIG.TID_WIDTH {6} CONFIG.HAS_TLAST {1} CONFIG.HAS_TKEEP {1} CONFIG.Component_Name {dwidth_output_32_512}] [get_ips dwidth_output_32_512]