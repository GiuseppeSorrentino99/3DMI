#include <cmath>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ap_axi_sdata.h>
#include "../mutual_information_master.cpp"
#include "../../common/common.h"
#define DIMENSION 512

double software_mi(int n_couples, uint8_t* input_flt, uint8_t* input_ref) {
	const int padding = 0;

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
    return mutualinfo;
}



int main(int argc, char** argv) {

    int n_couples = 256;
    int padding = 0;
    uint8_t* input_float    = new uint8_t[DIMENSION*DIMENSION * (n_couples + padding)];
    uint8_t* input_reference       = new uint8_t[DIMENSION*DIMENSION * (n_couples + padding)];
    srand(static_cast<unsigned>( time(nullptr) ));

    // generate a random image
    for (int i = 0; i < DIMENSION*DIMENSION * (n_couples + padding); i++) {
        input_float[i] = rand() % 256;
    }
    // generate a random reference image
    for (int i = 0; i < DIMENSION*DIMENSION * (n_couples + padding); i++) {
        input_reference[i] = rand() % 256;
    }

    // create the input streams
    hls::stream<INPUT_DATA_TYPE> input_img("input_img");
    hls::stream<INPUT_DATA_TYPE> input_ref("input_ref");
    hls::stream<data_t> mutual_info("mutual_info");
    hls::stream<INPUT_DATA_TYPE> n_couples_stream("n_couples_stream");

    // write the input data to the streams
    for (int i = 0; i < DIMENSION*DIMENSION * (n_couples + padding); i+=8) {
        INPUT_DATA_TYPE input_data;
        input_data.data.range(7,0) = input_float[i];
        input_data.data.range(15,8) = input_float[i+1];
        input_data.data.range(23,16) = input_float[i+2];
        input_data.data.range(31,24) = input_float[i+3];
        input_data.data.range(39,32) = input_float[i+4];
        input_data.data.range(47,40) = input_float[i+5];
        input_data.data.range(55,48) = input_float[i+6];
        input_data.data.range(63,56) = input_float[i+7];
        
        input_img.write(input_data);
    }
    for (int i = 0; i < DIMENSION*DIMENSION * (n_couples + padding); i+=8) {
        INPUT_DATA_TYPE input_data;
        input_data.data.range(7,0) = input_reference[i];
        input_data.data.range(15,8) = input_reference[i+1];
        input_data.data.range(23,16) = input_reference[i+2];
        input_data.data.range(31,24) = input_reference[i+3];
        input_data.data.range(39,32) = input_reference[i+4];
        input_data.data.range(47,40) = input_reference[i+5];
        input_data.data.range(55,48) = input_reference[i+6];
        input_data.data.range(63,56) = input_reference[i+7];
        input_ref.write(input_data);
    }

    // write the number of couples to the stream
    // leggi da input ref
    INPUT_DATA_TYPE n_couples_data;
    n_couples_data.data = n_couples;
    n_couples_stream.write(n_couples_data);
    mutual_information_master(input_img, input_ref, mutual_info, n_couples_stream);
    data_t output_data = mutual_info.read();
    std::cout << "Output data: " << output_data.data << std::endl;
    
    // call software mi
    double sw_mi = software_mi(n_couples, input_float, input_reference);
    std::cout << "Software Mutual Information: " << sw_mi << std::endl;

    // compare the results
    if (abs(sw_mi - output_data.data) < 0.01) {
        std::cout << "Test passed" << std::endl;
    } else {
        std::cout << "Test failed" << std::endl;
    }
}