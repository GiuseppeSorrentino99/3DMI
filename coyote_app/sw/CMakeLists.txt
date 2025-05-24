# CMake configuration
cmake_minimum_required(VERSION 3.5)
project(3DMI_Coyote)

set(CYT_DIR ${CMAKE_SOURCE_DIR}/../../../)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CYT_DIR}/cmake)
find_package(CoyoteSW REQUIRED)
find_package(OpenCV REQUIRED PATHS "/scratch/mvenere/local")
find_package(VitisHLS REQUIRED)

# Try to find XRT via CMake config package
find_package(XRT REQUIRED PATHS /opt/xilinx/xrt/share/cmake NO_DEFAULT_PATH)
message("*** Coyote APP for 3D Image Registration ***")


include_directories( ${OpenCV_INCLUDE_DIRS} )


# Directory containing the executable(s) to be compiled
set(TARGET_DIR "${CMAKE_SOURCE_DIR}/src/")

# Create build targets and link against required libraries
set(EXEC test)
add_executable(${EXEC} ${TARGET_DIR}/main.cpp)

# Set this to your actual XRT installation if not using env var
if(NOT DEFINED ENV{XILINX_XRT})
    message(FATAL_ERROR "XILINX_XRT environment variable not set")
endif()

set(XRT_ROOT $ENV{XILINX_XRT})
include_directories(${XRT_ROOT}/include)
include_directories(${VITIS_HLS_INCLUDE_DIRS})
link_directories(${XRT_ROOT}/lib)

target_link_libraries(${EXEC} PUBLIC Coyote)
target_link_libraries(${EXEC} PUBLIC ${OpenCV_LIBS} )
target_link_directories(${EXEC} PUBLIC /usr/local/lib)

