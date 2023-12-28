/*

Copyright (c) 2023, Dominic Szablewski - https://phoboslab.org
SPDX-License-Identifier: MIT

QOA - The "Quite OK Audio" format for fast, lossy audio compression


-- Data Format

QOA encodes pulse-code modulated (PCM) audio data with up to 255 channels, 
sample rates from 1 up to 16777215 hertz and a bit depth of 16 bits.

The compression method employed in QOA is lossy; it discards some information
from the uncompressed PCM data. For many types of audio signals this compression
is "transparent", i.e. the difference from the original file is often not
audible.

QOA encodes 20 samples of 16 bit PCM data into slices of 64 bits. A single
sample therefore requires 3.2 bits of storage space, resulting in a 5x
compression (16 / 3.2).

A QOA file consists of an 8 byte file header, followed by a number of frames.
Each frame contains an 8 byte frame header, the current 16 byte en-/decoder
state per channel and 256 slices per channel. Each slice is 8 bytes wide and
encodes 20 samples of audio data.

All values, including the slices, are big endian. The file layout is as follows:

struct {
	struct {
		char     magic[4];         // magic bytes "qoaf"
		uint32_t samples;          // samples per channel in this file
	} file_header;             

	struct {
		struct {
			uint8_t  num_channels; // no. of channels
			uint24_t samplerate;   // samplerate in hz
			uint16_t fsamples;     // samples per channel in this frame
			uint16_t fsize;        // frame size (includes this header)
		} frame_header;          

		struct {
			int16_t history[4];    // most recent last
			int16_t weights[4];    // most recent last
		} lms_state[num_channels]; 

		qoa_slice_t slices[256][num_channels];

	} frames[ceil(samples / (256 * 20))];
} qoa_file_t;

Each `qoa_slice_t` contains a quantized scalefactor `sf_quant` and 20 quantized
residuals `qrNN`:

.- QOA_SLICE -- 64 bits, 20 samples --------------------------/  /------------.
|        Byte[0]         |        Byte[1]         |  Byte[2]  \  \  Byte[7]   |
| 7  6  5  4  3  2  1  0 | 7  6  5  4  3  2  1  0 | 7  6  5   /  /    2  1  0 |
|------------+--------+--------+--------+---------+---------+-\  \--+---------|
|  sf_quant  |  qr00  |  qr01  |  qr02  |  qr03   |  qr04   | /  /  |  qr19   |
`-------------------------------------------------------------\  \------------`

Each frame except the last must contain exactly 256 slices per channel. The last
frame may contain between 1 .. 256 (inclusive) slices per channel. The last
slice (for each channel) in the last frame may contain less than 20 samples; the
slice still must be 8 bytes wide, with the unused samples zeroed out.

Channels are interleaved per slice. E.g. for 2 channel stereo: 
slice[0] = L, slice[1] = R, slice[2] = L, slice[3] = R ...

A valid QOA file or stream must have at least one frame. Each frame must contain
at least one channel and one sample with a samplerate between 1 .. 16777215
(inclusive).

If the total number of samples is not known by the encoder, the samples in the
file header may be set to 0x00000000 to indicate that the encoder is 
"streaming". In a streaming context, the samplerate and number of channels may
differ from frame to frame. For static files (those with samples set to a
non-zero value), each frame must have the same number of channels and same
samplerate.

Note that this implementation of QOA only handles files with a known total
number of samples.

A decoder should support at least 8 channels. The channel layout for channel
counts 1 .. 8 is:

	1. Mono
	2. L, R
	3. L, R, C 
	4. FL, FR, B/SL, B/SR 
	5. FL, FR, C, B/SL, B/SR 
	6. FL, FR, C, LFE, B/SL, B/SR
	7. FL, FR, C, LFE, B, SL, SR 
	8. FL, FR, C, LFE, BL, BR, SL, SR

QOA predicts each audio sample based on the previously decoded ones using a
"Sign-Sign Least Mean Squares Filter" (LMS). This prediction plus the 
dequantized residual forms the final output sample.

*/

#pragma once

#define QOA_MIN_FILESIZE 16
#define QOA_MAX_CHANNELS 8

#define QOA_SLICE_LEN 20
#define QOA_SLICES_PER_FRAME 256
#define QOA_FRAME_LEN (QOA_SLICES_PER_FRAME * QOA_SLICE_LEN)
#define QOA_LMS_LEN 4
#define QOA_MAGIC 0x716f6166 /* 'qoaf' */

#define QOA_FRAME_SIZE(channels, slices) \
	(8 + QOA_LMS_LEN * 4 * channels + 8 * slices * channels)

typedef struct {
	int history[QOA_LMS_LEN];
	int weights[QOA_LMS_LEN];
} qoa_lms_t;

typedef struct {
	unsigned int channels;
	unsigned int samplerate;
	unsigned int samples;
	qoa_lms_t lms[QOA_MAX_CHANNELS];
	#ifdef QOA_RECORD_TOTAL_ERROR
		double error;
	#endif
} qoa_desc;

unsigned int qoa_encode_header(qoa_desc *qoa, unsigned char *bytes);
unsigned int qoa_encode_frame(const short *sample_data, qoa_desc *qoa, unsigned int frame_len, unsigned char *bytes);
void *qoa_encode(const short *sample_data, qoa_desc *qoa, unsigned int *out_len);

unsigned int qoa_max_frame_size(qoa_desc *qoa);
unsigned int qoa_decode_header(const unsigned char *bytes, int size, qoa_desc *qoa);
unsigned int qoa_decode_frame(const unsigned char *bytes, unsigned int size, qoa_desc *qoa, short *sample_data, unsigned int *frame_len);
short *qoa_decode(const unsigned char *bytes, int size, qoa_desc *file);

int qoa_write(const char *filename, const short *sample_data, qoa_desc *qoa);
void *qoa_read(const char *filename, qoa_desc *qoa);