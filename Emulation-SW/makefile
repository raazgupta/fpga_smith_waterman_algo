#
# this file was created by a computer. trust it.
#

# compiler tools
XILINX_VIVADO_HLS ?= $(XILINX_SDX)/Vivado_HLS

SDX_CXX ?= $(XILINX_SDX)/bin/xcpp
XOCC ?= $(XILINX_SDX)/bin/xocc
EMCONFIGUTIL = $(XILINX_SDX)/bin/emconfigutil --od .
RM = rm -f
RMDIR = rm -rf

SDX_PLATFORM = xilinx_aws-vu9p-f1-04261818_dynamic_5_0

# host compiler global settings
CXXFLAGS += -DSDX_PLATFORM=$(SDX_PLATFORM) -D__USE_XOPEN2K8 -I/opt/xilinx/xrt/include/ -I/opt/Xilinx/Vivado/2018.3.op2405991/include/ -O0 -g -Wall -c -fmessage-length=0 -std=c++14
LDFLAGS += -lxilinxopencl -lpthread -lrt -lstdc++ -lmpfr -lgmp -lhlsmc++-GCC46 -lIp_floating_point_v7_0_bitacc_cmodel -lIp_xfft_v9_1_bitacc_cmodel -lIp_fir_compiler_v7_2_bitacc_cmodel -lIp_dds_compiler_v6_0_bitacc_cmodel -L/opt/xilinx/xrt/lib/ -L$(XILINX_VIVADO)/lnx64/tools/fpo_v7_0 -L$(XILINX_VIVADO)/lnx64/lib/csim -L$(XILINX_VIVADO)/lnx64/tools/dds_v6_0 -L$(XILINX_VIVADO)/lnx64/tools/fir_v7_0 -L$(XILINX_VIVADO)/lnx64/tools/fft_v9_1 -Wl,-rpath,$(XILINX_VIVADO)/lnx64/lib/csim -Wl,-rpath,$(XILINX_VIVADO)/lnx64/tools/fpo_v7_0 -Wl,-rpath,$(XILINX_VIVADO)/lnx64/tools/fft_v9_1 -Wl,-rpath,$(XILINX_VIVADO)/lnx64/tools/fir_v7_0 -Wl,-rpath,$(XILINX_VIVADO)/lnx64/tools/dds_v6_0

# kernel compiler global settings
XOCC_OPTS = -t sw_emu --platform /home/centos/src/project_data/aws-fpga/SDAccel/aws_platform/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm --save-temps  

#
# OpenCL kernel files
#

BINARY_CONTAINERS += binary_container_1.xclbin

BUILD_SUBDIRS += binary_container_1
BINARY_CONTAINER_1_OBJS += binary_container_1/compute_matrices.xo
ALL_KERNEL_OBJS += binary_container_1/compute_matrices.xo

ALL_MESSAGE_FILES = $(subst .xo,.mdb,$(ALL_KERNEL_OBJS)) $(subst .xclbin,.mdb,$(BINARY_CONTAINERS))

#
# host files
#

HOST_OBJECTS += src/host.o
HOST_OBJECTS += src/xcl2.o

HOST_EXE = smith_waterman_3.exe

BUILD_SUBDIRS += src/

EMCONFIG_FILE = emconfig.json

#
# primary build targets
#

.PHONY: all clean
all: $(BINARY_CONTAINERS) $(HOST_EXE) $(EMCONFIG_FILE)

.NOTPARALLEL: clean

clean:
	-$(RM) $(BINARY_CONTAINERS) $(ALL_KERNEL_OBJS) $(ALL_MESSAGE_FILES)  $(EMCONFIG_FILE) $(HOST_EXE) $(HOST_OBJECTS)
	-$(RM) *.xclbin.sh
	-$(RMDIR) $(BUILD_SUBDIRS)
	-$(RMDIR) _xocc*
	-$(RMDIR) .Xil

.PHONY: incremental
incremental: all

include makeemconfig.mk

nothing:

#
# binary container: binary_container_1.xclbin
#

binary_container_1/compute_matrices.xo: ../src/compute_matrices.cpp
	@mkdir -p $(@D)
	-@$(RM) $@
	$(XOCC) $(XOCC_OPTS) -c -k compute_matrices -g --messageDb binary_container_1/compute_matrices.mdb -I"$(<D)" --xp misc:solution_name=compute_matrices --temp_dir binary_container_1 --report_dir binary_container_1/reports --log_dir binary_container_1/logs -o"$@" "$<"

binary_container_1.xclbin: $(BINARY_CONTAINER_1_OBJS)
	-@echo $(XOCC) $(XOCC_OPTS) -l --nk compute_matrices:1:compute_matrices_1 -g --messageDb binary_container_1.mdb --xp misc:solution_name=link --temp_dir binary_container_1 --report_dir binary_container_1/reports --log_dir binary_container_1/logs --remote_ip_cache /home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx_2018.3/getting_started/host/smith_waterman_3/ip_cache -o"$@" $(BINARY_CONTAINER_1_OBJS) > binary_container_1.xclbin.sh
	$(XOCC) $(XOCC_OPTS) -l --nk compute_matrices:1:compute_matrices_1 -g --messageDb binary_container_1.mdb --xp misc:solution_name=link --temp_dir binary_container_1 --report_dir binary_container_1/reports --log_dir binary_container_1/logs --remote_ip_cache /home/centos/src/project_data/aws-fpga/SDAccel/examples/xilinx_2018.3/getting_started/host/smith_waterman_3/ip_cache -o"$@" $(BINARY_CONTAINER_1_OBJS)

#
# host rules
#

src/host.o: ../src/host.cpp ../src/xcl2.hpp
	@mkdir -p $(@D)
	$(SDX_CXX) $(CXXFLAGS) -DSDX_PLATFORM=$(SDX_PLATFORM) -D__USE_XOPEN2K8 -I/opt/xilinx/xrt/include/ -I/opt/Xilinx/Vivado/2018.3.op2405991/include/ -O0 -g -Wall -c -fmessage-length=0 -o "$@" "$<"

src/xcl2.o: ../src/xcl2.cpp ../src/xcl2.hpp
	@mkdir -p $(@D)
	$(SDX_CXX) $(CXXFLAGS) -DSDX_PLATFORM=$(SDX_PLATFORM) -D__USE_XOPEN2K8 -I/opt/xilinx/xrt/include/ -I/opt/Xilinx/Vivado/2018.3.op2405991/include/ -O0 -g -Wall -c -fmessage-length=0 -o "$@" "$<"

$(HOST_EXE): $(HOST_OBJECTS)
	$(SDX_CXX) -o "$@" $(+) $(LDFLAGS) -lxilinxopencl -lpthread -lrt -lstdc++ -lmpfr -lgmp -lhlsmc++-GCC46 -lIp_floating_point_v7_0_bitacc_cmodel -lIp_xfft_v9_1_bitacc_cmodel -lIp_fir_compiler_v7_2_bitacc_cmodel -lIp_dds_compiler_v6_0_bitacc_cmodel -L/opt/xilinx/xrt/lib/ -L$(XILINX_VIVADO)/lnx64/tools/fpo_v7_0 -L$(XILINX_VIVADO)/lnx64/lib/csim -L$(XILINX_VIVADO)/lnx64/tools/dds_v6_0 -L$(XILINX_VIVADO)/lnx64/tools/fir_v7_0 -L$(XILINX_VIVADO)/lnx64/tools/fft_v9_1 -Wl,-rpath,$(XILINX_VIVADO)/lnx64/lib/csim -Wl,-rpath,$(XILINX_VIVADO)/lnx64/tools/fpo_v7_0 -Wl,-rpath,$(XILINX_VIVADO)/lnx64/tools/fft_v9_1 -Wl,-rpath,$(XILINX_VIVADO)/lnx64/tools/fir_v7_0 -Wl,-rpath,$(XILINX_VIVADO)/lnx64/tools/dds_v6_0

#
# emulation configuration file
#

$(EMCONFIG_FILE): makeemconfig.mk
	$(EMCONFIGUTIL) --nd $(NUMBER_OF_DEVICES)  --platform /home/centos/src/project_data/aws-fpga/SDAccel/aws_platform/xilinx_aws-vu9p-f1-04261818_dynamic_5_0/xilinx_aws-vu9p-f1-04261818_dynamic_5_0.xpfm
	-@$(RMDIR) TempConfig .Xil/configutil*

