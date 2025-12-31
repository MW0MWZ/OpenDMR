/*
 * dmr_codec - Command-line DMR AMBE+2 Codec Tool
 *
 * Demonstrates the OpenDMR library for encoding and decoding DMR voice.
 *
 * Usage:
 *   dmr_codec decode <input.ambe> <output.raw>
 *   dmr_codec encode <input.raw> <output.ambe>
 *   dmr_codec transcode <input.ambe> <output.ambe>
 *
 * File formats:
 *   .ambe - Raw AMBE+2 frames (9 bytes per frame, 72 bits)
 *   .raw  - Raw PCM audio (16-bit signed, 8kHz mono, little-endian)
 *
 * The .raw files can be played with:
 *   aplay -f S16_LE -r 8000 -c 1 output.raw
 *   sox -t raw -r 8000 -e signed -b 16 -c 1 output.raw output.wav
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "opendmr.h"

static void print_usage(const char *prog)
{
    printf("OpenDMR Codec Tool v%s\n", opendmr_version());
    printf("\n");
    printf("Usage:\n");
    printf("  %s decode <input.ambe> <output.raw>   - Decode AMBE+2 to PCM\n", prog);
    printf("  %s encode <input.raw> <output.ambe>   - Encode PCM to AMBE+2\n", prog);
    printf("  %s transcode <in.ambe> <out.ambe>     - Decode and re-encode\n", prog);
    printf("  %s info                               - Show library info\n", prog);
    printf("\n");
    printf("File formats:\n");
    printf("  .ambe - Raw AMBE+2 frames (9 bytes/frame, 72 bits, 50 frames/sec)\n");
    printf("  .raw  - Raw PCM audio (16-bit signed LE, 8kHz mono)\n");
    printf("\n");
    printf("Convert .raw to .wav:\n");
    printf("  sox -t raw -r 8000 -e signed -b 16 -c 1 input.raw output.wav\n");
    printf("\n");
    printf("Convert .wav to .raw:\n");
    printf("  sox input.wav -t raw -r 8000 -e signed -b 16 -c 1 output.raw\n");
    printf("\n");
}

static int do_decode(const char *in_file, const char *out_file)
{
    FILE *fin = fopen(in_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", in_file);
        return 1;
    }

    FILE *fout = fopen(out_file, "wb");
    if (!fout) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", out_file);
        fclose(fin);
        return 1;
    }

    opendmr_decoder_t *dec = opendmr_decoder_create();
    if (!dec) {
        fprintf(stderr, "Error: Failed to create decoder\n");
        fclose(fin);
        fclose(fout);
        return 1;
    }

    uint8_t ambe[OPENDMR_AMBE_FRAME_BYTES];
    int16_t pcm[OPENDMR_PCM_SAMPLES];
    int frames = 0;
    int total_errors = 0;

    while (fread(ambe, 1, OPENDMR_AMBE_FRAME_BYTES, fin) == OPENDMR_AMBE_FRAME_BYTES) {
        int errs = 0;
        if (opendmr_decode(dec, ambe, pcm, &errs)) {
            fwrite(pcm, sizeof(int16_t), OPENDMR_PCM_SAMPLES, fout);
            frames++;
            total_errors += errs;
        } else {
            fprintf(stderr, "Warning: Decode failed for frame %d\n", frames);
        }
    }

    opendmr_decoder_destroy(dec);
    fclose(fin);
    fclose(fout);

    printf("Decoded %d frames (%.2f seconds)\n", frames, frames * 0.02f);
    printf("Total bit errors corrected: %d\n", total_errors);

    return 0;
}

static int do_encode(const char *in_file, const char *out_file)
{
    FILE *fin = fopen(in_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", in_file);
        return 1;
    }

    FILE *fout = fopen(out_file, "wb");
    if (!fout) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", out_file);
        fclose(fin);
        return 1;
    }

    opendmr_encoder_t *enc = opendmr_encoder_create();
    if (!enc) {
        fprintf(stderr, "Error: Failed to create encoder\n");
        fclose(fin);
        fclose(fout);
        return 1;
    }

    int16_t pcm[OPENDMR_PCM_SAMPLES];
    uint8_t ambe[OPENDMR_AMBE_FRAME_BYTES];
    int frames = 0;

    while (fread(pcm, sizeof(int16_t), OPENDMR_PCM_SAMPLES, fin) == OPENDMR_PCM_SAMPLES) {
        if (opendmr_encode(enc, pcm, ambe)) {
            fwrite(ambe, 1, OPENDMR_AMBE_FRAME_BYTES, fout);
            frames++;
        } else {
            fprintf(stderr, "Warning: Encode failed for frame %d\n", frames);
        }
    }

    opendmr_encoder_destroy(enc);
    fclose(fin);
    fclose(fout);

    printf("Encoded %d frames (%.2f seconds)\n", frames, frames * 0.02f);

    return 0;
}

static int do_transcode(const char *in_file, const char *out_file)
{
    FILE *fin = fopen(in_file, "rb");
    if (!fin) {
        fprintf(stderr, "Error: Cannot open input file '%s'\n", in_file);
        return 1;
    }

    FILE *fout = fopen(out_file, "wb");
    if (!fout) {
        fprintf(stderr, "Error: Cannot open output file '%s'\n", out_file);
        fclose(fin);
        return 1;
    }

    opendmr_decoder_t *dec = opendmr_decoder_create();
    opendmr_encoder_t *enc = opendmr_encoder_create();

    if (!dec || !enc) {
        fprintf(stderr, "Error: Failed to create codec\n");
        if (dec) opendmr_decoder_destroy(dec);
        if (enc) opendmr_encoder_destroy(enc);
        fclose(fin);
        fclose(fout);
        return 1;
    }

    uint8_t ambe_in[OPENDMR_AMBE_FRAME_BYTES];
    uint8_t ambe_out[OPENDMR_AMBE_FRAME_BYTES];
    int16_t pcm[OPENDMR_PCM_SAMPLES];
    int frames = 0;

    while (fread(ambe_in, 1, OPENDMR_AMBE_FRAME_BYTES, fin) == OPENDMR_AMBE_FRAME_BYTES) {
        if (opendmr_decode(dec, ambe_in, pcm, NULL) &&
            opendmr_encode(enc, pcm, ambe_out)) {
            fwrite(ambe_out, 1, OPENDMR_AMBE_FRAME_BYTES, fout);
            frames++;
        } else {
            fprintf(stderr, "Warning: Transcode failed for frame %d\n", frames);
        }
    }

    opendmr_decoder_destroy(dec);
    opendmr_encoder_destroy(enc);
    fclose(fin);
    fclose(fout);

    printf("Transcoded %d frames (%.2f seconds)\n", frames, frames * 0.02f);

    return 0;
}

static void do_info(void)
{
    printf("OpenDMR Library Information\n");
    printf("===========================\n");
    printf("\n");
    printf("Version: %s\n", opendmr_version());
    printf("\n");
    printf("Codec: DMR AMBE+2 (AMBE 3600x2450)\n");
    printf("  - Voice data rate: 2450 bps\n");
    printf("  - FEC overhead: 1150 bps\n");
    printf("  - Total bit rate: 3600 bps\n");
    printf("\n");
    printf("Audio Format:\n");
    printf("  - Sample rate: 8000 Hz\n");
    printf("  - Bit depth: 16-bit signed\n");
    printf("  - Channels: Mono\n");
    printf("  - Frame size: 160 samples (20ms)\n");
    printf("\n");
    printf("AMBE Frame Format:\n");
    printf("  - Size: 72 bits (9 bytes)\n");
    printf("  - Frame rate: 50 fps\n");
    printf("  - Structure: A(24) + B(23) + C(25) bits\n");
    printf("  - FEC: Golay(24,12) on A, Golay(23,12)+PRNG on B\n");
    printf("\n");
    printf("Components:\n");
    printf("  - Decoder: mbelib-neo (GPL)\n");
    printf("  - Encoder: MBEEncoder from OP25 (GPL)\n");
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "decode") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s decode <input.ambe> <output.raw>\n", argv[0]);
            return 1;
        }
        return do_decode(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "encode") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s encode <input.raw> <output.ambe>\n", argv[0]);
            return 1;
        }
        return do_encode(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "transcode") == 0) {
        if (argc != 4) {
            fprintf(stderr, "Usage: %s transcode <input.ambe> <output.ambe>\n", argv[0]);
            return 1;
        }
        return do_transcode(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "info") == 0) {
        do_info();
        return 0;
    }
    else {
        fprintf(stderr, "Unknown command: %s\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }
}
