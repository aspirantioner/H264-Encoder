#ifndef FRAME
#define FRAME

#include <vector>
#include <iostream>
#include "log.h"
#include "macroblock.h"
#include <cstdlib>
#include <ctime>
#include <numeric>
using namespace std;
enum
{
	MB_NEIGHBOR_UL,
	MB_NEIGHBOR_U,
	MB_NEIGHBOR_UR,
	MB_NEIGHBOR_L
};

enum
{
	P_PICTURE,
	B_PICTURE,
	I_PICTURE,
};

enum BLOCKTYPE
{
	Y_BLOCK,
	Cr_BLOCK,
	Cb_BLOCK,
};
enum P_BLOCK_TYPE
{
	P_L0_16X16,
	P_L0_L0_16X8,
	P_L0_L0_8X16,
	P_L0_L0_8X8,
	P_L0_L0_REF0,
};

template <BLOCKTYPE TYPE>
struct TypeBlockSelector;

template <>
struct TypeBlockSelector<Y_BLOCK>
{
	using type = Block16x16;
};

template <>
struct TypeBlockSelector<Cb_BLOCK>
{
	using type = Block8x8;
};

template <>
struct TypeBlockSelector<Cr_BLOCK>
{
	using type = Block8x8;
};

class RawFrame
{
public:
	int width;
	int height;
	std::vector<int> Y;
	std::vector<int> Cb;
	std::vector<int> Cr;

	RawFrame(const int, const int);
};

class PadFrame
{
public:
	int width;
	int height;
	int raw_width;
	int raw_height;
	std::vector<int> Y;
	std::vector<int> Cb;
	std::vector<int> Cr;

	PadFrame(const int, const int);
	PadFrame(const RawFrame &);
};

class Frame
{
public:
	int type;
	int width;
	int height;
	int nb_mb_rows;
	int nb_mb_cols;
	std::vector<int> Y;
	std::vector<int> Cr;
	std::vector<int> Cb;
	std::vector<int> inter_y;
	std::vector<int> inter_cb;
	std::vector<int> inter_cr;
	std::vector<MacroBlock> mbs;
	Frame(){};
	// Frame(const PadFrame &);
	void operator=(const Frame &frame)
	{
		height = frame.height;
		width = frame.width;
		nb_mb_cols = frame.nb_mb_cols;
		nb_mb_rows = frame.nb_mb_rows;
		type = frame.type;
	}
	template <BLOCKTYPE TYPE>
	typename TypeBlockSelector<TYPE>::type &get_block(int x, int y)
	{
		if constexpr (TYPE == Y_BLOCK)
			return get_y_block(x, y);
		else if constexpr (TYPE == Cb_BLOCK)
			return get_cb_block(x, y);
		else if constexpr (TYPE == Cr_BLOCK)
			return get_cr_block(x, y);
	}
	Block16x16 &get_y_block(int x, int y)
	{
		return mbs[y * nb_mb_cols + x].Y;
	}
	Block8x8 &get_cr_block(int x, int y)
	{
		return mbs[y * nb_mb_cols + x].Cr;
	}
	Block8x8 &get_cb_block(int x, int y)
	{
		return mbs[y * nb_mb_cols + x].Cb;
	}
	int *get_y_pixel(int i, int j)
	{
		return &(Y[i * width + j]);
	}
	void get_6x6block(int i, int j, std::array<int, 36> &arr)
	{
		auto get_a_row = [&](int i, int j, int *row) -> void
		{
			auto iter = get_y_pixel(i, j);
			auto len = width - j;
			if (len >= 6)
			{
				std::copy(iter, iter + 6, row);
			}
			else
			{
				std::copy(iter, iter + len, row);
				std::fill(iter + len, iter + 6, *row);
			}
		};
		get_a_row(i, j, &arr[12]);
		int index = i - 1;
		for (int count = 0; count < 2; count++)
		{
			if (index < 0)
			{
				std::copy(&arr[12 - 6 * count], &arr[18 - 6 * count], &arr[6 - 6 * count]);
			}
			else
			{
				get_a_row(index, j, &arr[6 - 6 * count]);
			}
			index--;
		}
		index = i + 1;
		for (int count = 0; count < 3; count++)
		{
			if (index >= height)
			{
				std::copy(&arr[12 + 6 * count], &arr[18 + 6 * count], &arr[18 + 6 * count]);
			}
			else
			{
				get_a_row(index, j, &arr[18 + 6 * count]);
			}
			index++;
		}
	}
	void get_y_interpolate_pixel(std::array<int, 36> &arr, std::array<int, 16> &block)
	{
		std::array<int, 6> x_sum;
		std::array<int, 6> y_sum;
		auto iter = arr.begin();
		for (int i = 0; i < 6; i++)
		{
			x_sum[i] = std::accumulate(iter, iter + 6, 0) / 6;
			iter += 6;
		}

		for (int i = 0; i < 6; i++)
		{
			iter = arr.begin() + i;
			y_sum[i] = 0;
			for (int j = 0; j < 6; j++)
			{
				y_sum[i] += arr[i + j * 6];
				iter += 6;
			}
			y_sum[i] /= 6;
		}

		block[0] = arr[14];
		auto calculate_half_pixel = [](int *arr, int start) -> int
		{ return arr[start] - 5 * arr[start + 1] + 20 * arr[start + 2] + 20 * arr[start + 3] - 5 * arr[start + 4] + arr[start + 5]+16; };
		block[2] = calculate_half_pixel(&arr[0], 12) >> 5;
		block[8] = (arr[2] - 5 * arr[8] + 20 * arr[14] + 20 * arr[20] - 5 * arr[26] + arr[32]+16) >> 5;
		block[10] = calculate_half_pixel(&x_sum[0], 0) >> 5;
		block[1] = (block[0] + block[2] + 1) >> 1;
		block[3] = (block[2] + arr[15] + 1) >> 1;
		block[9] = (block[8] + block[10] + 1) >> 1;
		block[11] = (block[10] + y_sum[3] + 1) >> 1;
		block[4] = (block[0] + block[8] + 1) >> 1;
		block[6] = (block[2] + block[10] + 1) >> 1;
		block[12] = (block[8] + arr[20] + 1) >> 1;
		block[14] = (block[10] + x_sum[3] + 1) >> 1;
		block[5] = (block[0] + block[10] + 1) >> 1;
		block[7] = (block[10] + arr[15] + 1) >> 1;
		block[13] = (block[10] + arr[20] + 1) >> 1;
		block[15] = (block[10] + arr[21] + 1) >> 1;
		for (auto &elem : block)
		{
			elem = std::min(255, std::max(0, elem));
		}
	}
	void y_pixel_interpolate()
	{
		std::array<int, 36> arr;
		std::array<int, 16> block;
		for (int i = 0; i < height; i++)
		{
			for (int j = 0; j < width; j++)
			{
				get_6x6block(i, j, arr);
				get_y_interpolate_pixel(arr, block);
				auto iter = inter_y.begin() + i * width * 16 + j * 4;
				auto block_iter = block.begin();
				for (int i = 0; i < 16; i += 4)
				{
					std::copy(block_iter, block_iter + 4, iter);
					iter += width * 4;
					block_iter += 4;
				}
			}
		}
	}
	template <BLOCKTYPE TYPE>
	void get_4x4block(int i, int j, std::array<int, 4> &arr)
	{
		auto iter = Cr.begin() + (i * width / 2) + j;
		if constexpr (TYPE == Cb_BLOCK)
		{
			iter = Cb.begin() + (i * width / 2) + j;
		}
		arr[0] = *iter;
		arr[1] = *(iter + 1);
		arr[2] = *(iter + width / 2);
		arr[3] = *(iter + width / 2 + 1);
	}
	void get_cbcr_interpolate_pixel(std::array<int, 4> &arr, std::array<int, 64> &block)
	{
		for (int i = 0; i < 8; i++)
		{
			for (int j = 0; j < 8; j++)
			{
				block[8 * i + j] = ((8 - i) * (8 - j) * arr[0] + j * (8 - i) * arr[1] + i * (8 - j) * arr[2] + i * j * arr[3]) >> 6;
			}
		}
	}
	void cbcr_pixel_interpolate()
	{
		std::array<int, 4> arr;
		std::array<int, 64> block;
		for (int i = 0; i < height / 2; i += 2)
		{
			for (int j = 0; j < width / 2; j += 2)
			{
				get_4x4block<Cb_BLOCK>(i, j, arr);
				get_cbcr_interpolate_pixel(arr, block);
				auto iter = inter_cb.begin() + i * width * 8 + j * 4;
				auto block_iter = block.begin();
				for (int i = 0; i < 64; i += 8)
				{
					std::copy(block_iter, block_iter + 8, iter);
					iter += width * 2;
					block_iter += 8;
				}
				get_4x4block<Cr_BLOCK>(i, j, arr);
				get_cbcr_interpolate_pixel(arr, block);
				iter = inter_cr.begin() + i * width * 8 + j * 4;
				block_iter = block.begin();
				for (int i = 0; i < 64; i += 8)
				{
					std::copy(block_iter, block_iter + 8, iter);
					iter += width * 2;
					block_iter += 8;
				}
			}
		}
	}
	void pixel_interpolate()
	{
		int size = (width * height) << 4;
		inter_y.resize(size);
		inter_cb.resize(size >> 2);
		inter_cr.resize(size >> 2);
		y_pixel_interpolate();
		cbcr_pixel_interpolate();
	}
	template <BLOCKTYPE TYPE>
	std::pair<int, int> transform_mbxy_into_framexy(MacroBlock &mb)
	{
		constexpr int N = (TYPE == Y_BLOCK ? 16 : 8);
		return std::pair<int, int>(mb.mb_col * N * 4, mb.mb_row * N * 4);
	}
	template <BLOCKTYPE TYPE>
	bool is_valid_xy(int x, int y)
	{

		constexpr int N = (TYPE == Y_BLOCK ? 16 : 8);
		constexpr int scale = (TYPE == Y_BLOCK ? 4 : 2);
		return x >= 0 && y >= 0 && x + N <= width * scale && y + N <= height * scale;
	}
	template <BLOCKTYPE TYPE>
	void get_block_from_framexy(std::pair<int, int> frame_xy, typename TypeBlockSelector<TYPE>::type &block)
	{
		constexpr uint8_t N = (TYPE == Y_BLOCK ? 16 : 8);
		auto inter_width = width << 2;
		auto inter_height = height << 2;
		auto x = frame_xy.first;
		auto y = frame_xy.second;
		if constexpr (TYPE != Y_BLOCK)
		{
			inter_width /= 2;
			inter_height /= 2;
		}
		auto iter = inter_y.begin() + x + y * inter_width;
		if constexpr (TYPE == Cr_BLOCK)
		{
			iter = inter_cr.begin() + x + y * inter_width;
		}
		else if constexpr (TYPE == Cb_BLOCK)
		{
			iter = inter_cb.begin() + x + y * inter_width;
		}
		auto block_iter = block.begin();
		for (int i = 0; i < N; i++)
		{
			std::copy(iter, iter + N, block_iter);
			iter += inter_width;
			block_iter += N;
		}
		// auto row_bottom = frame_xy.second % N;
		// auto row_top = N - row_bottom;
		// auto col_right = frame_xy.first % N;
		// auto col_left = N - col_right;
		// auto x = frame_xy.first / N;
		// auto y = frame_xy.second / N;
		// if (row_bottom == 0 && col_right == 0)
		//{
		//	// std::cout << "get while block" << std::endl;
		//	auto target_block = get_block<TYPE>(x, y);
		//	std::copy(target_block.begin(), target_block.end(), block.begin());
		//	return;
		// }

		// auto UL_block = row_top * col_left != 0 ? get_block<TYPE>(x, y) : block;
		// auto BL_block = row_bottom * col_left != 0 ? get_block<TYPE>(x, y + 1) : block;
		// auto UR_block = row_top * col_right != 0 ? get_block<TYPE>(x + 1, y) : block;
		// auto BR_block = row_bottom * col_right != 0 ? get_block<TYPE>(x + 1, y + 1) : block;

		//// std::cout << row_bottom << "\t" << row_top << std::endl;
		// for (int i = row_bottom; i < N; i++)
		//{
		//	// std::cout << "copy " << i - row_bottom << "row" << std::endl;
		//	std::copy(UL_block.begin() + i * N + col_right, UL_block.begin() + (i + 1) * N, block.begin() + (i - row_bottom) * N);
		//	std::copy(UR_block.begin() + i * N, UR_block.begin() + i * N + col_right, block.begin() + (i - row_bottom) * N + col_left);
		// }
		//// std::cout << "top row has copyed!" << std::endl;
		// for (int i = 0; i < row_bottom; i++)
		//{
		//	// std::cout << "copy " << i + row_top << "row" << std::endl;
		//	std::copy(BL_block.begin() + i * N + col_right, BL_block.begin() + (i + 1) * N, block.begin() + (i + row_top) * N);
		//	std::copy(BR_block.begin() + i * N, BR_block.begin() + i * N + col_right, block.begin() + (i + row_top) * N + col_left);
		// }
		//// std::cout << "bottom row has copyed!" << std::endl;
	}
	int get_neighbor_index(const int, const int);
	template <BLOCKTYPE TYPE>
	typename TypeBlockSelector<TYPE>::type test_block(int x, int y)
	{
		typename TypeBlockSelector<TYPE>::type block;
		constexpr int N = (TYPE == Y_BLOCK ? 16 : 8);
		if (TYPE == Y_BLOCK)
		{
			for (int i = 0; i < N; i++)
			{
				std::copy(Y.begin() + (y + i) * width + x, Y.begin() + (y + i) * width + x + N, block.begin() + i * N);
			}
		}
		else if (TYPE == Cb_BLOCK)
		{
			for (int i = 0; i < N; i++)
			{
				std::copy(Cb.begin() + (y + i) * width / 2 + x, Cb.begin() + (y + i) * width / 2 + x + N, block.begin() + i * N);
			}
		}
		else
		{
			for (int i = 0; i < N; i++)
			{
				std::copy(Cr.begin() + (y + i) * width / 2 + x, Cr.begin() + (y + i) * width / 2 + x + N, block.begin() + i * N);
			}
		}
		return block;
	}
	void test()
	{

		auto equal = [](int *arr1, int *arr2, int len) -> bool
		{ for (int i = 0; i < len;i++){
			if(arr1[i]!=arr2[i]){
				return false;
			}
		 } return true; };
		srand((int)time(0));
		for (int i = 0; i < 1000; i++)
		{
			int x = rand() % width;
			int y = rand() % height;
			if (is_valid_xy<Y_BLOCK>(x, y) && is_valid_xy<Cr_BLOCK>(x, y))
			{
				Block16x16 y_block, yy_block;
				Block8x8 cr_block, crcr_block;
				Block8x8 cb_block, cbcb_block;

				get_block_from_framexy<Y_BLOCK>(std::pair<int, int>(x, y), y_block);
				get_block_from_framexy<Cb_BLOCK>(std::pair<int, int>(x, y), cb_block);
				get_block_from_framexy<Cr_BLOCK>(std::pair<int, int>(x, y), cr_block);
				yy_block = test_block<Y_BLOCK>(x, y);
				cbcb_block = test_block<Cb_BLOCK>(x, y);
				crcr_block = test_block<Cr_BLOCK>(x, y);
				if (!equal(y_block.data(), yy_block.data(), y_block.size()) || !equal(cb_block.data(), cb_block.data(), cbcb_block.size()) || !equal(cr_block.data(), crcr_block.data(), cr_block.size()))
				{
					std::cout << i << " time error:" << x << "\t" << y << std::endl;
					return;
				}
			}
		}
	}
};

#endif
