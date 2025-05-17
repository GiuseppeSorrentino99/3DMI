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


#include <ap_int.h>
#include <hls_stream.h>
#include <hls_math.h>
#include <ap_axi_sdata.h>
#include "../common/common.h"
#include "../mutual_info/include/hw/mutualInfo/mutual_info.hpp"

extern "C" {

void mm2s(int32_t n_couples, ap_uint<64>* input_flt,ap_uint<64>* input_ref, hls::stream<INPUT_DATA_TYPE>& out_flt, hls::stream<INPUT_DATA_TYPE>& out_ref , hls::stream<INPUT_DATA_TYPE>& data_info) {

	#pragma HLS interface m_axi port=input_flt depth=100 offset=slave bundle=gmem0
	#pragma HLS interface m_axi port=input_ref depth=100 offset=slave bundle=gmem1
	#pragma HLS interface s_axilite port=input_flt bundle=control
	#pragma HLS interface s_axilite port=input_ref bundle=control

	#pragma HLS interface axis port=out_flt
	#pragma HLS interface axis port=out_ref
	#pragma HLS interface axis port=data_info

	#pragma HLS interface s_axilite port=n_couples bundle=control
	#pragma HLS interface s_axilite port=return bundle=control

	// 1. Scrivere n_couples sullo stream
	INPUT_DATA_TYPE n_couples_data;
	n_couples_data.data = (uint64_t)n_couples;
	n_couples_data.last = 1;
	data_info.write(n_couples_data);

	// 2. Scrivere i dati di input sullo stream
	for (int i = 0; i < n_couples * DIMENSION * DIMENSION / 8; i++) {
		#pragma HLS PIPELINE II=1

		ap_uint<64> input_flt_data = input_flt[i];
		ap_uint<64> input_ref_data = input_ref[i];

		INPUT_DATA_TYPE flt_data;
		flt_data.data = input_flt_data;
		flt_data.last = (i == n_couples * DIMENSION * DIMENSION / 8 - 1) ? 1 : 0;
		out_flt.write(flt_data);

		INPUT_DATA_TYPE ref_data;
		ref_data.data = input_ref_data;
		ref_data.last = (i == n_couples * DIMENSION * DIMENSION / 8 - 1) ? 1 : 0;
		out_ref.write(ref_data);
	}


}
}