/*
 * MBEEncoder - DMR AMBE+2 Encoder
 *
 * Based on OP25 MBE Encoder by Max H. Parke KA1RBI
 * Simplified for DMR-only use by OpenDMR project
 *
 * Copyright (C) 2016 Max H. Parke KA1RBI
 *
 * This file is part of OP25 and part of GNU Radio
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 */

#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <cstring>

#include "mbeenc.h"
#include "cgolay24128.h"
#include "ambe3600x2450_const.h"  // DMR AMBE+2 tables

/* Lookup table for b0 (pitch) encoding */
static const short b0_lookup[] = {
	0, 0, 0, 1, 1, 2, 2, 2,
	3, 3, 4, 4, 4, 5, 5, 5,
	6, 6, 7, 7, 7, 8, 8, 8,
	9, 9, 9, 10, 10, 11, 11, 11,
	12, 12, 12, 13, 13, 13, 14, 14,
	14, 15, 15, 15, 16, 16, 16, 17,
	17, 17, 17, 18, 18, 18, 19, 19,
	19, 20, 20, 20, 21, 21, 21, 21,
	22, 22, 22, 23, 23, 23, 24, 24,
	24, 24, 25, 25, 25, 25, 26, 26,
	26, 27, 27, 27, 27, 28, 28, 28,
	29, 29, 29, 29, 30, 30, 30, 30,
	31, 31, 31, 31, 31, 32, 32, 32,
	32, 33, 33, 33, 33, 34, 34, 34,
	34, 35, 35, 35, 35, 36, 36, 36,
	36, 37, 37, 37, 37, 38, 38, 38,
	38, 38, 39, 39, 39, 39, 40, 40,
	40, 40, 40, 41, 41, 41, 41, 42,
	42, 42, 42, 42, 43, 43, 43, 43,
	43, 44, 44, 44, 44, 45, 45, 45,
	45, 45, 46, 46, 46, 46, 46, 47,
	47, 47, 47, 47, 48, 48, 48, 48,
	48, 49, 49, 49, 49, 49, 49, 50,
	50, 50, 50, 50, 51, 51, 51, 51,
	51, 52, 52, 52, 52, 52, 52, 53,
	53, 53, 53, 53, 54, 54, 54, 54,
	54, 54, 55, 55, 55, 55, 55, 56,
	56, 56, 56, 56, 56, 57, 57, 57,
	57, 57, 57, 58, 58, 58, 58, 58,
	58, 59, 59, 59, 59, 59, 59, 60,
	60, 60, 60, 60, 60, 61, 61, 61,
	61, 61, 61, 62, 62, 62, 62, 62,
	62, 63, 63, 63, 63, 63, 63, 63,
	64, 64, 64, 64, 64, 64, 65, 65,
	65, 65, 65, 65, 65, 66, 66, 66,
	66, 66, 66, 67, 67, 67, 67, 67,
	67, 67, 68, 68, 68, 68, 68, 68,
	68, 69, 69, 69, 69, 69, 69, 69,
	70, 70, 70, 70, 70, 70, 70, 71,
	71, 71, 71, 71, 71, 71, 72, 72,
	72, 72, 72, 72, 72, 73, 73, 73,
	73, 73, 73, 73, 73, 74, 74, 74,
	74, 74, 74, 74, 75, 75, 75, 75,
	75, 75, 75, 75, 76, 76, 76, 76,
	76, 76, 76, 76, 77, 77, 77, 77,
	77, 77, 77, 77, 77, 78, 78, 78,
	78, 78, 78, 78, 78, 79, 79, 79,
	79, 79, 79, 79, 79, 80, 80, 80,
	80, 80, 80, 80, 80, 81, 81, 81,
	81, 81, 81, 81, 81, 81, 82, 82,
	82, 82, 82, 82, 82, 82, 83, 83,
	83, 83, 83, 83, 83, 83, 83, 84,
	84, 84, 84, 84, 84, 84, 84, 84,
	85, 85, 85, 85, 85, 85, 85, 85,
	85, 86, 86, 86, 86, 86, 86, 86,
	86, 86, 87, 87, 87, 87, 87, 87,
	87, 87, 87, 88, 88, 88, 88, 88,
	88, 88, 88, 88, 89, 89, 89, 89,
	89, 89, 89, 89, 89, 89, 90, 90,
	90, 90, 90, 90, 90, 90, 90, 90,
	91, 91, 91, 91, 91, 91, 91, 91,
	91, 92, 92, 92, 92, 92, 92, 92,
	92, 92, 92, 93, 93, 93, 93, 93,
	93, 93, 93, 93, 93, 94, 94, 94,
	94, 94, 94, 94, 94, 94, 94, 95,
	95, 95, 95, 95, 95, 95, 95, 95,
	95, 95, 96, 96, 96, 96, 96, 96,
	96, 96, 96, 96, 96, 97, 97, 97,
	97, 97, 97, 97, 97, 97, 97, 98,
	98, 98, 98, 98, 98, 98, 98, 98,
	98, 98, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 100, 100, 100,
	100, 100, 100, 100, 100, 100, 100, 100,
	101, 101, 101, 101, 101, 101, 101, 101,
	101, 101, 101, 101, 102, 102, 102, 102,
	102, 102, 102, 102, 102, 102, 102, 103,
	103, 103, 103, 103, 103, 103, 103, 103,
	103, 103, 103, 104, 104, 104, 104, 104,
	104, 104, 104, 104, 104, 104, 104, 105,
	105, 105, 105, 105, 105, 105, 105, 105,
	105, 105, 105, 106, 106, 106, 106, 106,
	106, 106, 106, 106, 106, 106, 106, 106,
	107, 107, 107, 107, 107, 107, 107, 107,
	107, 107, 107, 107, 108, 108, 108, 108,
	108, 108, 108, 108, 108, 108, 108, 108,
	108, 109, 109, 109, 109, 109, 109, 109,
	109, 109, 109, 109, 109, 109, 110, 110,
	110, 110, 110, 110, 110, 110, 110, 110,
	110, 110, 110, 111, 111, 111, 111, 111,
	111, 111, 111, 111, 111, 111, 111, 111,
	111, 112, 112, 112, 112, 112, 112, 112,
	112, 112, 112, 112, 112, 112, 113, 113,
	113, 113, 113, 113, 113, 113, 113, 113,
	113, 113, 113, 113, 114, 114, 114, 114,
	114, 114, 114, 114, 114, 114, 114, 114,
	114, 114, 115, 115, 115, 115, 115, 115,
	115, 115, 115, 115, 115, 115, 115, 115,
	116, 116, 116, 116, 116, 116, 116, 116,
	116, 116, 116, 116, 116, 116, 117, 117,
	117, 117, 117, 117, 117, 117, 117, 117,
	117, 117, 117, 117, 117, 118, 118, 118,
	118, 118, 118, 118, 118, 118, 118, 118,
	118, 118, 118, 118, 119, 119, 119, 119,
	119, 119, 119, 119, 119, 119, 119, 119,
	119, 119, 119, 120, 120, 120, 120, 120,
	120, 120, 120, 120, 120, 120, 120, 120,
	120, 120, 121, 121, 121, 121, 121, 121,
	121, 121, 121, 121, 121, 121, 121, 121,
	121, 121, 122, 122, 122, 122, 122, 122,
	122, 122, 122, 122, 122, 122, 122, 122,
	122, 123, 123, 123, 123, 123, 123, 123,
	123, 123, 123, 123, 123, 123, 123, 123,
	123, 124
};

/* DMR interleaving tables */
static const unsigned int DMR_A_TABLE[] = {
	0U, 4U, 8U, 12U, 16U, 20U, 24U, 28U,
	32U, 36U, 40U, 44U, 48U, 52U, 56U, 60U,
	64U, 68U, 1U, 5U, 9U, 13U, 17U, 21U
};

static const unsigned int DMR_B_TABLE[] = {
	25U, 29U, 33U, 37U, 41U, 45U, 49U, 53U,
	57U, 61U, 65U, 69U, 2U, 6U, 10U, 14U,
	18U, 22U, 26U, 30U, 34U, 38U, 42U
};

static const unsigned int DMR_C_TABLE[] = {
	46U, 50U, 54U, 58U, 62U, 66U, 70U, 3U,
	7U, 11U, 15U, 19U, 23U, 27U, 31U, 35U,
	39U, 43U, 47U, 51U, 55U, 59U, 63U, 67U, 71U
};

/* PRNG table for B-block scrambling */
static const unsigned int PRNG_TABLE[] = {
	0x000000U, 0x000001U, 0x000002U, 0x000003U, 0x000004U, 0x000007U, 0x000006U, 0x000005U,
	0x000008U, 0x000009U, 0x00000EU, 0x00000FU, 0x00000AU, 0x00000BU, 0x00000CU, 0x00000DU,
	0x000010U, 0x000011U, 0x000012U, 0x000013U, 0x00001CU, 0x00001DU, 0x00001AU, 0x00001BU,
	0x000018U, 0x000019U, 0x000016U, 0x000017U, 0x000014U, 0x000015U, 0x00003AU, 0x00003BU,
	0x000024U, 0x000025U, 0x000026U, 0x000027U, 0x000020U, 0x000021U, 0x000022U, 0x000023U,
	0x000038U, 0x000039U, 0x00002EU, 0x00002FU, 0x000034U, 0x000035U, 0x00002CU, 0x00002DU,
	0x000028U, 0x000029U, 0x00002AU, 0x00002BU, 0x00003CU, 0x00003DU, 0x000032U, 0x000033U,
	0x000030U, 0x000031U, 0x000036U, 0x000037U, 0x00006CU, 0x00006DU, 0x00006AU, 0x00006BU,
	0x000048U, 0x000049U, 0x00004EU, 0x00004FU, 0x00004AU, 0x00004BU, 0x00004CU, 0x00004DU,
	0x000074U, 0x000075U, 0x000076U, 0x000077U, 0x000070U, 0x000071U, 0x000072U, 0x000073U,
	0x000068U, 0x000069U, 0x00005EU, 0x00005FU, 0x000054U, 0x000055U, 0x00005CU, 0x00005DU,
	0x000058U, 0x000059U, 0x00005AU, 0x00005BU, 0x00007CU, 0x00007DU, 0x000062U, 0x000063U,
	0x000040U, 0x000041U, 0x000046U, 0x000047U, 0x000044U, 0x000045U, 0x00007AU, 0x00007BU,
	0x000078U, 0x000079U, 0x00007EU, 0x00007FU, 0x00006EU, 0x00006FU, 0x000064U, 0x000065U,
	0x000066U, 0x000067U, 0x000060U, 0x000061U, 0x000042U, 0x000043U, 0x000050U, 0x000051U,
	0x000052U, 0x000053U, 0x00009CU, 0x00009DU, 0x00009AU, 0x00009BU, 0x000098U, 0x000099U,
	0x000090U, 0x000091U, 0x000096U, 0x000097U, 0x000094U, 0x000095U, 0x0000DAU, 0x0000DBU,
	0x0000D8U, 0x0000D9U, 0x0000E8U, 0x0000E9U, 0x0000EEU, 0x0000EFU, 0x0000ECU, 0x0000EDU,
	0x0000E4U, 0x0000E5U, 0x0000E6U, 0x0000E7U, 0x0000E0U, 0x0000E1U, 0x0000E2U, 0x0000E3U,
	0x0000DCU, 0x0000DDU, 0x0000C2U, 0x0000C3U, 0x0000C0U, 0x0000C1U, 0x0000C6U, 0x0000C7U,
	0x0000F8U, 0x0000F9U, 0x0000FEU, 0x0000FFU, 0x0000FAU, 0x0000FBU, 0x0000FCU, 0x0000FDU,
	0x0000F4U, 0x0000F5U, 0x0000F6U, 0x0000F7U, 0x0000F0U, 0x0000F1U, 0x0000F2U, 0x0000F3U,
	0x0000C4U, 0x0000C5U, 0x0000CAU, 0x0000CBU, 0x0000D4U, 0x0000D5U, 0x0000CCU, 0x0000CDU,
	0x0000C8U, 0x0000C9U, 0x0000CEU, 0x0000CFU, 0x0000D0U, 0x0000D1U, 0x0000DEU, 0x0000DFU,
	0x000080U, 0x000081U, 0x000086U, 0x000087U, 0x000084U, 0x000085U, 0x0000BAU, 0x0000BBU,
	0x0000B8U, 0x0000B9U, 0x0000BEU, 0x0000BFU, 0x0000AEU, 0x0000AFU, 0x0000A4U, 0x0000A5U,
	0x0000A6U, 0x0000A7U, 0x0000A0U, 0x0000A1U, 0x000082U, 0x000083U, 0x000092U, 0x000093U,
	0x0000D2U, 0x0000D3U, 0x0000D6U, 0x0000D7U, 0x000088U, 0x000089U, 0x00008EU, 0x00008FU,
	0x00008AU, 0x00008BU, 0x00008CU, 0x00008DU, 0x0000B4U, 0x0000B5U, 0x0000B6U, 0x0000B7U,
	0x0000B0U, 0x0000B1U, 0x0000B2U, 0x0000B3U, 0x0000ACU, 0x0000ADU, 0x0000A2U, 0x0000A3U,
	0x0000BCU, 0x0000BDU, 0x0000AAU, 0x0000ABU, 0x0000A8U, 0x0000A9U, 0x0000AEU, 0x0000AFU
};

/* Bit manipulation macros */
#define READ_BIT(p, i)    (((p)[(i) >> 3] >> (7 - ((i) & 7))) & 1)
#define WRITE_BIT(p, i, b) (p)[(i) >> 3] = ((b) ? ((p)[(i) >> 3] | (1 << (7 - ((i) & 7)))) : ((p)[(i) >> 3] & ~(1 << (7 - ((i) & 7)))))

/* Forward declarations */
static void encode_49bit(uint8_t outp[49], const int b[9]);
static void encode_ambe(const IMBE_PARAM *imbe_param, int b[], mbe_parms *cur_mp, mbe_parms *prev_mp, float gain_adjust);

/*
 * Encode voice parameters to 49-bit DMR format.
 * Called after encode_ambe to update decoder state.
 */
static void encode_49bit(uint8_t outp[49], const int b[9])
{
	outp[0] = (b[0] >> 6) & 1;
	outp[1] = (b[0] >> 5) & 1;
	outp[2] = (b[0] >> 4) & 1;
	outp[3] = (b[0] >> 3) & 1;
	outp[4] = (b[1] >> 4) & 1;
	outp[5] = (b[1] >> 3) & 1;
	outp[6] = (b[1] >> 2) & 1;
	outp[7] = (b[1] >> 1) & 1;
	outp[8] = (b[2] >> 4) & 1;
	outp[9] = (b[2] >> 3) & 1;
	outp[10] = (b[2] >> 2) & 1;
	outp[11] = (b[2] >> 1) & 1;
	outp[12] = (b[3] >> 8) & 1;
	outp[13] = (b[3] >> 7) & 1;
	outp[14] = (b[3] >> 6) & 1;
	outp[15] = (b[3] >> 5) & 1;
	outp[16] = (b[3] >> 4) & 1;
	outp[17] = (b[3] >> 3) & 1;
	outp[18] = (b[3] >> 2) & 1;
	outp[19] = (b[3] >> 1) & 1;
	outp[20] = (b[4] >> 6) & 1;
	outp[21] = (b[4] >> 5) & 1;
	outp[22] = (b[4] >> 4) & 1;
	outp[23] = (b[4] >> 3) & 1;
	outp[24] = (b[5] >> 4) & 1;
	outp[25] = (b[5] >> 3) & 1;
	outp[26] = (b[5] >> 2) & 1;
	outp[27] = (b[5] >> 1) & 1;
	outp[28] = (b[6] >> 3) & 1;
	outp[29] = (b[6] >> 2) & 1;
	outp[30] = (b[6] >> 1) & 1;
	outp[31] = (b[7] >> 3) & 1;
	outp[32] = (b[7] >> 2) & 1;
	outp[33] = (b[7] >> 1) & 1;
	outp[34] = (b[8] >> 2) & 1;
	outp[35] = b[1] & 1;
	outp[36] = b[2] & 1;
	outp[37] = (b[0] >> 2) & 1;
	outp[38] = (b[0] >> 1) & 1;
	outp[39] = b[0] & 1;
	outp[40] = b[3] & 1;
	outp[41] = (b[4] >> 2) & 1;
	outp[42] = (b[4] >> 1) & 1;
	outp[43] = b[4] & 1;
	outp[44] = b[5] & 1;
	outp[45] = b[6] & 1;
	outp[46] = b[7] & 1;
	outp[47] = (b[8] >> 1) & 1;
	outp[48] = b[8] & 1;
}

/*
 * Core AMBE+2 encoding function.
 * Converts IMBE parameters to 9 voice parameter values (b[0-8]).
 */
static void encode_ambe(const IMBE_PARAM *imbe_param, int b[], mbe_parms *cur_mp, mbe_parms *prev_mp, float gain_adjust)
{
	static const float SQRT_2 = sqrtf(2.0);
	static const int b0_lmax = sizeof(b0_lookup) / sizeof(b0_lookup[0]);

	/* Encode pitch (b[0]) */
	int b0_i = (imbe_param->ref_pitch >> 5) - 159;
	if (b0_i < 0 || b0_i >= b0_lmax) {
		/* Fallback to silence-ish frame */
		b[0] = 40;
		b[1] = 0;
		b[2] = 0;
		for (int i = 3; i < 9; i++) b[i] = 0;
		return;
	}
	b[0] = b0_lookup[b0_i];

	int L = (int)AmbeLtable[b[0]];

	/* Adjust b0 until L agrees with num_harms */
	while (L != imbe_param->num_harms) {
		if (L < imbe_param->num_harms)
			b0_i++;
		else
			b0_i--;
		if (b0_i < 0 || b0_i >= b0_lmax) {
			b[0] = 40;
			b[1] = 0;
			b[2] = 0;
			for (int i = 3; i < 9; i++) b[i] = 0;
			return;
		}
		b[0] = b0_lookup[b0_i];
		L = (int)AmbeLtable[b[0]];
	}

	/* Compute squared magnitudes */
	float m_float2[NUM_HARMS_MAX];
	for (int l = 1; l <= L; l++) {
		m_float2[l-1] = (float)imbe_param->sa[l-1];
		m_float2[l-1] = m_float2[l-1] * m_float2[l-1];
	}

	/* Encode voice/unvoiced (b[1]) */
	float en_min = 0;
	b[1] = 0;
	for (int n = 0; n < 17; n++) {
		float En = 0;
		for (int l = 1; l <= L; l++) {
			int jl = (int)((float)l * 16.0f * AmbeW0table[b[0]]);
			if (jl > 7) jl = 7;
			if (imbe_param->v_uv_dsn[l-1] != AmbeVuv[n][jl])
				En += m_float2[l-1];
		}
		if (n == 0)
			en_min = En;
		else if (En < en_min) {
			b[1] = n;
			en_min = En;
		}
	}

	/* Compute log spectral amplitudes */
	float num_harms_f = (float)imbe_param->num_harms;
	float log_l_2 = 0.5f * log2f(num_harms_f);
	float log_l_w0 = 0.5f * log2f(num_harms_f * AmbeW0table[b[0]] * 2.0f * (float)M_PI) + 2.289f;

	float lsa[NUM_HARMS_MAX];
	float lsa_sum = 0.0f;
	for (int i1 = 0; i1 < imbe_param->num_harms; i1++) {
		float sa = (float)imbe_param->sa[i1];
		if (sa < 1) sa = 1.0f;
		if (imbe_param->v_uv_dsn[i1])
			lsa[i1] = log_l_2 + log2f(sa);
		else
			lsa[i1] = log_l_w0 + log2f(sa);
		lsa_sum += lsa[i1];
	}

	/* Encode gain (b[2]) */
	float gain = lsa_sum / num_harms_f;
	float diff_gain = gain - 0.5f * prev_mp->gamma - gain_adjust;

	float error;
	int error_index = 0;
	for (int i1 = 0; i1 < 32; i1++) {
		float diff = fabsf(diff_gain - AmbeDg[i1]);
		if (i1 == 0 || diff < error) {
			error = diff;
			error_index = i1;
		}
	}
	b[2] = error_index;

	/* Compute prediction residuals */
	float l_prev_l = (float)(prev_mp->L) / num_harms_f;
	prev_mp->log2Ml[0] = prev_mp->log2Ml[1];

	float T[NUM_HARMS_MAX];
	for (int i1 = 0; i1 < imbe_param->num_harms; i1++) {
		float kl = l_prev_l * (float)(i1 + 1);
		int kl_floor = (int)kl;
		float kl_frac = kl - kl_floor;
		T[i1] = lsa[i1] - 0.65f * (1.0f - kl_frac) * prev_mp->log2Ml[kl_floor]
		              - 0.65f * kl_frac * prev_mp->log2Ml[kl_floor + 1];
	}

	/* DCT */
	const int *J = AmbeLmprbl[imbe_param->num_harms];
	float *c[4];
	int acc = 0;
	for (int i = 0; i < 4; i++) {
		c[i] = &T[acc];
		acc += J[i];
	}

	float C[4][17];
	for (int i = 1; i <= 4; i++) {
		for (int k = 1; k <= J[i-1]; k++) {
			float s = 0.0f;
			for (int j = 1; j <= J[i-1]; j++) {
				s += c[i-1][j-1] * cosf((float)M_PI * ((float)k - 1.0f) * ((float)j - 0.5f) / (float)J[i-1]);
			}
			C[i-1][k-1] = s / (float)J[i-1];
		}
	}

	float R[8];
	R[0] = C[0][0] + SQRT_2 * C[0][1];
	R[1] = C[0][0] - SQRT_2 * C[0][1];
	R[2] = C[1][0] + SQRT_2 * C[1][1];
	R[3] = C[1][0] - SQRT_2 * C[1][1];
	R[4] = C[2][0] + SQRT_2 * C[2][1];
	R[5] = C[2][0] - SQRT_2 * C[2][1];
	R[6] = C[3][0] + SQRT_2 * C[3][1];
	R[7] = C[3][0] - SQRT_2 * C[3][1];

	/* Encode PRBA (G coefficients) */
	float G[8];
	for (int m = 1; m <= 8; m++) {
		G[m-1] = 0.0f;
		for (int i = 1; i <= 8; i++) {
			G[m-1] += R[i-1] * cosf((float)M_PI * ((float)m - 1.0f) * ((float)i - 0.5f) / 8.0f);
		}
		G[m-1] /= 8.0f;
	}

	/* b[3] - PRBA24 */
	for (int i = 0; i < 512; i++) {
		float err = 0.0f;
		float diff = G[1] - AmbePRBA24[i][0]; err += diff * diff;
		diff = G[2] - AmbePRBA24[i][1]; err += diff * diff;
		diff = G[3] - AmbePRBA24[i][2]; err += diff * diff;
		if (i == 0 || err < error) {
			error = err;
			error_index = i;
		}
	}
	b[3] = error_index;

	/* b[4] - PRBA58 */
	for (int i = 0; i < 128; i++) {
		float err = 0.0f;
		float diff = G[4] - AmbePRBA58[i][0]; err += diff * diff;
		diff = G[5] - AmbePRBA58[i][1]; err += diff * diff;
		diff = G[6] - AmbePRBA58[i][2]; err += diff * diff;
		diff = G[7] - AmbePRBA58[i][3]; err += diff * diff;
		if (i == 0 || err < error) {
			error = err;
			error_index = i;
		}
	}
	b[4] = error_index;

	/* b[5] - higher order coefficients */
	int ii = 1;
	if (J[ii-1] <= 2) {
		b[4+ii] = 0;
	} else {
		for (int n = 0; n < 32; n++) {
			float err = 0.0f;
			for (int j = 1; j <= J[ii-1] - 2 && j <= 4; j++) {
				float diff = AmbeHOCb5[n][j-1] - C[ii-1][j+2-1];
				err += diff * diff;
			}
			if (n == 0 || err < error) {
				error = err;
				error_index = n;
			}
		}
		b[4+ii] = error_index;
	}

	/* b[6] */
	ii = 2;
	if (J[ii-1] <= 2) {
		b[4+ii] = 0;
	} else {
		for (int n = 0; n < 16; n++) {
			float err = 0.0f;
			for (int j = 1; j <= J[ii-1] - 2 && j <= 4; j++) {
				float diff = AmbeHOCb6[n][j-1] - C[ii-1][j+2-1];
				err += diff * diff;
			}
			if (n == 0 || err < error) {
				error = err;
				error_index = n;
			}
		}
		b[4+ii] = error_index;
	}

	/* b[7] */
	ii = 3;
	if (J[ii-1] <= 2) {
		b[4+ii] = 0;
	} else {
		for (int n = 0; n < 16; n++) {
			float err = 0.0f;
			for (int j = 1; j <= J[ii-1] - 2 && j <= 4; j++) {
				float diff = AmbeHOCb7[n][j-1] - C[ii-1][j+2-1];
				err += diff * diff;
			}
			if (n == 0 || err < error) {
				error = err;
				error_index = n;
			}
		}
		b[4+ii] = error_index;
	}

	/* b[8] */
	ii = 4;
	if (J[ii-1] <= 2) {
		b[4+ii] = 0;
	} else {
		for (int n = 0; n < 8; n++) {
			float err = 0.0f;
			for (int j = 1; j <= J[ii-1] - 2 && j <= 4; j++) {
				float diff = AmbeHOCb8[n][j-1] - C[ii-1][j+2-1];
				err += diff * diff;
			}
			if (n == 0 || err < error) {
				error = err;
				error_index = n;
			}
		}
		b[4+ii] = error_index;
	}

	/* Update decoder state with quantized values */
	uint8_t ambe_49[49];
	encode_49bit(ambe_49, b);

	char ambe_d[49];
	for (int i = 0; i < 49; i++) {
		ambe_d[i] = ambe_49[i];
	}

	mbe_decodeAmbe2450Parms(ambe_d, cur_mp, prev_mp);
	mbe_moveMbeParms(cur_mp, prev_mp);
}

/*
 * MBEEncoder implementation
 */
MBEEncoder::MBEEncoder()
	: d_gain_adjust(1.0f)
{
	mbe_parms enh_mp;
	mbe_initMbeParms(&cur_mp, &prev_mp, &enh_mp);
}

MBEEncoder::~MBEEncoder()
{
}

void MBEEncoder::set_dmr_mode(void)
{
	/* DMR mode is the only mode - no action needed */
}

void MBEEncoder::encode_dmr_params(const int16_t samples[], int b[9])
{
	int16_t frame_vector[8];  /* Result ignored */

	/* Do speech analysis to generate MBE model parameters
	 * Note: imbe_encode expects non-const pointer but does not modify the samples.
	 * This is a legacy API limitation from the original OP25 code.
	 * We use const_cast here as the underlying implementation only reads the data. */
	vocoder.imbe_encode(frame_vector, const_cast<int16_t*>(samples));

	/* Encode to get b[9] voice parameters */
	encode_ambe(vocoder.param(), b, &cur_mp, &prev_mp, d_gain_adjust);
}

void MBEEncoder::encode_dmr(const unsigned char* in, unsigned char* out)
{
	unsigned int aOrig = 0U;
	unsigned int bOrig = 0U;
	unsigned int cOrig = 0U;

	/* Extract A (u0) - bits 0-11 */
	unsigned int MASK = 0x000800U;
	for (unsigned int i = 0U; i < 12U; i++, MASK >>= 1) {
		if (READ_BIT(in, i))
			aOrig |= MASK;
		if (READ_BIT(in, i + 12U))
			bOrig |= MASK;
	}

	/* Extract C - bits 24-48 */
	MASK = 0x1000000U;
	for (unsigned int i = 0U; i < 25U; i++, MASK >>= 1) {
		if (READ_BIT(in, i + 24U))
			cOrig |= MASK;
	}

	/* Golay encode A */
	unsigned int a = CGolay24128::encode24128(aOrig);

	/* PRNG scramble and Golay encode B */
	unsigned int p = PRNG_TABLE[aOrig] >> 1;
	unsigned int b = CGolay24128::encode23127(bOrig) >> 1;
	b ^= p;

	/* Interleave into output */
	memset(out, 0, 9);

	MASK = 0x800000U;
	for (unsigned int i = 0U; i < 24U; i++, MASK >>= 1) {
		WRITE_BIT(out, DMR_A_TABLE[i], a & MASK);
	}

	MASK = 0x400000U;
	for (unsigned int i = 0U; i < 23U; i++, MASK >>= 1) {
		WRITE_BIT(out, DMR_B_TABLE[i], b & MASK);
	}

	MASK = 0x1000000U;
	for (unsigned int i = 0U; i < 25U; i++, MASK >>= 1) {
		WRITE_BIT(out, DMR_C_TABLE[i], cOrig & MASK);
	}
}
