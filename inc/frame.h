#ifndef FRAME
#define FRAME

#include <vector>
#include <iostream>
#include "log.h"
#include "macroblock.h"
#include <cstdlib>
#include <ctime>
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
	std::vector<MacroBlock> mbs;
	Frame(){};
	Frame(const PadFrame &);
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

	template <BLOCKTYPE TYPE>
	std::pair<int, int> transform_mbxy_into_framexy(MacroBlock &mb)
	{
		constexpr int N = (TYPE == Y_BLOCK ? 16 : 8);
		return std::pair<int, int>(mb.mb_col * N, mb.mb_row * N);
	}
	template <BLOCKTYPE TYPE>
	bool is_valid_xy(int x, int y)
	{

		constexpr int N = (TYPE == Y_BLOCK ? 16 : 8);
		constexpr int scale = (TYPE == Y_BLOCK ? 1 : 2);
		return x >= 0 && y >= 0 && x + N <= width / scale && y + N <= height / scale;
	}
	template <BLOCKTYPE TYPE>
	void get_block_from_framexy(std::pair<int, int> frame_xy, typename TypeBlockSelector<TYPE>::type &block)
	{
		constexpr uint8_t N = (TYPE == Y_BLOCK ? 16 : 8);
		auto row_bottom = frame_xy.second % N;
		auto row_top = N - row_bottom;
		auto col_right = frame_xy.first % N;
		auto col_left = N - col_right;
		auto x = frame_xy.first / N;
		auto y = frame_xy.second / N;
		if (row_bottom == 0 && col_right == 0)
		{
			// std::cout << "get while block" << std::endl;
			auto target_block = get_block<TYPE>(x, y);
			std::copy(target_block.begin(), target_block.end(), block.begin());
			return;
		}

		auto UL_block = row_top * col_left != 0 ? get_block<TYPE>(x, y) : block;
		auto BL_block = row_bottom * col_left != 0 ? get_block<TYPE>(x, y + 1) : block;
		auto UR_block = row_top * col_right != 0 ? get_block<TYPE>(x + 1, y) : block;
		auto BR_block = row_bottom * col_right != 0 ? get_block<TYPE>(x + 1, y + 1) : block;

		// std::cout << row_bottom << "\t" << row_top << std::endl;
		for (int i = row_bottom; i < N; i++)
		{
			// std::cout << "copy " << i - row_bottom << "row" << std::endl;
			std::copy(UL_block.begin() + i * N + col_right, UL_block.begin() + (i + 1) * N, block.begin() + (i - row_bottom) * N);
			std::copy(UR_block.begin() + i * N, UR_block.begin() + i * N + col_right, block.begin() + (i - row_bottom) * N + col_left);
		}
		// std::cout << "top row has copyed!" << std::endl;
		for (int i = 0; i < row_bottom; i++)
		{
			// std::cout << "copy " << i + row_top << "row" << std::endl;
			std::copy(BL_block.begin() + i * N + col_right, BL_block.begin() + (i + 1) * N, block.begin() + (i + row_top) * N);
			std::copy(BR_block.begin() + i * N, BR_block.begin() + i * N + col_right, block.begin() + (i + row_top) * N + col_left);
		}
		// std::cout << "bottom row has copyed!" << std::endl;
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
