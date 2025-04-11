# MIT License

# Copyright (c) 2023 Paolo Salvatore Galfano, Giuseppe Sorrentino

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

ECHO=@echo

.PHONY: help

help::
	$(ECHO) ""
	$(ECHO) "Makefile Usage:"
	$(ECHO) "  make all TARGET=<hw/hw_emu>"
	$(ECHO) ""
	$(ECHO) "  make clean"
	$(ECHO) "      Command to remove all the generated files."
	$(ECHO) ""

PLATFORM ?= xilinx_u55c_gen3x16_xdma_3_202210_1
#xilinx_vck5000_gen4x8_qdma_2_202220_1

TARGET := hw

# Use --optimize 3 to enable post-route optimizations. This may improve the bitstream but SIGNIFICANTLY increase compilation time
XOCCLFLAGS := --kernel_frequency 200 --platform $(PLATFORM) -t $(TARGET)  -s -g

XOS     := ../mutual_info/mutual_information_master_$(TARGET).xo 
XCLBIN  := overlay_$(TARGET).xclbin

XSA_OBJ := overlay_$(TARGET).xsa

.phony: clean

all: $(XCLBIN)

$(XCLBIN): $(XSA_OBJ)
	v++ -p -t $(TARGET) -f $(PLATFORM) $^ -o ${XCLBIN} --package.boot_mode=ospi

$(XSA_OBJ): $(XOS) 
	v++ -l $(XOCCFLAGS) $(XOCCLFLAGS) --config xclbin_overlay.cfg -o ${XCLBIN}  $^

clean:
	$(RM) -r _x .Xil .ipcache *.ltx *.log *.sh *.jou *.info *.xclbin *.xo.* *.str *.xsa *.cdo.bin *bif *BIN *.package_summary *.link_summary *.txt *.bin && rm -rf cfg emulation_data sim
	
