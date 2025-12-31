/*
 * Project 25 IMBE Encoder Fixed-Point implementation
 * Developed by Pavel Yazev E-mail: pyazev@gmail.com
 * Version 1.0 (c) Copyright 2009
 *
 * Modified for OpenDMR - encode-only version
 */

#include <cstdio>

#include "imbe_vocoder_impl.h"

imbe_vocoder_impl::imbe_vocoder_impl(void) :
	prev_pitch(0),
	prev_prev_pitch(0),
	prev_e_p(0),
	prev_prev_e_p(0),
	seed(1),
	num_harms_prev1(0),
	num_harms_prev2(0),
	th_max(0),
	dc_rmv_mem(0)
{
	memset(wr_array, 0, sizeof(wr_array));
	memset(wi_array, 0, sizeof(wi_array));
	memset(pitch_est_buf, 0, sizeof(pitch_est_buf));
	memset(pitch_ref_buf, 0, sizeof(pitch_ref_buf));
	memset(pe_lpf_mem, 0, sizeof(pe_lpf_mem));
	memset(fft_buf, 0, sizeof(fft_buf));
	memset(sa_prev1, 0, sizeof(sa_prev1));
	memset(sa_prev2, 0, sizeof(sa_prev2));
	memset(v_uv_dsn, 0, sizeof(v_uv_dsn));

	memset(&my_imbe_param, 0, sizeof(IMBE_PARAM));

	encode_init();
}
