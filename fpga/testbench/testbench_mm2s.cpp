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

#include <unistd.h>
#include <sys/stat.h>
#include <fstream>
#include <ap_axi_sdata.h>
#include <cmath>
#include "../mm2s.cpp"
#include <iostream>

void read_from_stream(float *buffer, hls::stream<float> &stream, size_t size) {
    for (unsigned int i = 0; i < size; i++) {
        buffer[i] = stream.read();
    }
}

int main(int argc, char* argv[]) {

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

    ap_uint<64>* input_flt = new ap_uint<64>[DIMENSION*DIMENSION * (n_couples + padding)/8];
    ap_uint<64>* input_ref = new ap_uint<64>[DIMENSION*DIMENSION * (n_couples + padding)/8];
    for (int i = 0; i < DIMENSION*DIMENSION * (n_couples + padding)/8; i++) {
        input_flt[i] = 0;
        input_ref[i] = 0;
        for (int j = 0; j < 8; j++) {
            input_flt[i].range((j+1)*8-1, j*8) = input_float[i*8+j];
            input_ref[i].range((j+1)*8-1, j*8) = input_reference[i*8+j];
        }
    }

    hls::stream<INPUT_DATA_TYPE> out_flt;
    hls::stream<INPUT_DATA_TYPE> out_ref;
    hls::stream<INPUT_DATA_TYPE> data_info;

    mm2s(n_couples, input_flt, input_ref, out_flt, out_ref, data_info);

    // read out_flt.data up to the end of the stream and check it is equal to input_flt
    INPUT_DATA_TYPE out_flt_data;
    INPUT_DATA_TYPE out_ref_data;
    INPUT_DATA_TYPE data_info_data;
    int i = 0;
    while (out_flt.empty() == 0) {
        out_flt_data = out_flt.read();
        out_ref_data = out_ref.read();
        if (out_flt_data.data != input_flt[i]) {
            std::cout << "Error: out_flt.data != out_ref.data" << std::endl;
            return 1;
        }
        if (out_ref_data.data != input_ref[i]) {
            std::cout << "Error: out_flt.data != out_ref.data" << std::endl;
            return 1;
        }
        i++;
    }
    int j = 0;
    while (data_info.empty() == 0) {
        data_info_data = data_info.read();
        if (data_info_data.data != n_couples) {
            std::cout << "Error: data_info.data != n_couples" << std::endl;
            return 1;
        }
        j++;
    }
    std::cout << "Value of i is " << i << std::endl;
    std::cout << "Value of j is " << j << std::endl;

    return 0;
}