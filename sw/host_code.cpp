/*
MIT License

Copyright (c) 2023 Paolo Salvatore Galfano, Giuseppe Sorrentino

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include "opencv2/opencv.hpp"
#include "experimental/xrt_kernel.h"
#include "experimental/xrt_uuid.h"
#include "../common/common.h"
#include "../mutual_info/include/hw/mutualInfo/entropy.h"
#include "../mutual_info/include/hw/mutualInfo/histogram.h"

// For hw emulation, run in sw directory: source ./setup_emu.sh -s on

#define DEVICE_ID 0

// every top function input that must be passed from the host to the kernel must have a unique index starting from 0

// args indexes for setup_aie kernel

#define arg_mi_img_flt 0
#define arg_mi_img_ref 1
#define arg_mi_out 2
#define arg_mi_n_couples 3
#define arg_mi_padding 4

bool get_xclbin_path(std::string& xclbin_file);
std::ostream& bold_on(std::ostream& os);
std::ostream& bold_off(std::ostream& os);

void write_slice_in_buffer(uint8_t *src, uint8_t *dest, const int slice_index, const int SIZE, const int LAYERS) {
    for (int i = 0; i < SIZE*SIZE; i++) {
        #ifndef USE_OLD_FORMAT
        const int dest_index = i * LAYERS + slice_index; // new formula
        #else
        const int dest_index = slice_index * SIZE * SIZE + i; // old formula
        #endif
        dest[dest_index] = src[i];
    }
}


int read_volume_from_file_PNG(uint8_t *volume, const int SIZE, const int N_COUPLES, const int BORDER_PADDING, const int DEPTH_PADDING, const std::string &path) {
    for (int i = 0; i < N_COUPLES; i++) {
        std::string s = path + "IM" + std::to_string(i+1) + ".png";
        cv::Mat image = cv::imread(s, cv::IMREAD_GRAYSCALE);
        if (!image.data) return -1;

        // add border-padding of 1px around the image
        cv::copyMakeBorder(image, image, BORDER_PADDING, BORDER_PADDING, BORDER_PADDING, BORDER_PADDING, cv::BORDER_CONSTANT, 0);

        // copy the slice into the buffer
        std::vector<uint8_t> tmp((SIZE+2*BORDER_PADDING)*(SIZE+2*BORDER_PADDING));
        tmp.assign(image.begin<uint8_t>(), image.end<uint8_t>());
        write_slice_in_buffer(tmp.data(), volume, i, SIZE+2*BORDER_PADDING, N_COUPLES+DEPTH_PADDING);
    }

    for (int i = 0; i < DEPTH_PADDING; i++) {
        // copy the slice into the buffer
        std::vector<uint8_t> tmp((SIZE+2*BORDER_PADDING)*(SIZE+2*BORDER_PADDING));
        tmp.assign(tmp.size(), 0);
        write_slice_in_buffer(tmp.data(), volume, N_COUPLES+i, SIZE+2*BORDER_PADDING, N_COUPLES+DEPTH_PADDING);
    }

    return 0;
}

double software_mi(int n_couples, const std::string &flt_path, const std::string &ref_path) {
    const int padding = (HIST_PE - (n_couples % HIST_PE)) % HIST_PE;
    uint8_t* input_ref = new uint8_t[DIMENSION*DIMENSION * (n_couples + padding)];
    uint8_t* input_flt = new uint8_t[DIMENSION*DIMENSION * (n_couples + padding)];

    if (read_volume_from_file_PNG(input_ref, DIMENSION, n_couples, 0, padding, ref_path) == -1) {
        std::cerr << "Could not open file" << std::endl;
        return 1;
    }
    if (read_volume_from_file_PNG(input_flt, DIMENSION, n_couples, 0, padding, flt_path) == -1) {
        std::cerr << "Could not open file" << std::endl;
        return 1;
    }


    // ----------------------------------------------------CALCOLO SOFTWARE DELLA MI-----------------------------------------

    double j_h[J_HISTO_ROWS][J_HISTO_COLS];
    for(int i=0;i<J_HISTO_ROWS;i++){
        for(int j=0;j<J_HISTO_COLS;j++){
            j_h[i][j]=0.0;
        }
    }

    const int N_COUPLES_TOTAL = n_couples + padding;

    for(int k = 0; k < N_COUPLES_TOTAL-padding; k++) {
        for(int i=0;i<DIMENSION;i++){
            for(int j=0;j<DIMENSION;j++){
                unsigned int a=input_ref[i * DIMENSION * (N_COUPLES_TOTAL) + j * (N_COUPLES_TOTAL) + k];
                unsigned int b=input_flt[i * DIMENSION * (N_COUPLES_TOTAL) + j * (N_COUPLES_TOTAL) + k];
                j_h[a][b]= (j_h[a][b])+1;
            }
        }
    }

    for (int i=0; i<J_HISTO_ROWS; i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
            j_h[i][j] = j_h[i][j]/((N_COUPLES_TOTAL-padding)*DIMENSION*DIMENSION);
        }
    }


    float entropy = 0.0;
    for (int i=0; i<J_HISTO_ROWS; i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
            float v = j_h[j][i];
            if (v > 0.000000000000001) {
            entropy += v*log2(v);
            }
        }
    }
    entropy *= -1;

    double href[ANOTHER_DIMENSION];
    for(int i=0;i<ANOTHER_DIMENSION;i++){
        href[i]=0.0;
    }

    for (int i=0; i<ANOTHER_DIMENSION; i++) {
        for (int j=0; j<ANOTHER_DIMENSION; j++) {
            href[i] += j_h[i][j];
        }
    }

    double hflt[ANOTHER_DIMENSION];
    for(int i=0;i<ANOTHER_DIMENSION;i++){
        hflt[i]=0.0;
    }

    for (int i=0; i<J_HISTO_ROWS; i++) {
        for (int j=0; j<J_HISTO_COLS; j++) {
            hflt[i] += j_h[j][i];
        }
    }


    double eref = 0.0;
    for (int i=0; i<ANOTHER_DIMENSION; i++) {
        if (href[i] > 0.000000000001) {
            eref += href[i] * log2(href[i]);
        }
    }
    eref *= -1;


    double eflt = 0.0;
    for (int i=0; i<ANOTHER_DIMENSION; i++) {
        if (hflt[i] > 0.000000000001) {
            eflt += hflt[i] * log2(hflt[i]);
        }
    }
    eflt =  eflt * (-1);

    double mutualinfo = eref + eflt - entropy;

    delete[] input_flt;
    delete[] input_ref;
    return mutualinfo;
}


int main(int argc, char *argv[]) {

    int n_couples = 512;
    int padding = (NUM_PIXELS_PER_READ - (n_couples % NUM_PIXELS_PER_READ)) % NUM_PIXELS_PER_READ;

    if (argc >= 2) {
        n_couples = atoi(argv[1]);
        if (n_couples > N_COUPLES_MAX)
            n_couples = N_COUPLES_MAX;
    }    int buffer_size = DIMENSION*DIMENSION * (n_couples+padding);

    std::string path_ref = "dataset/ref/";
    std::string path_flt = "dataset/flt/";


    uint8_t* input_flt  = new uint8_t[DIMENSION*DIMENSION * (n_couples+padding)];
    uint8_t* input_ref = new uint8_t[DIMENSION*DIMENSION * (n_couples+padding)];

    if (read_volume_from_file_PNG(input_flt, DIMENSION, n_couples, 1, padding, path_flt) != 0) {
        std::cerr << "Error reading flt volume" << std::endl;
        return EXIT_FAILURE;
    }
    if (read_volume_from_file_PNG(input_ref, DIMENSION, n_couples, 1, padding, path_ref) != 0) {
        std::cerr << "Error reading ref volume" << std::endl;
        return EXIT_FAILURE;
    }


//------------------------------------------------LOADING XCLBIN------------------------------------------    
    std::string xclbin_file;
    if (!get_xclbin_path(xclbin_file))
        return EXIT_FAILURE;

    // Load xclbin
    std::cout << "1. Loading bitstream (" << xclbin_file << ")... ";
    xrt::device device = xrt::device(DEVICE_ID);
    xrt::uuid xclbin_uuid = device.load_xclbin(xclbin_file);
    std::cout << "Done" << std::endl;
//----------------------------------------------INITIALIZING THE BOARD------------------------------------------

    // create kernel objects
    xrt::kernel krnl_mutual_info  = xrt::kernel(device, xclbin_uuid, "mutual_information_master");

    // get memory bank groups for device buffer - required for axi master input/ouput
    xrtMemoryGroup bank_flt  = krnl_mutual_info.group_id(arg_mi_img_flt);
    xrtMemoryGroup bank_ref  = krnl_mutual_info.group_id(arg_mi_img_ref);
    xrtMemoryGroup bank_mutual_info_output  = krnl_mutual_info.group_id(arg_mi_out);

    // create device buffers - if you have to load some data, here they are
    xrt::bo buffer_flt = xrt::bo(device, buffer_size * sizeof(int32_t), xrt::bo::flags::normal, bank_flt); 
    xrt::bo buffer_ref = xrt::bo(device, buffer_size * sizeof(int32_t), xrt::bo::flags::normal, bank_ref); 
    xrt::bo buffer_output = xrt::bo(device, sizeof(float), xrt::bo::flags::normal, bank_mutual_info_output);

    // create runner instances
    xrt::run run_mutual_info  = xrt::run(krnl_mutual_info);

    // set setup_aie kernel arguments
    run_mutual_info.set_arg(arg_mi_img_flt, buffer_flt);
    run_mutual_info.set_arg(arg_mi_img_ref, buffer_ref);
    run_mutual_info.set_arg(arg_mi_out, buffer_output);
    run_mutual_info.set_arg(arg_mi_n_couples, n_couples+padding);
    run_mutual_info.set_arg(arg_mi_padding, padding);

    // write data into the input buffer
    buffer_flt.write(input_flt);
    buffer_flt.sync(XCL_BO_SYNC_BO_TO_DEVICE);
    buffer_ref.write(input_ref);
    buffer_ref.sync(XCL_BO_SYNC_BO_TO_DEVICE);

    // run the kernel
    run_mutual_info.start();
    run_mutual_info.wait();


    // read the output buffer
    buffer_output.sync(XCL_BO_SYNC_BO_FROM_DEVICE);
    float mutual_info;
    buffer_output.read(&mutual_info);

    std::cout << "Mutual Information: " << mutual_info << std::endl;
    float sw_mi = software_mi(n_couples, path_flt, path_ref);
    std::cout << "Software Mutual Information: " << sw_mi << std::endl;
    // ---------------------------------Error Check--------------------------------------
        
    if (abs(sw_mi - mutual_info) < 0.01) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;
    }

    return 0;
    }


bool get_xclbin_path(std::string& xclbin_file) {
    // Judge emulation mode accoring to env variable
    char *env_emu;
    if (env_emu = getenv("XCL_EMULATION_MODE")) {
        std::string mode(env_emu);
        if (mode == "hw_emu")
        {
            std::cout << "Program running in hardware emulation mode" << std::endl;
            xclbin_file = "overlay_hw_emu.xclbin";
        }
        else
        {
            std::cout << "[ERROR] Unsupported Emulation Mode: " << mode << std::endl;
            return false;
        }
    }
    else {
        std::cout << bold_on << "Program running in hardware mode" << bold_off << std::endl;
        xclbin_file = "overlay_hw.xclbin";
    }

    std::cout << std::endl << std::endl;
    return true;
}

std::ostream& bold_on(std::ostream& os)
{
    return os << "\e[1m";
}

std::ostream& bold_off(std::ostream& os)
{
    return os << "\e[0m";
}