

/*
* This file is part of FFmpeg.
*
* FFmpeg is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* FFmpeg is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with FFmpeg; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

enum AVSampleFormat {
    AV_SAMPLE_FMT_NONE = -1,
    AV_SAMPLE_FMT_U8,              ///< unsigned 8 bits
    AV_SAMPLE_FMT_S16,             ///< signed 16 bits
    AV_SAMPLE_FMT_S32,             ///< signed 32 bits
    AV_SAMPLE_FMT_FLT,             ///< float
    AV_SAMPLE_FMT_DBL,             ///< double
    AV_SAMPLE_FMT_NB               ///< Number of sample formats. DO NOT USE if dynamically linking to libavcodec
};
#include "stdint.h"

#ifdef __cplusplus
extern "C" 
{
#endif

const char *av_get_sample_fmt_name(enum AVSampleFormat sample_fmt);
enum AVSampleFormat av_get_sample_fmt(const char *name);
char *av_get_sample_fmt_string (char *buf, int buf_size, enum AVSampleFormat sample_fmt);
int av_get_bits_per_sample_fmt(enum AVSampleFormat sample_fmt);
int av_samples_fill_arrays(uint8_t *pointers[8], int linesizes[8],
						uint8_t *buf, int nb_channels, int nb_samples,
						   enum AVSampleFormat sample_fmt, int planar, int align);
int av_samples_alloc(uint8_t *pointers[8], int linesizes[8],
					 int nb_samples, int nb_channels,
					 enum AVSampleFormat sample_fmt, int planar,
					 int align);

#ifdef __cplusplus
};
#endif