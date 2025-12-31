/*
 * OpenDMR - Open Source DMR (AMBE+2) Vocoder Library
 *
 * Implementation file
 */

#include "opendmr.h"
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <new>

/* mbelib-neo for decoding */
extern "C" {
#include "mbelib.h"
}

/* MBEEncoder for encoding */
#include "mbeenc.h"

/*
 * ============================================================================
 * Version
 * ============================================================================
 */

#define OPENDMR_VERSION_MAJOR   1
#define OPENDMR_VERSION_MINOR   0
#define OPENDMR_VERSION_PATCH   0

static const char *version_string = "1.0.0";

/*
 * ============================================================================
 * Internal Constants
 * ============================================================================
 */

/* Golay FEC processing */
#include "cgolay24128.h"

/*
 * ============================================================================
 * Decoder Implementation
 * ============================================================================
 */

struct opendmr_decoder {
    mbe_parms cur_mp;
    mbe_parms prev_mp;
    mbe_parms prev_mp_enhanced;
};

opendmr_decoder_t *opendmr_decoder_create(void)
{
    opendmr_decoder_t *dec = static_cast<opendmr_decoder_t *>(calloc(1, sizeof(opendmr_decoder_t)));
    if (dec) {
        mbe_initMbeParms(&dec->cur_mp, &dec->prev_mp, &dec->prev_mp_enhanced);
    }
    return dec;
}

void opendmr_decoder_destroy(opendmr_decoder_t *dec)
{
    free(dec);
}

void opendmr_decoder_reset(opendmr_decoder_t *dec)
{
    if (dec) {
        mbe_initMbeParms(&dec->cur_mp, &dec->prev_mp, &dec->prev_mp_enhanced);
    }
}

/*
 * Compute PRNG mask for B-block descrambling.
 * Uses the same algorithm as mbelib's mbe_demodulateAmbe3600Data_common.
 */
static uint32_t compute_prng_mask_23bit(uint32_t aOrig)
{
    uint16_t pr[24];
    pr[0] = static_cast<uint16_t>(16U * aOrig);

    for (int i = 1; i < 24; i++) {
        pr[i] = static_cast<uint16_t>((173U * static_cast<uint32_t>(pr[i - 1]) + 13849U) % 65536U);
    }

    for (int i = 1; i < 24; i++) {
        pr[i] = pr[i] / 32768U;
    }

    uint32_t mask = 0;
    for (int i = 1; i <= 23; i++) {
        if (pr[i])
            mask |= (1U << (23 - i));
    }
    return mask;
}

/*
 * Decode 72-bit AMBE+2 frame to 49-bit voice parameters.
 *
 * Frame format (DVSI/canonical order):
 *   - Bits 0-23:  A block (Golay 24,12 protected)
 *   - Bits 24-46: B block (Golay 23,12 + PRNG scrambled)
 *   - Bits 47-71: C block (raw: 11-bit C2 + 14-bit C3)
 *
 * Output format (mbelib ambe_d):
 *   - ambe_d[0-11]:  C0 data (12 bits from A)
 *   - ambe_d[12-23]: C1 data (12 bits from B)
 *   - ambe_d[24-34]: C2 data (11 bits)
 *   - ambe_d[35-48]: C3 data (14 bits)
 */
static void decode_ambe_frame(const uint8_t *frame72, char ambe_d[49])
{
    /* Extract A block - bits 0-23 */
    uint32_t a = 0;
    for (int i = 0; i < 24; i++) {
        int byte_idx = i / 8;
        int bit_pos = 7 - (i % 8);
        if ((frame72[byte_idx] >> bit_pos) & 1)
            a |= (0x800000U >> i);
    }

    /* Extract B block - bits 24-46 */
    uint32_t b = 0;
    for (int i = 0; i < 23; i++) {
        int pos = 24 + i;
        int byte_idx = pos / 8;
        int bit_pos = 7 - (pos % 8);
        if ((frame72[byte_idx] >> bit_pos) & 1)
            b |= (0x400000U >> i);
    }

    /* Extract C block - bits 47-71 */
    uint32_t c = 0;
    for (int i = 0; i < 25; i++) {
        int pos = 47 + i;
        int byte_idx = pos / 8;
        int bit_pos = 7 - (pos % 8);
        if ((frame72[byte_idx] >> bit_pos) & 1)
            c |= (0x1000000U >> i);
    }

    /* Golay decode A to get 12-bit C0 data */
    uint32_t aOrig = CGolay24128::decode24128(a);

    /* Descramble B with PRNG, then Golay decode to get 12-bit C1 data */
    uint32_t prng_mask = compute_prng_mask_23bit(aOrig);
    uint32_t b_descrambled = b ^ prng_mask;
    uint32_t bOrig = CGolay24128::decode23127(b_descrambled);

    /* Populate ambe_d in mbelib format */
    memset(ambe_d, 0, 49);

    /* ambe_d[0-11] = C0 data (aOrig, MSB first) */
    for (int i = 0; i < 12; i++)
        ambe_d[i] = (aOrig >> (11 - i)) & 1;

    /* ambe_d[12-23] = C1 data (bOrig, MSB first) */
    for (int i = 0; i < 12; i++)
        ambe_d[12 + i] = (bOrig >> (11 - i)) & 1;

    /* ambe_d[24-48] = C2 + C3 (from c, MSB first) */
    for (int i = 0; i < 25; i++)
        ambe_d[24 + i] = (c >> (24 - i)) & 1;
}

bool opendmr_decode(opendmr_decoder_t *dec,
                    const uint8_t ambe[OPENDMR_AMBE_FRAME_BYTES],
                    int16_t pcm[OPENDMR_PCM_SAMPLES],
                    int *errs)
{
    if (!dec || !ambe || !pcm)
        return false;

    /* Decode 72-bit frame to 49-bit voice parameters */
    char ambe_d[49];
    decode_ambe_frame(ambe, ambe_d);

    /* Decode voice parameters to PCM using mbelib */
    int err_count = 0;
    int err_count2 = 0;
    char err_str[64] = {0};

    mbe_processAmbe2450Data(pcm, &err_count, &err_count2, err_str,
                            ambe_d, &dec->cur_mp, &dec->prev_mp,
                            &dec->prev_mp_enhanced, 3);

    if (errs)
        *errs = err_count;

    return true;
}

/*
 * ============================================================================
 * Encoder Implementation
 * ============================================================================
 */

struct opendmr_encoder {
    MBEEncoder *enc;
    int gain_db;
};

opendmr_encoder_t *opendmr_encoder_create(void)
{
    opendmr_encoder_t *enc = static_cast<opendmr_encoder_t *>(calloc(1, sizeof(opendmr_encoder_t)));
    if (enc) {
        enc->enc = new (std::nothrow) MBEEncoder();
        if (!enc->enc) {
            /* Allocation failed - clean up and return NULL */
            free(enc);
            return nullptr;
        }
        enc->enc->set_dmr_mode();   /* AMBE+2 mode */
        enc->enc->set_gain_adjust(1.0f);
        enc->gain_db = 0;
    }
    return enc;
}

void opendmr_encoder_destroy(opendmr_encoder_t *enc)
{
    if (enc) {
        delete enc->enc;
        free(enc);
    }
}

void opendmr_encoder_reset(opendmr_encoder_t *enc)
{
    if (enc && enc->enc) {
        /* Re-create encoder to reset state */
        delete enc->enc;
        enc->enc = new (std::nothrow) MBEEncoder();
        if (!enc->enc) {
            /* Allocation failed - encoder is now in invalid state */
            return;
        }
        enc->enc->set_dmr_mode();
        enc->enc->set_gain_adjust(powf(10.0f, enc->gain_db / 20.0f));
    }
}

void opendmr_encoder_set_gain(opendmr_encoder_t *enc, int gain_db)
{
    if (enc && enc->enc) {
        /* Clamp to reasonable range */
        if (gain_db < -20) gain_db = -20;
        if (gain_db > 20) gain_db = 20;
        enc->gain_db = gain_db;
        enc->enc->set_gain_adjust(powf(10.0f, gain_db / 20.0f));
    }
}

/*
 * Encode 49-bit voice parameters to 72-bit AMBE+2 frame.
 *
 * Input: b[9] voice parameters from MBEEncoder
 * Output: 72-bit frame in DVSI/canonical order
 */
static void encode_ambe_frame(const int b[9], uint8_t *frame72)
{
    /* Pack b[9] into 49 bits */
    static const int b_lengths[9] = {7, 5, 5, 9, 7, 5, 4, 4, 3};
    uint8_t bits49[49];
    int pos = 0;
    for (int i = 0; i < 9; i++) {
        int val = b[i];
        for (int j = b_lengths[i] - 1; j >= 0; j--) {
            bits49[pos++] = (val >> j) & 1;
        }
    }

    /* Extract 12-bit values for FEC encoding */
    /* C0 = bits49[0-11], C1 = bits49[12-23], C2 = bits49[24-34], C3 = bits49[35-48] */
    uint32_t c0 = 0, c1 = 0;
    for (int i = 0; i < 12; i++) {
        c0 = (c0 << 1) | bits49[i];
        c1 = (c1 << 1) | bits49[12 + i];
    }

    /* Golay encode C0 -> A block (24 bits) */
    uint32_t a = CGolay24128::encode24128(c0);

    /* Golay encode C1, then scramble with PRNG -> B block (23 bits) */
    uint32_t b_codeword = CGolay24128::encode23127(c1);
    uint32_t prng_mask = compute_prng_mask_23bit(c0);
    b_codeword ^= prng_mask;

    /* C block = raw C2 + C3 (25 bits) */
    uint32_t c_block = 0;
    for (int i = 24; i < 49; i++) {
        c_block = (c_block << 1) | bits49[i];
    }

    /* Pack into 72-bit output frame (DVSI order) */
    memset(frame72, 0, 9);

    /* A block: bits 0-23 */
    for (int i = 0; i < 24; i++) {
        int byte_idx = i / 8;
        int bit_pos = 7 - (i % 8);
        if ((a >> (23 - i)) & 1)
            frame72[byte_idx] |= (1 << bit_pos);
    }

    /* B block: bits 24-46 */
    for (int i = 0; i < 23; i++) {
        int pos = 24 + i;
        int byte_idx = pos / 8;
        int bit_pos = 7 - (pos % 8);
        if ((b_codeword >> (22 - i)) & 1)
            frame72[byte_idx] |= (1 << bit_pos);
    }

    /* C block: bits 47-71 */
    for (int i = 0; i < 25; i++) {
        int pos = 47 + i;
        int byte_idx = pos / 8;
        int bit_pos = 7 - (pos % 8);
        if ((c_block >> (24 - i)) & 1)
            frame72[byte_idx] |= (1 << bit_pos);
    }
}

bool opendmr_encode(opendmr_encoder_t *enc,
                    const int16_t pcm[OPENDMR_PCM_SAMPLES],
                    uint8_t ambe[OPENDMR_AMBE_FRAME_BYTES])
{
    if (!enc || !enc->enc || !pcm || !ambe)
        return false;

    /* Encode PCM to voice parameters */
    int b[9] = {0};
    enc->enc->encode_dmr_params(pcm, b);

    /* Encode voice parameters to 72-bit frame */
    encode_ambe_frame(b, ambe);

    return true;
}

/*
 * ============================================================================
 * Utility Functions
 * ============================================================================
 */

const char *opendmr_version(void)
{
    return version_string;
}

void opendmr_convert_frame(uint8_t bytes[OPENDMR_AMBE_FRAME_BYTES],
                           uint8_t bits[OPENDMR_AMBE_FRAME_BITS],
                           bool to_bits)
{
    if (to_bits) {
        /* bytes -> bits */
        for (int i = 0; i < 72; i++) {
            int byte_idx = i / 8;
            int bit_pos = 7 - (i % 8);
            bits[i] = (bytes[byte_idx] >> bit_pos) & 1;
        }
    } else {
        /* bits -> bytes */
        memset(bytes, 0, 9);
        for (int i = 0; i < 72; i++) {
            int byte_idx = i / 8;
            int bit_pos = 7 - (i % 8);
            if (bits[i])
                bytes[byte_idx] |= (1 << bit_pos);
        }
    }
}
