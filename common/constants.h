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

/***************************************************************
*
* Configuration header file for Versal 3D Image Registration
*
****************************************************************/
#ifndef CONSTANTS_H
#define CONSTANTS_H

//typedef float data_t;
#define DATA_T_BITWIDTH 32

#define DIMENSION 512

#define HIST_PE 8
#define HIST_PE_EXPO 3
#define UNPACK_DATA_BITWIDTH 8
#define INPUT_DATA_BITWIDTH (HIST_PE*UNPACK_DATA_BITWIDTH)
// #define INPUT_DATA_BITWIDTH_INTERP 8 // TODO remove (use INPUT_DATA_BITWIDTH_FETCHER instead)
#define INPUT_DATA_BITWIDTH_FETCHER 1024
#define INPUT_DATA_BITWIDTH_FETCHER_MIN 256
#define NUM_PIXELS_PER_READ 128
#define NUM_PIXELS_PER_READ_EXPO 7
#define NUM_INPUT_DATA (DIMENSION*DIMENSION/(HIST_PE))
#define N_COUPLES_MAX 512
#define J_HISTO_ROWS 256
#define J_HISTO_COLS J_HISTO_ROWS
#define HISTO_ROWS J_HISTO_ROWS
#define INTERVAL_NUMBER 256 // L, amount of levels we want for the binning process, thus at the output

#define SIZE_ROWS 512 // how many rows per aie tile
#define SIZE_COLS 512 // how many columns per aie tile

#define INIT_COLS {-256, -255, -254, -253, -252, -251, -250, -249, -248, -247, -246, -245, -244, -243, -242, -241, -240, -239, -238, -237, -236, -235, -234, -233, -232, -231, -230, -229, -228, -227, -226, -225} // the initial columns for the aie tiles

#define ENTROPY_PE 1
#define INT_PE 32
#define INT_PE_EXPO 5
#define DIV_EXPO 3

#define AIE_PATTERNS {{1ULL, 0ULL}, {65537ULL, 0ULL}, {2098177ULL, 0ULL}, {16843009ULL, 0ULL}, {34082881ULL, 0ULL}, {69272609ULL, 0ULL}, {138682897ULL, 0ULL}, {286331153ULL, 0ULL}, {287458441ULL, 0ULL}, {306778697ULL, 0ULL}, {613566757ULL, 0ULL}, {623191333ULL, 0ULL}, {692736661ULL, 0ULL}, {710224469ULL, 0ULL}, {715806037ULL, 0ULL}, {1431655765ULL, 0ULL}, {1431677611ULL, 0ULL}, {1437291947ULL, 0ULL}, {1454746987ULL, 0ULL}, {1532713819ULL, 0ULL}, {1533916891ULL, 0ULL}, {1840737719ULL, 0ULL}, {1860025207ULL, 0ULL}, {2004318071ULL, 0ULL}, {2008800751ULL, 0ULL}, {2078243807ULL, 0ULL}, {2113400767ULL, 0ULL}, {2139062143ULL, 0ULL}, {2145385471ULL, 0ULL}, {2147450879ULL, 0ULL}, {2147483647ULL, 0ULL}, {4294967295ULL, 0ULL}}
#define AIE_PATTERN_OFFSETS {{31, 1}, {15, 2}, {21, 1}, {7, 4}, {19, 1}, {5, 2}, {9, 1}, {3, 8}, {7, 1}, {3, 2}, {29, 1}, {5, 4}, {27, 1}, {9, 2}, {17, 1}, {1, 16}, {15, 1}, {7, 2}, {5, 1}, {3, 4}, {3, 1}, {13, 2}, {25, 1}, {1, 8}, {23, 1}, {11, 2}, {13, 1}, {1, 4}, {11, 1}, {1, 2}, {1, 1}, {0, 32}}

#define COORD_BITWIDTH 32

#define AIE_TOP_LEFT 0
#define AIE_TOP_RIGHT 1
#define AIE_BOTTOM_LEFT 2
#define AIE_BOTTOM_RIGHT 3

#endif
