#!/bin/sh
lli=${LLVMINTERP-lli}
exec $lli \
    /home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx_2018.3/getting_started/host/smith_waterman_3/Emulation-SW/binary_container_1/compute_matrices/compute_matrices/compute_matrices/solution/.autopilot/db/a.g.bc ${1+"$@"}
