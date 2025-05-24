# VOTED: Versal-Optimization-Toolkit-for-Education-ed-Heterogeneous-System-Development - FPGA101 Course

## Info & Description
This repository contains the main infrastructure for to build the 3DMI Coyote app.

## Main Structure

The coyote_app folder contains the hw and sw to build a Coyote project.

**mutual_info** - contains the hardware for your application. 
**sw** - contains the software for your application.  
**linking** - contains the linking info

**Main Commands**

_make aie_compile_x86_ : compile your code for x86 architecture.  
_make aie_simulate_x86_ : simulate your x86 architecture.  
_make aie_compile SHELL_NAME=< qdma|xdma >_ : compile your code for VLIW architecture, as your final hardware for HW ad HW_EMU. 
_make aie_simulate_ : simulate your code for VLIW architecture, as your final hardware.  
_make clean_ : removes all the output file created by the commands listed above.  


**Main Commands**
_make coyote_hw_ : builds the hw bitstream for Coyote

_make coyote_sw_ : build the software hostcode for Coyote

_make build_sw_ : it compiles the sw

_./setup_emu.sh -s on --shell =< qdma|xdma >_ : enables the hardware emulation

i.e.: make build_sw && ./setup_emu.sh && ./host_overlay.exe : this will compile, prepare the emulation, and run it.


## General useful commands:
If you need to move your bitstream and executable on the target machine, you may want it prepared in a single folder that contains all the required stuff to be moved. In this case, you can use the

_make build_and_pack TARGET=hw/hw_emu SHELL_NAME=< qdma|xdma >_ :  it allows you to pack our build in a single folder. Notice that the hw_emu does not have to be moved on the device, it must be executed on the development machine.

## OpenCV setup
To install OpenCV on your machine, use the following commands:
```
mkdir ~/opencv_build && cd ~/opencv_build
git clone https://github.com/opencv/opencv.git
git clone https://github.com/opencv/opencv_contrib.git
mkdir -p ~/opencv_build/opencv/build && cd ~/opencv_build/opencv/build

cmake -D CMAKE_BUILD_TYPE=Release -D CMAKE_INSTALL_PREFIX=/usr/local -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON -D OPENCV_GENERATE_PKGCONFIG=ON -D CMAKE_INSTALL_PREFIX=$HOME/local -D BUILD_EXAMPLES=ON -D OPENCV_EXTRA_MODULES_PATH=~/opencv_build/opencv_contrib/modules ..

make -j
make install
```

To add your OpenCV to the PATH through the .bashrc file, modify the .bashrc file as follows:
```
export PKG_CONFIG_PATH=$HOME/local/lib/pkgconfig:$PKG_CONFIG_PATH
export LD_LIBRARY_PATH=$HOME/local/lib:$LD_LIBRARY_PATH
```