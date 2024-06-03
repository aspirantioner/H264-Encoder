#include "frame_encode.h"
#include <algorithm>
#define SUPPORT_INTRA4x4 false
#define SEARCH_STEP_LEN 1
Log f_logger("Frame encode");
std::vector<Frame> ref_frame_list(1);
void get_match_Y_block(Frame &ref_frame, int x, int y, Block16x16 &luam_pred, Block8x8 &cr_pred, Block8x8 &cb_pred)
{
}
void get_mv(MacroBlock &mb, Frame &ref_frame)
{
	int min_y_sad = (1 << 16), min_cr_cb_sad = (1 << 14);
	auto &mv = mb.mv;
	Block16x16 luma_res;
	Block8x8 cr_res;
	Block8x8 cb_res;
	int y_error, cr_cb_error;
	int diff = 0;

	Block16x16 luma_pred;
	Block8x8 cr_pred;
	Block8x8 cb_pred;
	auto y_mb_xy = ref_frame.transform_mbxy_into_framexy<Y_BLOCK>(mb);
	auto crcb_mb_xy = ref_frame.transform_mbxy_into_framexy<Cr_BLOCK>(mb);

	ref_frame.get_block_from_framexy<Y_BLOCK>(y_mb_xy, luma_pred);
	ref_frame.get_block_from_framexy<Cb_BLOCK>(crcb_mb_xy, cb_pred);
	ref_frame.get_block_from_framexy<Cr_BLOCK>(crcb_mb_xy, cr_pred);
	for (int i = 0; i < 256; i++)
	{
		diff = mb.Y[i] - luma_pred[i];
		luma_pred[i] = diff;
		y_error += abs(luma_pred[i]);
	}
	for (int i = 0; i < 64; i++)
	{
		diff = mb.Cr[i] - cr_pred[i];
		cr_pred[i] = diff;
		diff = mb.Cb[i] - cb_pred[i];
		cb_pred[i] = diff;
		cr_cb_error += abs(cr_pred[i]) + abs(cb_pred[i]);
	}
	if (y_error < min_y_sad)
	{
		min_y_sad = y_error;
		std::copy(luma_pred.begin(), luma_pred.end(), luma_res.begin());
		std::copy(cb_pred.begin(), cb_pred.end(), cb_res.begin());
		std::copy(cr_pred.begin(), cr_pred.end(), cr_res.begin());
		mv = std::pair<int, int>(0, 0);
	}
	for (int i = 1; i <= SEARCH_STEP_LEN; i++)
	{
		y_mb_xy.second--;
		crcb_mb_xy.second--;
		if (!ref_frame.is_valid_xy<Y_BLOCK>(y_mb_xy.first, y_mb_xy.second) || !ref_frame.is_valid_xy<Cr_BLOCK>(crcb_mb_xy.first, crcb_mb_xy.second))
		{
			break;
		}
		ref_frame.get_block_from_framexy<Y_BLOCK>(y_mb_xy, luma_pred);
		ref_frame.get_block_from_framexy<Cb_BLOCK>(crcb_mb_xy, cb_pred);
		ref_frame.get_block_from_framexy<Cr_BLOCK>(crcb_mb_xy, cr_pred);
		for (int i = 0; i < 256; i++)
		{
			diff = mb.Y[i] - luma_pred[i];
			luma_pred[i] = diff;
			y_error += abs(luma_pred[i]);
		}
		for (int i = 0; i < 64; i++)
		{
			diff = mb.Cr[i] - cr_pred[i];
			cr_pred[i] = diff;
			diff = mb.Cb[i] - cb_pred[i];
			cb_pred[i] = diff;
			cr_cb_error += abs(cr_pred[i]) + abs(cb_pred[i]);
		}
		if (y_error < min_y_sad)
		{
			min_y_sad = y_error;
			std::copy(luma_pred.begin(), luma_pred.end(), luma_res.begin());
			std::copy(cb_pred.begin(), cb_pred.end(), cb_res.begin());
			std::copy(cr_pred.begin(), cr_pred.end(), cr_res.begin());
			mv = std::pair<int, int>(0, i * -1);
		}
	}
	for (int i = 1; i <= SEARCH_STEP_LEN; i++)
	{
		y_mb_xy.second++;
		crcb_mb_xy.second++;
		if (!ref_frame.is_valid_xy<Y_BLOCK>(y_mb_xy.first, y_mb_xy.second) || !ref_frame.is_valid_xy<Cr_BLOCK>(crcb_mb_xy.first, crcb_mb_xy.second))
		{
			break;
		}
		ref_frame.get_block_from_framexy<Y_BLOCK>(y_mb_xy, luma_pred);
		ref_frame.get_block_from_framexy<Cb_BLOCK>(crcb_mb_xy, cb_pred);
		ref_frame.get_block_from_framexy<Cr_BLOCK>(crcb_mb_xy, cr_pred);
		for (int i = 0; i < 256; i++)
		{
			diff = mb.Y[i] - luma_pred[i];
			luma_pred[i] = diff;
			y_error += abs(luma_pred[i]);
		}
		for (int i = 0; i < 64; i++)
		{
			diff = mb.Cr[i] - cr_pred[i];
			cr_pred[i] = diff;
			diff = mb.Cb[i] - cb_pred[i];
			cb_pred[i] = diff;
			cr_cb_error += abs(cr_pred[i]) + abs(cb_pred[i]);
		}
		if (y_error < min_y_sad)
		{
			min_y_sad = y_error;
			std::copy(luma_pred.begin(), luma_pred.end(), luma_res.begin());
			std::copy(cb_pred.begin(), cb_pred.end(), cb_res.begin());
			std::copy(cr_pred.begin(), cr_pred.end(), cr_res.begin());
			mv = std::pair<int, int>(0, i);
		}
	}
	for (int i = 1; i <= SEARCH_STEP_LEN; i++)
	{
		y_mb_xy.first--;
		crcb_mb_xy.first--;
		if (!ref_frame.is_valid_xy<Y_BLOCK>(y_mb_xy.first, y_mb_xy.second) || !ref_frame.is_valid_xy<Cr_BLOCK>(crcb_mb_xy.first, crcb_mb_xy.second))
		{
			break;
		}
		ref_frame.get_block_from_framexy<Y_BLOCK>(y_mb_xy, luma_pred);
		ref_frame.get_block_from_framexy<Cb_BLOCK>(crcb_mb_xy, cb_pred);
		ref_frame.get_block_from_framexy<Cr_BLOCK>(crcb_mb_xy, cr_pred);
		for (int i = 0; i < 256; i++)
		{
			diff = mb.Y[i] - luma_pred[i];
			luma_pred[i] = diff;
			y_error += abs(luma_pred[i]);
		}
		for (int i = 0; i < 64; i++)
		{
			diff = mb.Cr[i] - cr_pred[i];
			cr_pred[i] = diff;
			diff = mb.Cb[i] - cb_pred[i];
			cb_pred[i] = diff;
			cr_cb_error += abs(cr_pred[i]) + abs(cb_pred[i]);
		}
		if (y_error < min_y_sad)
		{
			min_y_sad = y_error;
			std::copy(luma_pred.begin(), luma_pred.end(), luma_res.begin());
			std::copy(cb_pred.begin(), cb_pred.end(), cb_res.begin());
			std::copy(cr_pred.begin(), cr_pred.end(), cr_res.begin());
			mv = std::pair<int, int>(-1 * i, 0);
		}
	}
	for (int i = 1; i <= SEARCH_STEP_LEN; i++)
	{
		y_mb_xy.first++;
		crcb_mb_xy.first++;
		if (!ref_frame.is_valid_xy<Y_BLOCK>(y_mb_xy.first, y_mb_xy.second) || !ref_frame.is_valid_xy<Cr_BLOCK>(crcb_mb_xy.first, crcb_mb_xy.second))
		{
			break;
		}
		ref_frame.get_block_from_framexy<Y_BLOCK>(y_mb_xy, luma_pred);
		ref_frame.get_block_from_framexy<Cb_BLOCK>(crcb_mb_xy, cb_pred);
		ref_frame.get_block_from_framexy<Cr_BLOCK>(crcb_mb_xy, cr_pred);
		for (int i = 0; i < 256; i++)
		{
			diff = mb.Y[i] - luma_pred[i];
			luma_pred[i] = diff;
			y_error += abs(luma_pred[i]);
		}
		for (int i = 0; i < 64; i++)
		{
			diff = mb.Cr[i] - cr_pred[i];
			cr_pred[i] = diff;
			diff = mb.Cb[i] - cb_pred[i];
			cb_pred[i] = diff;
			cr_cb_error += abs(cr_pred[i]) + abs(cb_pred[i]);
		}
		if (y_error < min_y_sad)
		{
			min_y_sad = y_error;
			std::copy(luma_pred.begin(), luma_pred.end(), luma_res.begin());
			std::copy(cb_pred.begin(), cb_pred.end(), cb_res.begin());
			std::copy(cr_pred.begin(), cr_pred.end(), cr_res.begin());
			mv = std::pair<int, int>(i, 0);
		}
	}
	mb.Y = luma_res;
	mb.Cb = cb_res;
	mb.Cr = cr_res;
}

void get_mvp(MacroBlock &mb, Frame &frame)
{

	std::vector<std::pair<int, int>> mv_vec(3, std::pair<int, int>(0, 0));
	if (mb.mb_row > 0)
	{
		mv_vec[0] = frame.mbs[(mb.mb_row - 1) * frame.nb_mb_rows + mb.mb_col].mv;
	}
	if (mb.mb_col > 0)
	{
		mv_vec[1] = frame.mbs[mb.mb_row * frame.nb_mb_rows + mb.mb_col - 1].mv;
	}
	if (mb.mb_row > 0 && mb.mb_col < frame.nb_mb_cols - 1)
	{
		mv_vec[2] = frame.mbs[(mb.mb_row - 1) * frame.nb_mb_rows + mb.mb_col + 1].mv;
	}
	else if (mb.mb_row > 0 && mb.mb_col > 0)
	{
		mv_vec[2] = frame.mbs[(mb.mb_row - 1) * frame.nb_mb_rows + mb.mb_col - 1].mv;
	}
	auto get_mv_v = [](std::pair<int, int> &mv) -> unsigned int
	{ return mv.first * mv.first + mv.second * mv.second; };

	auto mv_comp = [&](std::pair<int, int> &mv1, std::pair<int, int> &mv2) -> bool
	{ return get_mv_v(mv1) < get_mv_v(mv2); };
	std::sort(mv_vec.begin(), mv_vec.end(), mv_comp);
	if (mv_vec[2] == std::pair<int, int>(0, 0))
	{
		mb.mvp = mb.mv;
	}
	else if (mv_vec[1] == std::pair<int, int>(0, 0))
	{
		mb.mvp = mv_vec[2];
	}
	else if (mv_vec[0] == std::pair<int, int>(0, 0))
	{
		mb.mvp = mv_vec[1];
	}
	else
	{
		mb.mvp = mv_vec[0];
	}
}

void encode_P_frame(Frame &frame)
{
	// std::vector<MacroBlock> decoded_blocks;
	// decoded_blocks.reserve(frame.mbs.size());
	for (auto &mb : frame.mbs)
	{
		// std::cout << "start p code (" << mb.mb_row << "," << mb.mb_col << ")" << std::endl;
		std::cout << "the " << mb.mb_index << " mb block" << std::endl;
		std::cout << "before P pred " << std::endl;
		for (int i = 0; i < 256; i++)
		{
			std::cout << mb.Y[i] << " ";
			if (i % 16 == 15)
			{
				std::cout << std::endl;
			}
		}
		get_mv(mb, ref_frame_list[0]);
		// std::cout << "after P pred " <<"("<<mb.mv.first<<","<<mb.mv.second<<")"<< std::endl;
		// for (int i = 0; i < 256; i++)
		//{
		//	std::cout << mb.Y[i] << " ";
		//	if (i % 16 == 15)
		//	{
		//		std::cout << std::endl;
		//	}
		// }
		get_mvp(mb, frame);
		// std::cout << "the " << mb.mb_index << " mv is" << "(" << mb.mv.first << "," << mb.mv.second << ")" << std::endl;

		std::cout << "before qdct" << std::endl;
		for (int i = 0; i < 256; i++)
		{
			std::cout << mb.Y[i] << " ";
			if (i % 16 == 15)
			{
				std::cout << std::endl;
			}
		}
		// QDCT
		for (int cur_pos = 0; cur_pos < 16; cur_pos++)
		{
			qdct_luma4x4_intra(mb.get_Y_4x4_block(cur_pos));
		}

		// qdct_luma16x16_intra(mb.Y);
		std::cout << "after qdct" << std::endl;
		for (int i = 0; i < 256; i++)
		{
			std::cout << mb.Y[i] << " ";
			if (i % 16 == 15)
			{
				std::cout << std::endl;
			}
		}
		qdct_chroma8x8_intra(mb.Cb);
		qdct_chroma8x8_intra(mb.Cr);
		// decoded_blocks[mb.mb_index] = mb;
		//  inv_qdct_luma16x16_intra(decoded_blocks[mb.mb_index].Y);
		//  inv_qdct_chroma8x8_intra(decoded_blocks[mb.mb_index].Cb);
		//  inv_qdct_chroma8x8_intra(decoded_blocks[mb.mb_index].Cr);
		//  Block16x16 y_block;
		//  Block8x8 cb_block;
		//  Block8x8 cr_block;
		//  ref_frame_list[0].get_block_from_framexy<Y_BLOCK>(mb.mv, y_block);
		//  ref_frame_list[0].get_block_from_framexy<Cb_BLOCK>(mb.mv, cb_block);
		//  ref_frame_list[0].get_block_from_framexy<Cr_BLOCK>(mb.mv, cr_block);
		//  std::transform(decoded_blocks[mb.mb_index].Y.begin(), decoded_blocks[mb.mb_index].Y.end(), y_block.begin(), decoded_blocks[mb.mb_index].Y.begin(), std::plus<int>());
		//  std::transform(decoded_blocks[mb.mb_index].Cb.begin(), decoded_blocks[mb.mb_index].Cb.end(), cb_block.begin(), decoded_blocks[mb.mb_index].Cb.begin(), std::plus<int>());
		//  std::transform(decoded_blocks[mb.mb_index].Cr.begin(), decoded_blocks[mb.mb_index].Cr.end(), cr_block.begin(), decoded_blocks[mb.mb_index].Cr.begin(), std::plus<int>());
		//  mb.is_intra16x16 = true;
	}
}
void encode_I_frame(Frame &frame)
{
	// decoded Y blocks for intra prediction
	std::vector<MacroBlock> &decoded_blocks = ref_frame_list[0].mbs;
	decoded_blocks.reserve(frame.mbs.size());

	int mb_no = 0;
	for (auto &mb : frame.mbs)
	{
		f_logger.log(Level::DEBUG, "mb #" + std::to_string(mb_no++));
		decoded_blocks.push_back(mb);
		MacroBlock origin_block = mb;

		int error_luma = encode_Y_block(mb, decoded_blocks, frame);
		int error_chroma = encode_Cr_Cb_block(mb, decoded_blocks, frame);

		if (error_luma > 2000 || error_chroma > 1000)
		{
			f_logger.log(Level::VERBOSE, "error exceeded: luma " + std::to_string(error_luma) + " chroma: " + std::to_string(error_chroma));
			mb = origin_block;
			decoded_blocks.back() = origin_block;
			mb.is_I_PCM = true;
		}
	}

	// in-loop deblocking filter
	deblocking_filter(decoded_blocks, frame);
	ref_frame_list[0] = frame;
}

int encode_Y_block(MacroBlock &mb, std::vector<MacroBlock> &decoded_blocks, Frame &frame)
{
	// temp marcoblock for choosing two predicitons
	MacroBlock temp_block = mb;
	MacroBlock temp_decoded_block = mb;

	// perform intra16x16 prediction
	int error_intra16x16 = encode_Y_intra16x16_block(mb, decoded_blocks, frame);

	// perform intra4x4 prediction
	int error_intra4x4 = 0;
	if (SUPPORT_INTRA4x4)
	{
		for (int i = 0; i != 16; i++)
			error_intra4x4 += encode_Y_intra4x4_block(i, temp_block, temp_decoded_block, decoded_blocks, frame);
	}

	// compare the error of two predictions
	// if (error_intra4x4 < error_intra16x16) {
	if (SUPPORT_INTRA4x4 && error_intra4x4 < error_intra16x16)
	{
		mb = temp_block;
		decoded_blocks.at(mb.mb_index) = temp_decoded_block;
		std::string mode = "\tmode:";
		for (auto &m : mb.intra4x4_Y_mode)
			mode += " " + std::to_string(static_cast<int>(m));
		f_logger.log(Level::DEBUG, "luma intra4x4\terror: " + std::to_string(error_intra4x4) + mode);
		mb.is_intra16x16 = false;
		return error_intra4x4;
	}
	else
	{
		f_logger.log(Level::DEBUG, "luma intra16x16\terror: " + std::to_string(error_intra16x16) + "\tmode: " + std::to_string(static_cast<int>(mb.intra16x16_Y_mode)));
		mb.is_intra16x16 = true;
		return error_intra16x16;
	}
}

int encode_Y_intra16x16_block(MacroBlock &mb, std::vector<MacroBlock> &decoded_blocks, Frame &frame)
{
	auto get_decoded_Y_block = [&](int direction)
	{
		int index = frame.get_neighbor_index(mb.mb_index, direction);
		if (index == -1)
			return std::experimental::optional<std::reference_wrapper<Block16x16>>();
		else
			return std::experimental::optional<std::reference_wrapper<Block16x16>>(decoded_blocks.at(index).Y);
	};

	// apply intra prediction
	int error;
	Intra16x16Mode mode;
	std::tie(error, mode) = intra16x16(mb.Y, get_decoded_Y_block(MB_NEIGHBOR_UL),
									   get_decoded_Y_block(MB_NEIGHBOR_U),
									   get_decoded_Y_block(MB_NEIGHBOR_L));

	mb.intra16x16_Y_mode = mode;

	// QDCT
	qdct_luma16x16_intra(mb.Y);

	// reconstruct for later prediction
	decoded_blocks.at(mb.mb_index).Y = mb.Y;
	inv_qdct_luma16x16_intra(decoded_blocks.at(mb.mb_index).Y);
	intra16x16_reconstruct(decoded_blocks.at(mb.mb_index).Y,
						   get_decoded_Y_block(MB_NEIGHBOR_UL),
						   get_decoded_Y_block(MB_NEIGHBOR_U),
						   get_decoded_Y_block(MB_NEIGHBOR_L),
						   mode);

	for (int i = 0; i < 16; i++)
		for (int j = 0; j < 16; j++)
			decoded_blocks.at(mb.mb_index).Y[i * 16 + j] = std::max(16, std::min(235, decoded_blocks.at(mb.mb_index).Y[i * 16 + j]));

	return error;
}

int encode_Y_intra4x4_block(int cur_pos, MacroBlock &mb, MacroBlock &decoded_block, std::vector<MacroBlock> &decoded_blocks, Frame &frame)
{
	int temp_pos = MacroBlock::convert_table[cur_pos];

	auto get_4x4_block = [&](int index, int pos)
	{
		if (index == -1)
			return std::experimental::optional<Block4x4>();
		else if (index == mb.mb_index)
			return std::experimental::optional<Block4x4>(decoded_block.get_Y_4x4_block(pos));
		else
			return std::experimental::optional<Block4x4>(decoded_blocks.at(index).get_Y_4x4_block(pos));
	};

	auto get_UL_4x4_block = [&]()
	{
		int index, pos;
		if (temp_pos == 0)
		{
			index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_UL);
			pos = 15;
		}
		else if (1 <= temp_pos && temp_pos <= 3)
		{
			index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
			pos = 11 + temp_pos;
		}
		else if (temp_pos % 4 == 0)
		{
			index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
			pos = temp_pos - 1;
		}
		else
		{
			index = mb.mb_index;
			pos = temp_pos - 5;
		}

		return get_4x4_block(index, MacroBlock::convert_table[pos]);
	};

	auto get_U_4x4_block = [&]()
	{
		int index, pos;
		if (0 <= temp_pos && temp_pos <= 3)
		{
			index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
			pos = 12 + temp_pos;
		}
		else
		{
			index = mb.mb_index;
			pos = temp_pos - 4;
		}

		return get_4x4_block(index, MacroBlock::convert_table[pos]);
	};

	auto get_UR_4x4_block = [&]()
	{
		int index, pos;
		if (temp_pos == 3)
		{
			index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_UR);
			pos = 12;
		}
		else if (temp_pos == 5 || temp_pos == 13)
		{
			index = -1;
			pos = 0;
		}
		else if (0 <= temp_pos && temp_pos <= 2)
		{
			index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_U);
			pos = 1 + temp_pos;
		}
		else if ((temp_pos + 1) % 4 == 0)
		{
			index = -1;
			pos = 0;
		}
		else
		{
			index = mb.mb_index;
			pos = temp_pos - 3;
		}

		return get_4x4_block(index, MacroBlock::convert_table[pos]);
	};

	auto get_L_4x4_block = [&]()
	{
		int index, pos;
		if (temp_pos % 4 == 0)
		{
			index = frame.get_neighbor_index(mb.mb_index, MB_NEIGHBOR_L);
			pos = temp_pos + 3;
		}
		else
		{
			index = mb.mb_index;
			pos = temp_pos - 1;
		}

		return get_4x4_block(index, MacroBlock::convert_table[pos]);
	};

	int error = 0;
	Intra4x4Mode mode;
	std::tie(error, mode) = intra4x4(mb.get_Y_4x4_block(cur_pos),
									 get_UL_4x4_block(),
									 get_U_4x4_block(),
									 get_UR_4x4_block(),
									 get_L_4x4_block());

	mb.intra4x4_Y_mode.at(cur_pos) = mode;

	// QDCT
	qdct_luma4x4_intra(mb.get_Y_4x4_block(cur_pos));

	// reconstruct for later prediction
	auto temp_4x4 = decoded_block.get_Y_4x4_block(cur_pos);
	auto temp_mb = mb.get_Y_4x4_block(cur_pos);
	for (int i = 0; i != 16; i++)
		temp_4x4[i] = temp_mb[i];
	inv_qdct_luma4x4_intra(decoded_block.get_Y_4x4_block(cur_pos));
	intra4x4_reconstruct(decoded_block.get_Y_4x4_block(cur_pos),
						 get_UL_4x4_block(),
						 get_U_4x4_block(),
						 get_UR_4x4_block(),
						 get_L_4x4_block(),
						 mode);

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			decoded_block.get_Y_4x4_block(cur_pos)[i * 4 + j] = std::max(16, std::min(235, decoded_block.get_Y_4x4_block(cur_pos)[i * 4 + j]));

	return error;
}

int encode_Cr_Cb_block(MacroBlock &mb, std::vector<MacroBlock> &decoded_blocks, Frame &frame)
{
	int error_intra8x8 = encode_Cr_Cb_intra8x8_block(mb, decoded_blocks, frame);
	f_logger.log(Level::DEBUG, "chroma intra8x8\terror: " + std::to_string(error_intra8x8) + "\tmode: " + std::to_string(static_cast<int>(mb.intra_Cr_Cb_mode)));

	return error_intra8x8;
}

int encode_Cr_Cb_intra8x8_block(MacroBlock &mb, std::vector<MacroBlock> &decoded_blocks, Frame &frame)
{
	auto get_decoded_Cr_block = [&](int direction)
	{
		int index = frame.get_neighbor_index(mb.mb_index, direction);
		if (index == -1)
			return std::experimental::optional<std::reference_wrapper<Block8x8>>();
		else
			return std::experimental::optional<std::reference_wrapper<Block8x8>>(decoded_blocks.at(index).Cr);
	};

	auto get_decoded_Cb_block = [&](int direction)
	{
		int index = frame.get_neighbor_index(mb.mb_index, direction);
		if (index == -1)
			return std::experimental::optional<std::reference_wrapper<Block8x8>>();
		else
			return std::experimental::optional<std::reference_wrapper<Block8x8>>(decoded_blocks.at(index).Cb);
	};

	int error;
	IntraChromaMode mode;
	std::tie(error, mode) = intra8x8_chroma(mb.Cr, get_decoded_Cr_block(MB_NEIGHBOR_UL),
											get_decoded_Cr_block(MB_NEIGHBOR_U),
											get_decoded_Cr_block(MB_NEIGHBOR_L),
											mb.Cb, get_decoded_Cb_block(MB_NEIGHBOR_UL),
											get_decoded_Cb_block(MB_NEIGHBOR_U),
											get_decoded_Cb_block(MB_NEIGHBOR_L));

	mb.intra_Cr_Cb_mode = mode;

	// QDCT
	qdct_chroma8x8_intra(mb.Cr);
	qdct_chroma8x8_intra(mb.Cb);

	// reconstruct for later prediction
	decoded_blocks.at(mb.mb_index).Cr = mb.Cr;
	inv_qdct_chroma8x8_intra(decoded_blocks.at(mb.mb_index).Cr);
	intra8x8_chroma_reconstruct(decoded_blocks.at(mb.mb_index).Cr,
								get_decoded_Cr_block(MB_NEIGHBOR_UL),
								get_decoded_Cr_block(MB_NEIGHBOR_U),
								get_decoded_Cr_block(MB_NEIGHBOR_L),
								mode);

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			decoded_blocks.at(mb.mb_index).Cr[i * 8 + j] = std::max(16, std::min(240, decoded_blocks.at(mb.mb_index).Cr[i * 8 + j]));

	decoded_blocks.at(mb.mb_index).Cb = mb.Cb;
	inv_qdct_chroma8x8_intra(decoded_blocks.at(mb.mb_index).Cb);
	intra8x8_chroma_reconstruct(decoded_blocks.at(mb.mb_index).Cb,
								get_decoded_Cb_block(MB_NEIGHBOR_UL),
								get_decoded_Cb_block(MB_NEIGHBOR_U),
								get_decoded_Cb_block(MB_NEIGHBOR_L),
								mode);

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < 8; j++)
			decoded_blocks.at(mb.mb_index).Cb[i * 8 + j] = std::max(16, std::min(240, decoded_blocks.at(mb.mb_index).Cb[i * 8 + j]));

	return error;
}
