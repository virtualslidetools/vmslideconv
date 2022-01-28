/*******************************************************************************
Copyright (c) 2005-2008, Paul F. Richards

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/
#ifndef __TIFFSUPPORT_FILE_H
#define __TIFFSUPPORT_FILE_H

#include <cstring>
#include <cstdio>
#include <cstdint>
#include "imagesupport.h"
#include "safebmp.h"

namespace TIFFLIB {
#include "tiffio.h"
}

typedef struct tiff_dir_attribs
{
  int dirno; 
  int64_t width;
  int64_t length;
  int compression;
  int tileWidth;
  int tileLength;
  int tileDepth;
  int64_t tileSize;
  int quality;
  int bitsPerSample;
  int bitCount;
  int samplesPerPixel;
  int imageDepth;
  int rowsPerStrip;
  int64_t stripSize;
  int photometric;
  int planarConfig;
  int64_t unpaddedScanlineBytes;
  int64_t paddedScanlineBytes;
  int64_t bitmapSize;
  int64_t totalTiles;
  double xAdj;
  double yAdj;
  std::string description;
} TiffDirAttribs;

typedef struct tiff_photometric_desc
{
  uint16_t photometric;
  char const* description;
} TiffPhotometricDesc;


class Tiff : public Image {
protected:
  TIFFLIB::TIFF* mTif;
  safeBmp mDestBmp;
  std::vector<TiffDirAttribs*> mDir;
  std::vector<TiffDirAttribs*> mDirBottomUp;
  int mDirCount;
  int mBaseDirno;
  int mTileWidth, mTileHeight;
  int mQuality;
  int mLastDir;
  int mLastLevel;
  bool mDebugMode;
  static TiffPhotometricDesc mPhotoStrings[11];
public:
  Tiff() { tiffClearAttribs(); }
	virtual ~Tiff() { tiffCleanup(); }
  void tiffClearAttribs();
  void tiffCleanup();
  void close() { tiffCleanup(); baseCleanup(); tiffClearAttribs(); baseClearAttribs(); }
  bool read(safeBmp* pDestBmp, int level, int64_t x, int64_t y, int64_t cx, int64_t cy, int64_t *pReadWidth, int64_t *pReadHeight);
  bool open(const std::string&, bool setGrayScale = false);
  bool createFile(const std::string& newFilename);
  bool setThumbNail();
  void setDebugMode(bool mode) { mDebugMode = mode; }
  bool setAttributes(int newSamplesPerPixel, int newBitsPerSample, int64_t newImageWidth, int64_t newImageHeight, int newTileWidth, int newTileHeight, int newTileDepth, int quality);
  bool setDescription(std::string& strAttributes, int baseWidth, int baseHeight);
  bool setDirectory(int dirno);
  bool setBottomUpDirectory(int dirno);
  int directorySize() { return mDirCount; }
  TiffDirAttribs* getDirectoryAttribs(int dirno);
  TiffDirAttribs* at(int dirno);
  bool writeDirectory();
  bool writeEncodedTile(BYTE* buff, int64_t x, int64_t y, int64_t z);
  bool writeImage(BYTE* buff);
	static bool testHeader(BYTE*, int);
};

class AttribSortPyramid
{
public:
  bool operator() (const TiffDirAttribs* attrib1, const TiffDirAttribs* attrib2) 
  {
    return ((double) attrib1->width* (double) attrib1->length) > ((double) attrib2->width*(double) attrib2->length);
  }
};

#endif
