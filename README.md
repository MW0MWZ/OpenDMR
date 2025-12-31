# OpenDMR

**Open Source DMR (AMBE+2) Vocoder Library**

A complete software implementation of the DMR AMBE+2 vocoder for encoding and decoding digital voice. No proprietary DVSI hardware required.

## Overview

OpenDMR provides a clean, well-documented C API for:

- **Decoding**: Convert DMR AMBE+2 frames to PCM audio
- **Encoding**: Convert PCM audio to DMR AMBE+2 frames
- **Transcoding**: Decode and re-encode (useful for testing round-trip quality)

The library is designed for easy integration into other projects such as:
- DMR repeaters and reflectors
- Amateur radio gateway software
- Digital voice transcoding systems
- Educational and research applications

## Quick Start

### Building

```bash
cd OpenDMR
make
```

This produces:
- `libopendmr.a` - Static library
- `libopendmr.dylib` (macOS) or `libopendmr.so` (Linux) - Shared library
- `dmr_codec` - Command-line test tool

### Testing with the CLI Tool

```bash
# Decode AMBE+2 to PCM
./dmr_codec decode input.ambe output.raw

# Encode PCM to AMBE+2
./dmr_codec encode input.raw output.ambe

# Transcode (decode then re-encode)
./dmr_codec transcode input.ambe output.ambe

# Show library info
./dmr_codec info
```

### Converting Audio Files

```bash
# Convert raw PCM to WAV
sox -t raw -r 8000 -e signed -b 16 -c 1 output.raw output.wav

# Convert WAV to raw PCM for encoding
sox input.wav -t raw -r 8000 -e signed -b 16 -c 1 output.raw

# Play raw PCM directly
aplay -f S16_LE -r 8000 -c 1 output.raw
```

## API Reference

### Header Include

```c
#include "opendmr.h"
```

### Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `OPENDMR_AMBE_FRAME_BYTES` | 9 | AMBE+2 frame size in bytes (72 bits) |
| `OPENDMR_AMBE_FRAME_BITS` | 72 | AMBE+2 frame size in bits |
| `OPENDMR_PCM_SAMPLES` | 160 | PCM samples per frame (20ms @ 8kHz) |
| `OPENDMR_SAMPLE_RATE` | 8000 | Audio sample rate in Hz |
| `OPENDMR_VOICE_PARAMS` | 49 | Voice parameter bits per frame |

### Decoder API

```c
// Create a decoder instance
opendmr_decoder_t *opendmr_decoder_create(void);

// Decode one AMBE+2 frame to PCM
// Returns: true on success, false on failure
// errs: optional pointer to receive bit error count
bool opendmr_decode(opendmr_decoder_t *dec,
                    const uint8_t ambe[9],
                    int16_t pcm[160],
                    int *errs);

// Reset decoder state (call at start of new transmission)
void opendmr_decoder_reset(opendmr_decoder_t *dec);

// Destroy decoder and free resources
void opendmr_decoder_destroy(opendmr_decoder_t *dec);
```

### Encoder API

```c
// Create an encoder instance
opendmr_encoder_t *opendmr_encoder_create(void);

// Encode one PCM frame to AMBE+2
// Returns: true on success, false on failure
bool opendmr_encode(opendmr_encoder_t *enc,
                    const int16_t pcm[160],
                    uint8_t ambe[9]);

// Set gain adjustment (-20 to +20 dB, default 0)
void opendmr_encoder_set_gain(opendmr_encoder_t *enc, int gain_db);

// Reset encoder state (call at start of new transmission)
void opendmr_encoder_reset(opendmr_encoder_t *enc);

// Destroy encoder and free resources
void opendmr_encoder_destroy(opendmr_encoder_t *enc);
```

### Utility Functions

```c
// Get library version string (e.g., "1.0.0")
const char *opendmr_version(void);

// Convert between byte array and bit array formats
// to_bits=true: bytes[9] -> bits[72]
// to_bits=false: bits[72] -> bytes[9]
void opendmr_convert_frame(uint8_t bytes[9], uint8_t bits[72], bool to_bits);
```

## Integration Examples

### Basic Decoding

```c
#include "opendmr.h"
#include <stdio.h>

int main() {
    opendmr_decoder_t *dec = opendmr_decoder_create();
    if (!dec) {
        fprintf(stderr, "Failed to create decoder\n");
        return 1;
    }

    uint8_t ambe_frame[9];  // Input: 72-bit AMBE+2 frame
    int16_t pcm[160];       // Output: 160 samples of 16-bit PCM
    int errors;

    // Read AMBE frames from your source...
    while (read_ambe_frame(ambe_frame)) {
        if (opendmr_decode(dec, ambe_frame, pcm, &errors)) {
            // Write PCM to audio output...
            write_audio(pcm, 160);
            printf("Decoded frame, %d bit errors corrected\n", errors);
        }
    }

    opendmr_decoder_destroy(dec);
    return 0;
}
```

### Basic Encoding

```c
#include "opendmr.h"
#include <stdio.h>

int main() {
    opendmr_encoder_t *enc = opendmr_encoder_create();
    if (!enc) {
        fprintf(stderr, "Failed to create encoder\n");
        return 1;
    }

    // Optional: adjust gain (+6 dB boost)
    opendmr_encoder_set_gain(enc, 6);

    int16_t pcm[160];       // Input: 160 samples of 16-bit PCM
    uint8_t ambe_frame[9];  // Output: 72-bit AMBE+2 frame

    // Read PCM frames from your source...
    while (read_pcm_frame(pcm)) {
        if (opendmr_encode(enc, pcm, ambe_frame)) {
            // Write AMBE frame to output...
            write_ambe_frame(ambe_frame);
        }
    }

    opendmr_encoder_destroy(enc);
    return 0;
}
```

### Linking

```bash
# Static linking
gcc -o myapp myapp.c -I/path/to/OpenDMR -L/path/to/OpenDMR -lopendmr -lm

# Dynamic linking
gcc -o myapp myapp.c -I/path/to/OpenDMR -L/path/to/OpenDMR -lopendmr -lm
export LD_LIBRARY_PATH=/path/to/OpenDMR:$LD_LIBRARY_PATH
```

### CMake Integration

```cmake
# Add OpenDMR as subdirectory or find the library
add_executable(myapp main.cpp)
target_include_directories(myapp PRIVATE /path/to/OpenDMR)
target_link_libraries(myapp /path/to/OpenDMR/libopendmr.a m)
```

## File Formats

### AMBE+2 Frame Format (.ambe files)

- **Size**: 9 bytes (72 bits) per frame
- **Frame rate**: 50 frames per second
- **Duration**: 20ms per frame
- **Byte order**: MSB first within each byte

Raw .ambe files are simply concatenated 9-byte frames with no header.

### PCM Audio Format (.raw files)

- **Sample rate**: 8000 Hz
- **Bit depth**: 16-bit signed integer
- **Channels**: Mono
- **Byte order**: Little-endian
- **Samples per frame**: 160 (20ms)

Raw .raw files are simply concatenated samples with no header.

## Technical Details

### AMBE+2 Codec Overview

DMR uses the AMBE+2 codec (also called AMBE 3600x2450):
- **Voice data rate**: 2450 bps (49 bits per 20ms frame)
- **FEC overhead**: 1150 bps
- **Total bit rate**: 3600 bps (72 bits per 20ms frame)

### Frame Structure

Each 72-bit AMBE+2 frame consists of three blocks:

```
+------------------+------------------+------------------+
|   A Block (24)   |   B Block (23)   |   C Block (25)   |
+------------------+------------------+------------------+
|  Golay(24,12)    | Golay(23,12)+PRNG|   Raw (C2+C3)    |
|  12-bit C0 data  |  12-bit C1 data  | 11-bit + 14-bit  |
+------------------+------------------+------------------+
```

**A Block (bits 0-23)**:
- Contains C0 voice parameters (12 bits)
- Protected by Golay(24,12) code
- Can correct up to 3 bit errors

**B Block (bits 24-46)**:
- Contains C1 voice parameters (12 bits)
- Protected by Golay(23,12) code
- Scrambled with PRNG seeded by C0 value
- Parity bit removed (24→23 bits)

**C Block (bits 47-71)**:
- Contains C2 (11 bits) and C3 (14 bits) parameters
- No FEC protection (raw data)

### Voice Parameters

The 49-bit voice data encodes 9 parameters (`b[0]` through `b[8]`):

| Parameter | Bits | Description |
|-----------|------|-------------|
| b[0] | 7 | Fundamental frequency (pitch) |
| b[1] | 5 | Voice/unvoiced decisions (L harmonics) |
| b[2] | 5 | Voice/unvoiced decisions (cont.) |
| b[3] | 9 | Gain |
| b[4] | 7 | Spectral magnitudes (PRBA78) |
| b[5] | 5 | Spectral magnitudes (PRBA78) |
| b[6] | 4 | Higher order magnitudes |
| b[7] | 4 | Higher order magnitudes |
| b[8] | 3 | Higher order magnitudes |

### FEC Processing

**Golay(24,12)**:
- Encodes 12 data bits into 24 code bits
- Minimum distance 8, can correct 3 errors

**Golay(23,12)**:
- Same as Golay(24,12) but parity bit removed
- Still maintains error correction capability

**PRNG Scrambling**:
- Uses linear congruential generator: `x[n+1] = (173 * x[n] + 13849) mod 65536`
- Seed derived from C0 data: `x[0] = 16 * C0`
- Produces 23-bit mask for B block descrambling

### Frame Order: DVSI vs Over-the-Air

**Important**: This library uses DVSI/canonical frame order (sequential bits), NOT DMR over-the-air interleaved order.

- **DVSI order**: Bits 0-23 = A, bits 24-46 = B, bits 47-71 = C (sequential)
- **Over-the-air**: Uses interleaving tables (DMR_A_TABLE, DMR_B_TABLE, etc.)

Most software (xlxd, MMDVM, etc.) already de-interleaves the frames before passing them along, so you typically receive data in DVSI order.

## Project Structure

```
OpenDMR/
├── opendmr.h          # Public C API header
├── opendmr.cpp        # Main implementation
├── dmr_codec.cpp      # CLI test tool
├── Makefile           # Build system
├── README.md          # This file
├── LICENSE            # GNU GPL v2
├── decoder/           # AMBE+2 decoder (from mbelib-neo)
│   ├── CREDITS        # Attribution for mbelib-neo
│   ├── mbelib.c/h     # Core decoder API
│   ├── ambe*.c        # AMBE+2 codec implementation
│   ├── ecc*.c         # Error correction (Golay decode)
│   └── pffft.c        # FFT library for audio synthesis
└── encoder/           # AMBE+2 encoder (from OP25)
    ├── CREDITS        # Attribution for MBEEncoder/OP25
    ├── mbeenc.cpp/h   # Encoder wrapper class
    ├── cgolay*.cpp    # Golay FEC encoding
    ├── imbe_vocoder*  # IMBE vocoder core
    └── *.cc           # Signal processing (pitch, spectral analysis)
```

## Credits

OpenDMR integrates code from two open-source projects:

### Decoder (mbelib-neo)

Enhanced version of the original mbelib. See `decoder/CREDITS` for full details.
- **Original mbelib**: Copyright (C) 2010 Pavel Yazev
- **mbelib-neo enhancements**: Copyright (C) 2023 arancormonk
- **PFFFT**: Copyright (C) 2013 Julien Pommier

### Encoder (OP25 MBEEncoder)

AMBE+2 encoder from the OP25 project. See `encoder/CREDITS` for full details.
- **MBEEncoder**: Copyright (C) 2013-2019 Max H. Parke KA1RBI
- **IMBE Vocoder**: Based on OP25 vocoder implementation

## Building Options

### Debug Build

```bash
make CXXFLAGS="-g -O0 -DDEBUG" CFLAGS="-g -O0 -DDEBUG"
```

### Install System-Wide

```bash
sudo make install
# Installs to /usr/local by default

# Or specify custom prefix
sudo make install PREFIX=/opt/opendmr
```

### Uninstall

```bash
sudo make uninstall
```

## Troubleshooting

### Silent or Distorted Audio

1. **Check frame order**: Ensure input frames are in DVSI order (sequential A+B+C), not interleaved
2. **Check sample rate**: Output must be played at 8000 Hz
3. **Check byte order**: PCM should be little-endian 16-bit signed

### High Bit Error Count

1. Normal DMR transmissions may have some bit errors
2. Consistently high errors may indicate:
   - Corrupt input data
   - Wrong frame format/order
   - Incorrect frame boundaries

### Linking Errors

Ensure you link with `-lm` for math functions:
```bash
gcc -o myapp myapp.c -lopendmr -lm
```

## Performance

Typical performance on modern hardware:
- **Decode**: ~0.5ms per frame (3000+ real-time)
- **Encode**: ~2ms per frame (1000+ real-time)
- **Memory**: ~50KB per encoder/decoder instance

## Upstream Sources

This library integrates vocoder implementations from mbelib-neo (decoder)
and OP25 (encoder), both long-standing open-source projects. See
`decoder/CREDITS` and `encoder/CREDITS` for attribution and upstream sources.

## License

This project is licensed under the GNU General Public License v2.0 (GPL-2.0).
See the LICENSE file for the full license text.

Both mbelib-neo and MBEEncoder are GPL-licensed, which requires this combined work to also be GPL.
