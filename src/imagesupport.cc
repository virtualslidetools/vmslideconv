/**************************************************************************
Initial author: Paul F. Richards (paulrichards321@gmail.com) 2005-2017
https://github.com/paulrichards321/jpg2svs

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*************************************************************************/
#include <cstring>
#include <cstdarg>
#include <cstdint>
#include <fstream>
#include "imagesupport.h"

int dprintf(const char* format, ...)
{
  int cx = 0;
  char debug[1024];
  va_list ap;
  va_start(ap, format);
  cx = vsnprintf(debug, sizeof(debug)-1, format, ap);
  puts(debug);
  va_end(ap);
  return cx;
}


void Image::baseClearAttribs()
{
  mActualWidth = 0;
  mActualHeight = 0;
  mBitCount = 0; 
  mSamplesPerPixel = 0; 
  mLevel = 0;
  mReadWidth = 0;
  mReadHeight = 0;
  mValidObject = false;
  mFileName = "";
  mBitmapSize = 0;
  mBkgColor = 255;
  mErrMsg.str("");
}


void Image::baseCleanup()
{
  mValidObject = false;
}


void RGBALineToRGB(BYTE *pRGBA, int64_t RGBALineSize, BYTE *pRGB, int64_t RGBLineSize, BYTE RGBBackground[3])
{
  int64_t src, dest;
  int64_t ialpha;
  float alpha, compalpha;
  float gamfg, gambg, comppix;
   
  /* Get max sample values in data and frame buffer */
    
  for (src=0,dest=0; src < RGBALineSize; src+=4,dest+=3) {
    /*
       * Get integer version of alpha.
       * Check for opaque and transparent special cases;
     * no compositing needed if so.
     *
     * We show the whole gamma decode/correct process in
     * floating point, but it would more likely be done
     * with lookup tables.
     */
    ialpha = pRGBA[src+3];
   
    if (ialpha == 0) {
      // Foreground image is transparent here, copy background
      pRGB[dest] = RGBBackground[2];
      pRGB[dest+1] = RGBBackground[1];
      pRGB[dest+2] = RGBBackground[0];
    } else if (ialpha == 255) {
      // Opaque, copy foreground
      pRGB[dest] = pRGBA[src];
      pRGB[dest+1] = pRGBA[src+1];
      pRGB[dest+2] = pRGBA[src+2];

    } else {
      /*
       * Compositing is necessary.
       * Get floating-point alpha and its complement.
       * Note: alpha is always linear; gamma does not
       * affect it.
       */
      alpha = (float) ialpha / 255;
      compalpha = (float) 1.0 - (float) alpha;

      for (int i = 0; i < 3; i++) {
        /*
         * Convert foreground and background to floating
         * point, then undo gamma encoding.
         */
        gamfg = (float) pRGBA[src+i] / 255;
        //linfg = pow(gamfg, 1.0 / fg_gamma);
        gambg = (float) RGBBackground[2-i] / 255;
        //linbg = pow(gambg, 1.0 / bg_gamma);
        /* 
         * Composite.
         */
        comppix = gamfg * alpha + gambg * compalpha;
        /*
         * Gamma correct for display.
         * Convert to integer frame buffer pixel.
         */
        pRGB[dest+i] = BYTE(comppix * 255 + 0.5);
      }
    }
  }
  while (dest < RGBLineSize) {
    pRGB[dest] = 0;
    dest++;
  }
}
