#ifndef VLC
#define VLC

#include <cmath>
#include <bitset>
#include <string>
#include <cstdint>

#include "block.h"
#include "bitstream.h"

const int intra_me[] = {
	3, 29, 30, 17, 31,
	18, 37, 8, 32, 38,
	19, 9, 20, 10, 11,
	2, 16, 33, 34, 21,
	35, 22, 39, 4, 36,
	40, 23, 5, 24, 6,
	7, 1, 41, 42, 43,
	25, 44, 26, 46, 12,
	45, 47, 27, 13, 28,
	14, 15, 0};

const int inter_me[] = {
	0, 2, 3, 7, 4, 8, 17, 13, 5, 18,
	9, 14, 10, 15, 16, 11, 1, 32, 33, 36, 34,
	37, 44, 40, 35, 45, 38, 41, 39, 42, 43,
	19, 6, 24, 25, 20, 26, 21, 46, 28, 27, 47,
	22, 29, 23, 30, 31, 12};

Bitstream ue(const unsigned int);
Bitstream se(const int);

std::pair<Bitstream, int> cavlc_block2x2(Block2x2, const int, const int);
std::pair<Bitstream, int> cavlc_block4x4(Block4x4, const int, const int);

#endif
