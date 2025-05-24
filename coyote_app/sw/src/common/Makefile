
################## software build for XRT Native API code
CXXFLAGS := -std=c++17 -Wno-deprecated-declarations
CXXFLAGS += -I$(XILINX_XRT)/include -I$(XILINX_HLS)/include

LDFLAGS := -L$(XILINX_XRT)/lib
LDFLAGS += $(LDFLAGS) -lxrt_coreutil

LIBS = `pkg-config --libs opencv`
CFLAGS = `pkg-config --cflags opencv`

EXECUTABLE := host_overlay.exe
HOST_SRCS := ./source/sw/host_code.cpp
HOST_SRCS += ./source/sw/include/image_utils/image_utils.cpp
HOST_SRCS += ./source/sw/include/software_mi/software_mi.cpp

build: $(EXECUTABLE)

$(EXECUTABLE): $(HOST_SRCS)
	$(CXX) -o $(EXECUTABLE) $^ $(CXXFLAGS) $(CFLAGS) $(LIBS)

run:
	./$(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)
	rm -f dataset_sw_output/*.png
	rm -f dataset_output/*.png

