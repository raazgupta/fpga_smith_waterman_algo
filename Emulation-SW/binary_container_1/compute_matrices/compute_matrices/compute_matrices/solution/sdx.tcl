# ==============================================================
# File generated on Sun Sep 15 21:49:58 UTC 2019
# Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC v2018.3.op (64-bit)
# SW Build 2405991 on Thu Dec  6 23:36:41 MST 2018
# IP Build 2404404 on Fri Dec  7 01:43:56 MST 2018
# Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
# ==============================================================
add_files /home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx_2018.3/getting_started/host/smith_waterman_3/src/compute_matrices.cpp -cflags {-g -I/home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx_2018.3/getting_started/host/smith_waterman_3/src}
set_part xcvu9p-flgb2104-2-i
create_clock -name default -period 250MHz
set_clock_uncertainty 27.000000% default
config_sdx -optimization_level=0
config_sdx -profile=true
config_sdx -target=xocc
config_export -vivado_phys_opt=none
config_export -format=ip_catalog
config_export -ipname=compute_matrices
config_bind -effort=medium
config_schedule -effort=medium
config_schedule -enable_dsp_full_reg=1
config_rtl -stall_sig_gen=1
config_rtl -profile=1
config_rtl -register_reset_num=3
config_rtl -auto_prefix=1
config_rtl -enable_maxiConservative=1
config_compile -pipeline_loops=64
config_compile -name_max_length=256
config_compile -skip_transform=1
config_interface -m_axi_addr64=1
config_dataflow -strict_mode=warning
config_debug -enable=1
