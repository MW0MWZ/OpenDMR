/*
 * MBEEncoder - DMR AMBE+2 Encoder
 *
 * Based on OP25 MBE Encoder by Max H. Parke KA1RBI
 * Simplified for DMR-only use by OpenDMR project
 *
 * Copyright (C) 2013, 2014 Max H. Parke KA1RBI
 *
 * This file is part of OP25 and is free software; you can redistribute
 * it and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 */

#ifndef MBEENC_H
#define MBEENC_H

#include <stdint.h>
#include "mbelib.h"
#include "imbe_vocoder.h"

class MBEEncoder {
public:
	MBEEncoder();
	~MBEEncoder();

	/**
	 * Set gain adjustment factor.
	 * @param gain_adjust Linear gain multiplier (1.0 = no change)
	 */
	void set_gain_adjust(const float gain_adjust) { d_gain_adjust = gain_adjust; }

	/**
	 * Enable DMR mode (AMBE+2). This is the default and only supported mode.
	 */
	void set_dmr_mode(void);

	/**
	 * Analyze PCM and return b[9] voice parameters for DMR.
	 * Use this when you want to do your own FEC encoding.
	 *
	 * @param samples Input: 160 PCM samples (16-bit signed, 8kHz)
	 * @param b       Output: 9 voice parameter values
	 */
	void encode_dmr_params(const int16_t samples[], int b[9]);

	/**
	 * Encode 49-bit voice data to 72-bit DMR frame with FEC.
	 *
	 * @param in  Input: 49 bits packed (u0[12] + u1[12] + raw[25])
	 * @param out Output: 72 bits (9 bytes) with Golay FEC
	 */
	void encode_dmr(const unsigned char* in, unsigned char* out);

private:
	imbe_vocoder vocoder;
	mbe_parms cur_mp;
	mbe_parms prev_mp;
	float d_gain_adjust;
};

#endif /* MBEENC_H */
