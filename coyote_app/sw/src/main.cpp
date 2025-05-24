#include <any>
#include <random>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include "opencv2/opencv.hpp"
#include "common/common.h"
// External library for easier parsing of CLI arguments by the executable
#include <boost/program_options.hpp>

// Coyote-specific includes
#include "cThread.hpp"

// Default vFPGA to assign cThreads to
#define DEFAULT_VFPGA_ID 0

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
        std::string s = path + "IM" + std::to_string(i) + ".png";
        cv::Mat image = cv::imread(s, cv::IMREAD_GRAYSCALE);
        if (!image.data) 
        {
            std::cerr << "Could not open file" << std::endl;
            return -1;
        }

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

double software_mi(int n_couples, uint8_t * input_ref, uint8_t * input_flt) {
    //const int padding = (HIST_PE - (n_couples % HIST_PE)) % HIST_PE;
    /*
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
    */

    // ----------------------------------------------------CALCOLO SOFTWARE DELLA MI-----------------------------------------

    double j_h[J_HISTO_ROWS][J_HISTO_COLS];
    for(int i=0;i<J_HISTO_ROWS;i++){
        for(int j=0;j<J_HISTO_COLS;j++){
            j_h[i][j]=0.0;
        }
    }

    unsigned int padding = 0;
    //const int N_COUPLES_TOTAL = n_couples + padding;
    const int N_COUPLES_TOTAL = n_couples;

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

    //delete[] input_flt;
    //delete[] input_ref;
    return mutualinfo;
}

int main(int argc, char *argv[]) {
    // CLI arguments
    uint n_couples;
    std::string path_ref,path_flt;
    boost::program_options::options_description runtime_options("Coyote 3D Image Registration Program Options");
    runtime_options.add_options()("n_couples,nc", boost::program_options::value<uint>(&n_couples)->default_value(1), "Number of couples");
    runtime_options.add_options()("flt,flt", 
    boost::program_options::value<std::string>(&path_flt)->default_value(""), "Path of floating volume");
    runtime_options.add_options()("ref,ref", 
    boost::program_options::value<std::string>(&path_ref)->default_value(""), "Path of reference volume");
    boost::program_options::variables_map command_line_arguments;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, runtime_options), command_line_arguments);
    boost::program_options::notify(command_line_arguments);

    PR_HEADER("Validation: 3D Image Registration");
    std::cout << "Reference path: " << path_ref << std::endl;
    std::cout << "Floating path: " << path_flt << std::endl;
    std::cout << "Number of couples: " << n_couples << std::endl;

    int padding = 0;
    std::cout << "Padding: " << padding << std::endl;
    int buffer_size = DIMENSION*DIMENSION * (n_couples+padding);
    std::cout << "Buffer size: " << buffer_size << std::endl;

    // Create a Coyote thread and allocate memory for the vectors
    std::unique_ptr<coyote::cThread<std::any>> coyote_thread(new coyote::cThread<std::any>(DEFAULT_VFPGA_ID, getpid(), 0));
    uint8_t *flt = (uint8_t *) coyote_thread->getMem({coyote::CoyoteAlloc::HPF, buffer_size * sizeof(uint8_t)});
    uint8_t *ref = (uint8_t *) coyote_thread->getMem({coyote::CoyoteAlloc::HPF, buffer_size * sizeof(uint8_t)});
    float *mutual_info = (float *) coyote_thread->getMem({coyote::CoyoteAlloc::HPF, 16 * sizeof(float)});
    uint64_t *n_couples_mem = (uint64_t *) coyote_thread->getMem({coyote::CoyoteAlloc::HPF, sizeof(uint64_t)});
    if (!flt || !ref || !mutual_info || !n_couples) { throw std::runtime_error("Could not allocate memory for vectors, exiting..."); }

    std::cout << "Buffers allocated" << std::endl;

    n_couples_mem[0] = (uint64_t) n_couples;

    
    std::cout<<"Reading volume FLT"<<std::endl;
    if (read_volume_from_file_PNG(flt, DIMENSION, n_couples, 0, padding, path_flt) != 0) {
        std::cerr << "Error reading flt volume" << std::endl;
        return EXIT_FAILURE;
    }
    std::cout<<"Reading volume REF"<<std::endl;
    if (read_volume_from_file_PNG(ref, DIMENSION, n_couples, 0, padding, path_ref) != 0) {
        std::cerr << "Error reading ref volume" << std::endl;
        return EXIT_FAILURE;
    }
        

    // fill flt and ref with random data from 0 to 255
    /*std::cout << "Filling volume FLT" << std::endl;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint8_t> dis(1, 256);
    for(int i = 0; i < buffer_size; i++) {
        flt[i] = dis(gen);
    }
    std::cout << "Filling volume REF" << std::endl;
    for(int i = 0; i < buffer_size; i++) {
        ref[i] = flt[i];
    }
    
    std::cout << "Reading volume FLT" << std::endl;
    for(int i = 0; i < buffer_size; i++) {
        flt[i] = static_cast<uint8_t>(1);
    }
    std::cout << "Reading volume REF" << std::endl; 
    for(int i = 0; i < buffer_size; i++) {
        ref[i] = static_cast<uint8_t>(2);
    }
    std::cout<<"Volumes read"<<std::endl;*/

    /*
    for(int i =0;i<buffer_size;i++){
        std::cout<<std::hex<<static_cast<int>(flt[i])<<" ";
    }
    std::cout<<std::endl;
    for(int i =0;i<buffer_size;i++){
        std::cout<<std::hex<<static_cast<int>(ref[i])<<" ";
    }
    std::cout<<std::endl;
    */
    // Set scatter-gather flags; note transfer size is always in bytes, so multiply vector dimensionality with sizeof(float)
    // Note, how the vector b has a destination of 1; corresponding to the second AXI Stream (see README for more details)
    coyote::sgEntry sg_flt, sg_ref, sg_mutual_info, sg_n_couples;
    memset(&sg_flt, 0, sizeof(coyote::sgEntry));
    memset(&sg_ref, 0, sizeof(coyote::sgEntry));
    memset(&sg_mutual_info, 0, sizeof(coyote::sgEntry));
    memset(&sg_n_couples, 0, sizeof(coyote::sgEntry));
    sg_flt.local = {.src_addr = flt, .src_len = buffer_size * sizeof(uint8_t), .src_dest = 0};
    sg_ref.local = {.src_addr = ref, .src_len = buffer_size * sizeof(uint8_t), .src_dest = 1};
    sg_mutual_info.local = {.dst_addr = mutual_info, .dst_len = sizeof(float), .dst_dest = 0};
    sg_n_couples.local = {.src_addr = n_couples_mem, .src_len = sizeof(uint64_t), .src_dest = 2};

    // Run kernel and wait until complete
    
    coyote_thread->invoke(coyote::CoyoteOper::LOCAL_READ,  &sg_n_couples);
    std::cout << "buffer 3 written" << std::endl;
    coyote_thread->invoke(coyote::CoyoteOper::LOCAL_READ,  &sg_flt);
    std::cout << "buffer 1 written" << std::endl;
    coyote_thread->invoke(coyote::CoyoteOper::LOCAL_READ,  &sg_ref);
    std::cout << "buffer 2 written" << std::endl;
    coyote_thread->setCSR(static_cast<uint64_t>(0x1), static_cast<uint32_t>(0));
    std::cout << "register written" << std::endl;
    coyote_thread->invoke(coyote::CoyoteOper::LOCAL_WRITE, &sg_mutual_info);
    std::cout << "buffer 4 read" << std::endl;
    std::cout << "status: " << coyote_thread->checkCompleted(coyote::CoyoteOper::LOCAL_WRITE) << std::endl;
    std::cout << "n_couples: " << n_couples_mem[0] << std::endl;
    //while(coyote_thread->getCSR(static_cast<uint32_t>(0)) == 0x1);
    //std::cout << "control register: " << std::hex << coyote_thread->getCSR(static_cast<uint32_t>(0)) << std::endl;
    std::cout << "ctrl_reg: " << std::hex << coyote_thread->getCSR(static_cast<uint32_t>(0)) << std::endl;
    while (coyote_thread->checkCompleted(coyote::CoyoteOper::LOCAL_READ) == 0);
    std::cout << "status: " << coyote_thread->checkCompleted(coyote::CoyoteOper::LOCAL_READ) << std::endl;
    while (coyote_thread->checkCompleted(coyote::CoyoteOper::LOCAL_WRITE) == 0);

    coyote_thread->userUnmap( (void*) flt);
    coyote_thread->userUnmap( (void*) ref);
    coyote_thread->userUnmap( (void*) mutual_info);
    coyote_thread->userUnmap( (void*) n_couples_mem);
    
    // Verify correctness of the results
    std::cout << "Mutual Information: " << mutual_info[0] << std::endl;
    //print 16 values from mutual_info
    std::cout << "Mutual Information: ";
    for (int i = 0; i < 16; i++) {
        std::cout << mutual_info[i] << " ";
    }
    std::cout << std::endl;
    // ---------------------------------Software MI--------------------------------------
    float sw_mi = software_mi(n_couples, flt, ref);
    std::cout << "Software Mutual Information: " << sw_mi << std::endl;
    // ---------------------------------Error Check--------------------------------------
        
    if (abs(sw_mi - mutual_info[0]) < 0.01) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;
    }
    PR_HEADER("Validation passed!");
}
