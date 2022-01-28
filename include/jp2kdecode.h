#ifndef __JP2K_DECODE_FILE_H
#define __JP2K_DECODE_FILE_H
#include <cstdint>

enum jp2k_colorspace 
{
  JP2K_RGB,
  JP2K_YCBCR,
};

bool jp2k_decode(uint8_t *dest, int32_t w, int32_t h, 
                 uint8_t *data, int32_t datalen, 
                 jp2k_colorspace space);

#endif
