#include "jp2kdecode.h"
#include <cstdlib>
#include <cstring>
#include <openjpeg.h>

struct buffer_state {
  uint8_t *data;
  int32_t offset;
  int32_t length;
};

#ifndef clamp
template <class myType> myType clamp(myType v, myType lo, myType hi)
{
  myType v2 = v;
  if (v < lo) v2 = lo;
  if (v > hi) v2 = hi;
  return v2;
}
#endif 

#ifndef min
template <class myType> myType min(myType a, myType b)
{
  return a < b ? a : b;
}
#endif

// YCbCr -> RGB chroma contributions
extern const int16_t _R_Cr[256];
extern const int32_t _G_Cb[256];
extern const int32_t _G_Cr[256];
extern const int16_t _B_Cb[256];

struct read_callback_params {
  void *data;
  int32_t datalen;
};


static inline void write_pixel_ycbcr(uint8_t *dest, uint8_t Y,
                                     int16_t R_chroma, int16_t G_chroma,
                                     int16_t B_chroma) {
  int16_t R = Y + R_chroma;
  int16_t G = Y + G_chroma;
  int16_t B = Y + B_chroma;

  R = clamp(R, (int16_t) 0, (int16_t) 255);
  G = clamp(G, (int16_t) 0, (int16_t) 255);
  B = clamp(B, (int16_t) 0, (int16_t) 255);

  *dest = (uint8_t) R;
  dest[1] = (uint8_t) G;
  dest[2] = (uint8_t) B;
}

static inline void write_pixel_rgb(uint8_t *dest,
                                   uint8_t R, uint8_t G, uint8_t B) {
  *dest = R;
  dest[1] = G;
  dest[2] = B;
}

static void unpack_argb(jp2k_colorspace space,
                        opj_image_comp_t *comps,
                        uint8_t *dest,
                        int32_t w, int32_t h) {
  int c0_sub_x = w / comps[0].w;
  int c1_sub_x = w / comps[1].w;
  int c2_sub_x = w / comps[2].w;
  int c0_sub_y = h / comps[0].h;
  int c1_sub_y = h / comps[1].h;
  int c2_sub_y = h / comps[2].h;

  uint8_t *start=dest;

  if (space == JP2K_YCBCR &&
      c0_sub_x == 1 && c1_sub_x == 2 && c2_sub_x == 2 &&
      c0_sub_y == 1 && c1_sub_y == 1 && c2_sub_y == 1) {
    // Aperio 33003
    // printf("YCBCR!!!");
    for (int32_t y = 0; y < h; y++) {
      int32_t c0_row_base = y * comps[0].w;
      int32_t c1_row_base = y * comps[1].w;
      int32_t c2_row_base = y * comps[2].w;
      int32_t x;
      for (x = 0; x < w - 1; x += 2) {
        uint8_t c0 = (uint8_t) comps[0].data[c0_row_base + x];
        uint8_t c1 = (uint8_t) comps[1].data[c1_row_base + (x / 2)];
        uint8_t c2 = (uint8_t) comps[2].data[c2_row_base + (x / 2)];
        int16_t R_chroma = _R_Cr[c2];
        int16_t G_chroma = (_G_Cb[c1] + _G_Cr[c2]) >> 16;
        int16_t B_chroma = _B_Cb[c1];
        write_pixel_ycbcr(dest, c0, R_chroma, G_chroma, B_chroma);
        dest += 3;
        c0 = (uint8_t) comps[0].data[c0_row_base + x + 1];
        write_pixel_ycbcr(dest, c0, R_chroma, G_chroma, B_chroma);
        dest += 3;
      }
      if (x < w) {
        uint8_t c0 = (uint8_t) comps[0].data[c0_row_base + x];
        uint8_t c1 = (uint8_t) comps[1].data[c1_row_base + (x / 2)];
        uint8_t c2 = (uint8_t) comps[2].data[c2_row_base + (x / 2)];
        int16_t R_chroma = _R_Cr[c2];
        int16_t G_chroma = (_G_Cb[c1] + _G_Cr[c2]) >> 16;
        int16_t B_chroma = _B_Cb[c1];
        write_pixel_ycbcr(dest, c0, R_chroma, G_chroma, B_chroma);
        dest += 3;
      }
    }

  } else if (space == JP2K_YCBCR) {
    // Slow fallback
    // printf("YCBCR SLOW!!!");
    for (int32_t y = 0; y < h; y++) {
      int32_t c0_row_base = (y / c0_sub_y) * comps[0].w;
      int32_t c1_row_base = (y / c1_sub_y) * comps[1].w;
      int32_t c2_row_base = (y / c2_sub_y) * comps[2].w;
      for (int32_t x = 0; x < w; x++) {
        uint8_t c0 = (uint8_t) comps[0].data[c0_row_base + (x / c0_sub_x)];
        uint8_t c1 = (uint8_t) comps[1].data[c1_row_base + (x / c1_sub_x)];
        uint8_t c2 = (uint8_t) comps[2].data[c2_row_base + (x / c2_sub_x)];
        int16_t R_chroma = _R_Cr[c2];
        int16_t G_chroma = (_G_Cb[c1] + _G_Cr[c2]) >> 16;
        int16_t B_chroma = _B_Cb[c1];
        write_pixel_ycbcr(dest, c0, R_chroma, G_chroma, B_chroma);
        dest += 3;
      }
    }

  } else if (space == JP2K_RGB &&
             c0_sub_x == 1 && c1_sub_x == 1 && c2_sub_x == 1 &&
             c0_sub_y == 1 && c1_sub_y == 1 && c2_sub_y == 1) {
    // Aperio 33005
    // printf("RGB!!!");
    for (int32_t y = 0; y < h; y++) {
      int32_t c0_row_base = y * comps[0].w;
      int32_t c1_row_base = y * comps[1].w;
      int32_t c2_row_base = y * comps[2].w;
      for (int32_t x = 0; x < w; x++) {
        uint8_t c0 = (uint8_t) comps[0].data[c0_row_base + x];
        uint8_t c1 = (uint8_t) comps[1].data[c1_row_base + x];
        uint8_t c2 = (uint8_t) comps[2].data[c2_row_base + x];
        write_pixel_rgb(dest, c0, c1, c2);
        dest += 3;
      }
    }

  } else if (space == JP2K_RGB) {
    // Slow fallback
    // printf("RGB SLOW!!!");
    for (int32_t y = 0; y < h; y++) {
      int32_t c0_row_base = (y / c0_sub_y) * comps[0].w;
      int32_t c1_row_base = (y / c1_sub_y) * comps[1].w;
      int32_t c2_row_base = (y / c2_sub_y) * comps[2].w;
      for (int32_t x = 0; x < w; x++) {
        uint8_t c0 = (uint8_t) comps[0].data[c0_row_base + (x / c0_sub_x)];
        uint8_t c1 = (uint8_t) comps[1].data[c1_row_base + (x / c1_sub_x)];
        uint8_t c2 = (uint8_t) comps[2].data[c2_row_base + (x / c2_sub_x)];
        write_pixel_rgb(dest, c0, c1, c2);
        dest += 3;
      }
    }
  }

  int64_t diff=(int64_t) (dest-start);
  int64_t max=w*h*3;
  //printf("DIFF=%i  MAX DIFF=%i", diff, max);
  if (diff > max)
  {
    printf("DIFF OVERFLOW!!!!!!!!!!!!!!!!!!!");
    exit(1);// 0000027078F92140
  }
}


static void info_callback(const char *msg, void *data)
{
  (void) msg;
  (void) data;
  return;
}


static void warning_callback(const char *msg,
                             void *data) 
{
  (void) data;
  if (msg)
  {
    printf("OpenJPEG warning: %s\n", msg);
  }
}


static void error_callback(const char *msg, void *data) 
{
  (void) data;
  // OpenJPEG can produce obscure error messages, so make sure to
  if (msg)
  {
    printf("OpenJPEG error: %s\n", msg);
  }
}


static OPJ_SIZE_T read_callback(void *buf, OPJ_SIZE_T count, void *data) 
{
  struct buffer_state *state = (struct buffer_state*) data;

  count = min(count, (OPJ_SIZE_T) (state->length - state->offset));
  if (!count) {
    return (OPJ_SIZE_T) -1;
  }
  memcpy(buf, state->data + state->offset, count);
  state->offset += (int32_t) count;
  return count;
}


static OPJ_OFF_T skip_callback(OPJ_OFF_T count, void *data) 
{
  struct buffer_state *state = (struct buffer_state*) data;

  int32_t orig_offset = state->offset;
  state->offset = clamp((int32_t) (state->offset + count), (int32_t) 0, (int32_t) state->length);
  if (count && state->offset == orig_offset) {
    return -1;
  }
  return state->offset - orig_offset;
}


static OPJ_BOOL seek_callback(OPJ_OFF_T offset, void *data) 
{
  struct buffer_state *state = (struct buffer_state*) data;

  if (offset < 0 || offset > state->length) {
    return OPJ_FALSE;
  }
  state->offset = (int32_t) offset;
  return OPJ_TRUE;
}


bool jp2k_decode(uint8_t *dest, int32_t w, int32_t h,
                 uint8_t *data, int32_t datalen, jp2k_colorspace space) 
{
  struct buffer_state state;
  opj_image_t *image = NULL;
  const char* tmp_err = "Error";
  const char** ptmp_err = &tmp_err;
  bool success = false;

  //if (data != NULL);
  //g_assert(datalen >= 0);
  if (dest == NULL || data == NULL || datalen <= 0) return false;

  // init stream
  // avoid tracking stream offset (and implementing skip callback) by having
  // OpenJPEG read the whole buffer at once
  opj_stream_t *stream = opj_stream_create(datalen, true);
  state.data = data;
  state.length = datalen;
  state.offset = 0;

  opj_stream_set_user_data(stream, &state, NULL);
  opj_stream_set_user_data_length(stream, datalen);
  opj_stream_set_read_function(stream, read_callback);
  opj_stream_set_skip_function(stream, skip_callback);
  opj_stream_set_seek_function(stream, seek_callback);

  // init codec
  opj_codec_t *codec = opj_create_decompress(OPJ_CODEC_J2K);
  opj_dparameters_t parameters;
  memset(&parameters, 0, sizeof(parameters));
  opj_set_default_decoder_parameters(&parameters);
  opj_setup_decoder(codec, &parameters);

  // enable error handlers
  opj_set_warning_handler(codec, warning_callback, &ptmp_err);
  opj_set_error_handler(codec, error_callback, &ptmp_err);
  opj_set_info_handler(codec, info_callback, &ptmp_err);

  // read header
  if (!opj_read_header(stream, codec, &image)) {
    goto DONE;
  }

  // sanity checks
  if (image->x1 != (OPJ_UINT32) w || image->y1 != (OPJ_UINT32) h) {
    goto DONE;
  }
  if (image->numcomps != 3) {
    goto DONE;
  }

  // decode
  if (!opj_decode(codec, stream, image)) {
    goto DONE;
  }

  // copy pixels
  unpack_argb(space, image->comps, dest, w, h);

  success = true;

DONE:
  opj_image_destroy(image);
  opj_destroy_codec(codec);
  opj_stream_destroy(stream);
  return success;
}

