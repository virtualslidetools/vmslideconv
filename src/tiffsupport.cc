/**************************************************************************
Initial author: Paul F. Richards (paulrichards321@gmail.com) 2016-2017 
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
#include <new>
#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include "imagesupport.h"
#include "tiffsupport.h"
#include "jp2kdecode.h"
#include "tilecachesupport.h"

extern TileCache tileCache;

TiffPhotometricDesc Tiff::mPhotoStrings[11] = 
{
  { PHOTOMETRIC_MINISWHITE, "Minimal is White" },
  { PHOTOMETRIC_MINISBLACK, "Minimal is Black" },
  { PHOTOMETRIC_MASK, "Hold Mask" },
  { PHOTOMETRIC_SEPARATED, "Color Separated" },
  { PHOTOMETRIC_YCBCR, "YCBCR/CCIR 601" },
  { PHOTOMETRIC_CIELAB, "CIE Lab" },
  { PHOTOMETRIC_ICCLAB, "ICC Lab" },
  { PHOTOMETRIC_ITULAB, "ITU Lab" },
  { PHOTOMETRIC_LOGL, "CIE Log2(L)" },
  { PHOTOMETRIC_LOGLUV, "CIE Log2(L) (u', v')" },
  { 0, NULL }
};


#define CVT(x)  (((x) * 255L) / ((1L<<16)-1))


void strReplaceAll(std::string& str, const std::string& from, const std::string& to) 
{
  if(from.empty())
    return;
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) 
  {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}


bool Tiff::testHeader(BYTE* header, int size)
{
  if (size < 4) return false;
  if (header[0] == 'I' && header[1] == 'I' && header[2] == 42 && header[3] == 0)
    return true;
  else if (header[0] == 'M' && header[1] == 'M' && header[2] == 0 && header[3] == 42)
    return true;
  else
    return false;
}


void Tiff::tiffClearAttribs()
{
  mTif = 0; 
  safeBmpClear(&mDestBmp);
  mDirCount = 0;
  mBaseDirno = 0; 
  mTileCols = 0; 
  mTileRows = 0; 
  mLastDir = 0;
  mLastLevel = -1;
}


void Tiff::tiffCleanup()
{
  if (mTif) 
  { 
    TIFFLIB::TIFFClose(mTif); 
    mTif=0; 
  } 
  for (size_t i = 0; i < mDir.size(); i++)
  {
    if (mDir[i])
    {
      delete mDir[i];
      mDir[i] = 0;
    }
  }
  mDir.clear();
  mDirBottomUp.clear();
}


TiffDirAttribs* Tiff::getDirectoryAttribs(int dirno) 
{ 
  return (dirno >= 0 && dirno < mDirCount) ? mDirBottomUp[dirno] : 0;
}


TiffDirAttribs* Tiff::at(int dirno) 
{ 
  return (dirno >= 0 && dirno < mDirCount) ? mDirBottomUp[dirno] : 0; 
}


bool Tiff::setDirectory(int dirno) 
{ 
  bool success = false;
  if (mLastDir != dirno)
  {
    if (mDirCount > 0 && dirno >= 0 && dirno < mDirCount) 
    { 
      success = TIFFLIB::TIFFSetDirectory(mTif, (uint16_t) dirno) == 1 ? true : false; 
      if (success)
      {
        mLastDir = dirno;
        mLastLevel = -1;
      }
    }
  } 
  else
  {
    success = true;
  }
  return success; 
}


bool Tiff::setBottomUpDirectory(int level)
{
  bool success = false;
  if (mValidObject)
  {
    if (mLastLevel != level)
    {
      if (mDirCount > 0 && level >= 0 && level < mDirCount) 
      { 
        int dirno = mDirBottomUp[level]->dirno;
        success = TIFFLIB::TIFFSetDirectory(mTif, (uint16_t) dirno) == 1 ? true : false; 
        if (success)
        {
          mLastDir = dirno;
          mLastLevel = level;
        }
      }
    } 
    else
    {
      success = true;
    }
  }
  return success; 
}


bool Tiff::open(const std::string& existingFileName, bool setGrayScale)
{
  std::string newLineStr = "\x0A";
  std::string newLineStr2 = "[0x0A]";
  std::string carriageStr = "\x0D";
  std::string carriageStr2 = "[0x0D]";

  (void) setGrayScale;
  if (mValidObject)
  {
    tiffCleanup();
    baseCleanup();
    tiffClearAttribs();
    baseClearAttribs();
  }
  setFileName(existingFileName);
  
  try 
  {
    mTif = TIFFLIB::TIFFOpen(mFileName.c_str(), "r");
    if (!mTif) {
      mErrMsg << "Error opening '" << mFileName << "': ";
      std::cerr << mErrMsg.str();
      throw std::runtime_error(mErrMsg.str());
    }
    do
    {
      uint32_t tifImageCols = 0, tifImageLength = 0;
      uint32_t tileCols = 0, tileLength = 0;
      uint16_t bitsPerSample = 0, samplesPerPixel = 0;
      uint32_t rowsPerStrip = 0;
      uint16_t photometric = 0;
      uint16_t planarConfig = 0;
      uint16_t compression = 0;
      uint32_t tileDepth = 0;
      uint32_t tileSize = 0;
      uint32_t imageDepth = 0;
      uint32_t xRes=0, yRes=0;
      uint32_t xPos=0, yPos=0;
      TIFFLIB::tsize_t stripSize = 0;
      const char * description = 0;
      std::string description2;
      unsigned int unpaddedScanlineBytes, paddedScanlineBytes;

      TiffDirAttribs* attribs = new TiffDirAttribs;

      TIFFGetField(mTif, TIFFTAG_COMPRESSION, &compression);
      TIFFGetField(mTif, TIFFTAG_IMAGEWIDTH, &tifImageCols);
      TIFFGetField(mTif, TIFFTAG_IMAGELENGTH, &tifImageLength);  
      TIFFGetField(mTif, TIFFTAG_BITSPERSAMPLE, &bitsPerSample);
      TIFFGetField(mTif, TIFFTAG_SAMPLESPERPIXEL, &samplesPerPixel);
      TIFFGetField(mTif, TIFFTAG_ROWSPERSTRIP, &rowsPerStrip);  
      TIFFGetField(mTif, TIFFTAG_PHOTOMETRIC, &photometric);
      TIFFGetField(mTif, TIFFTAG_TILEWIDTH, &tileCols);
      TIFFGetField(mTif, TIFFTAG_TILELENGTH, &tileLength);
      TIFFGetField(mTif, TIFFTAG_TILEDEPTH, &tileDepth);
      TIFFGetField(mTif, TIFFTAG_PLANARCONFIG, &planarConfig);
      TIFFGetField(mTif, TIFFTAG_IMAGEDEPTH, &imageDepth);
      TIFFGetField(mTif, TIFFTAG_IMAGEDESCRIPTION, &description);
      TIFFGetField(mTif, TIFFTAG_XRESOLUTION, &xRes);
      TIFFGetField(mTif, TIFFTAG_YRESOLUTION, &yRes);
      TIFFGetField(mTif, TIFFTAG_XPOSITION, &xPos);
      TIFFGetField(mTif, TIFFTAG_YPOSITION, &yPos);
      tileSize = (uint32_t) TIFFTileSize(mTif);
      unpaddedScanlineBytes = tifImageCols * samplesPerPixel; 
      paddedScanlineBytes = unpaddedScanlineBytes;
      
      // each row of the bitmap must be aligned at dword boundaries
      while ((paddedScanlineBytes & 3) != 0) paddedScanlineBytes++;
      attribs->dirno = mDirCount;
      attribs->compression = compression;
      attribs->cols = tifImageCols;
      attribs->length = tifImageLength;
      attribs->tileCols = tileCols;
      attribs->tileLength = tileLength;
      attribs->tileDepth = tileDepth;
      attribs->tileSize = tileSize;
      attribs->rowsPerStrip = rowsPerStrip;
      attribs->samplesPerPixel = samplesPerPixel;
      attribs->imageDepth = imageDepth;
      attribs->bitsPerSample = bitsPerSample;
      attribs->bitCount = bitsPerSample * samplesPerPixel;
      attribs->photometric = photometric;
      attribs->planarConfig = planarConfig;
      attribs->unpaddedScanlineBytes = unpaddedScanlineBytes;
      attribs->paddedScanlineBytes = paddedScanlineBytes;
      attribs->bitmapSize = paddedScanlineBytes * tifImageLength;
      if (planarConfig == 2)
      {
        stripSize = TIFFStripSize(mTif);
      }
      attribs->stripSize = stripSize;
      attribs->totalTiles = 0;
      if (tifImageCols > 0 && tifImageLength > 0 && tileCols > 0 && tileLength > 0)
      {
        attribs->totalTiles = (int) ceil(tifImageCols / tileCols) * (int) ceil(tifImageLength / tileLength);
      }  
      attribs->quality = 0;
      if (description)
      {
        description2 = description;
        attribs->description = description;
      }
      else
      {
        description2="";
      }
      strReplaceAll(description2, newLineStr, newLineStr2);
      strReplaceAll(description2, carriageStr, carriageStr2);
      if (mDebugMode)
      {
        std::cout << "------------------------------------------------------------";
        std::cout << std::endl;
        std::cout << "TiffDirectory=" << mDirCount;
        std::cout << " ImageCols=" << tifImageCols;
        std::cout << " ImageLength=" << tifImageLength;
        std::cout << std::endl;
        std::cout << "BitsPerSample=" << bitsPerSample;
        std::cout << " SamplesPerPixel=" << samplesPerPixel;
        std::cout << " ImageDepth=" << imageDepth;
        std::cout << std::endl;
        std::cout << "TileCols=" << tileCols;
        std::cout << " TileLength=" << tileLength;
        std::cout << std::endl;
        std::cout << "PlanarConfig=" << planarConfig;
        std::cout << " Photometric=" << photometric;
        std::cout << std::endl;
        std::cout << "TileDepth=" << tileDepth;
        std::cout << " TileSize=" << tileSize;
        std::cout << " RowsPerStrip=" << rowsPerStrip;
        std::cout << std::endl;
        std::cout << "xResolution=" << xRes;
        std::cout << " yResolution=" << yRes;
        std::cout << std::endl;
        std::cout << "xPosition=" << xPos;
        std::cout << " yPosition=" << yPos;
        std::cout << std::endl;
        std::cout << "Image Description: '" << description2 << "'";
        std::cout << std::endl;
      }
      mDir.push_back(attribs);
      mDirBottomUp.push_back(attribs);
      mDirCount++;
    } 
    while (TIFFReadDirectory(mTif));
      
    if (mDirCount > 0) 
    {
      std::sort(mDirBottomUp.begin(), mDirBottomUp.end(), AttribSortPyramid());
      mBaseDirno = mDirBottomUp[0]->dirno;
      mActualCols = mDirBottomUp[0]->cols;
      mActualRows = mDirBottomUp[0]->length;
      mBitCount = mDirBottomUp[0]->bitCount;
      mBitmapSize = mDirBottomUp[0]->bitmapSize;
      if (TIFFSetDirectory(mTif, (uint16_t) mBaseDirno) == 1)
      {
        mLastDir = mBaseDirno;
        mLastLevel = 0; 
        mValidObject = true;
      }
      if (mDebugMode)
      {
        std::cout << "------------------------------------------------------------";
        std::cout << std::endl;
        std::cout << "Levels Sorted:";
        std::cout << std::endl;
        std::cout << "------------------------------------------------------------";
        std::cout << std::endl;
        std::cout << "TotalLevels=" << mDirCount; 
        std::cout << " BaseCols=" << mActualCols;
        std::cout << " BaseRows=" << mActualRows;
        std::cout << std::endl;
        for (int i = 0; i < mDirCount; i++)
        {
          std::cout << "Level=" << i;
          std::cout << " Cols=" << mDirBottomUp[i]->cols;
          std::cout << " Rows=" << mDirBottomUp[i]->length;
          std::cout << std::endl;
          std::string description2 = mDirBottomUp[i]->description;
          strReplaceAll(description2, newLineStr, newLineStr2);
          strReplaceAll(description2, carriageStr, carriageStr2);
          std::cout << "Description='" << description2 << "'";
          std::cout << std::endl;
        }
      }
    }  
  } catch (std::bad_alloc &e) {
    (void) e;
    if (mTif) TIFFClose(mTif);
    mTif = 0;
    mErrMsg << "Insufficient memory to decompress '" << mFileName;
    mErrMsg << "' into memory";
    throw std::runtime_error(mErrMsg.str());
  } catch (std::runtime_error &e) {
    (void) e;
    if (mTif) TIFFClose(mTif);
    mTif = 0;
    return false;
  }
  return (mDirCount > 0);
}


bool Tiff::read(safeBmp* pDestBmp, int level, int64_t x, int64_t y, int64_t cx, int64_t cy, int64_t *pReadCols, int64_t *pReadRows)
{
  safeBmp srcTile;
  bool success = false;
  *pReadCols = 0;
  *pReadRows = 0;
  if (setBottomUpDirectory(level)==false)
  {
    return false;
  }
  TiffDirAttribs* dirPtr = mDir[mLastDir];

  int64_t imageCols = dirPtr->cols;
  int64_t imageLength = dirPtr->length;
  
  if (cx < 1 || cy < 1) return false;
  if (x < 0 || y < 0) return false;
  if (x >= imageCols || y >= imageLength) return false;
  int64_t cx2 = cx;
  int64_t cy2 = cy;
  if (x+cx > imageCols)
  {
    cx2 = imageCols - x;
  }
  if (y+cy > imageLength)
  {
    cy2 = imageLength - y;
  }
  switch (dirPtr->photometric) 
  {
    case PHOTOMETRIC_RGB:
    {
      if (dirPtr->samplesPerPixel != 3 || dirPtr->bitsPerSample != 8) {
        mErrMsg << "Error decompressing '" << mFileName << "': ";
        mErrMsg << "Unsupported bit depth in tiff file. Samples per pixel=" << dirPtr->samplesPerPixel << " bits per sample=" << dirPtr->bitsPerSample;
        throw std::runtime_error(mErrMsg.str());
      }
      break;
    }  
    case PHOTOMETRIC_PALETTE: 
    { // color palette
      mErrMsg << "Error decompressing '" << mFileName << "': ";
      mErrMsg << "Unsupported palette in tiff file.";
      throw std::runtime_error(mErrMsg.str());
      break;
    }
    default:
    {
      int i = 0;
      while (mPhotoStrings[i].description != NULL &&
             mPhotoStrings[i].photometric != dirPtr->photometric)
      {
        i++;
      }
      std::ostringstream description;
      if (mPhotoStrings[i].description == NULL)
          description << dirPtr->photometric;
      else
          description << mPhotoStrings[i].description;
      mErrMsg << "Error decompressing '" << mFileName << "': ";
      mErrMsg << "Unsupported Tiff file format.\n";
      mErrMsg << "Tiff files that are encoded using a photometric value of " 
          << description.str() << " are currently unsupported.";
      throw std::runtime_error(mErrMsg.str());
    }
  }
  int compression = dirPtr->compression;
  int tileCols = dirPtr->tileCols;
  int tileLength = dirPtr->tileLength;
  jp2k_colorspace compressionType = JP2K_RGB;

  if (dirPtr->planarConfig == 2) 
  {
    tileLength = (int) dirPtr->rowsPerStrip;
    tileCols = (int) imageCols;
  }
  else
  {
    if (tileCols <= 0) tileCols = 256;
    if (tileLength <= 0) tileLength = 256;
  }
  BYTE* compressedTileBmp = NULL;
  int uncompressedTileSize = tileCols * 3 * tileLength;
  int64_t tilesPerRow = (int64_t) ceil((double)imageCols/(double)tileCols);
  int64_t startx = (x / tileCols) * tileCols;
  int64_t starty = (y / tileLength) * tileLength;
  int64_t endx = (int64_t) ceil((double)(x+cx2) / (double)tileCols) * tileCols;
  int64_t endy = (int64_t) ceil((double)(y+cy2) / (double)tileLength) * tileLength;

  if (compression == 33003 || compression == 33005)
  {
    if (compression == 33003) compressionType = JP2K_YCBCR;
    compressedTileBmp = new BYTE[uncompressedTileSize];
  }
  for (int64_t srcy = starty; srcy < endy; srcy += tileLength) 
  {
    int64_t desty = srcy - y;
    int64_t ycopy = tileLength;
    int64_t srcy2 = 0;
    if (y > srcy) 
    {
      ycopy += desty;
      srcy2 = y - srcy;
      desty = 0;
    }  
    for (int64_t srcx = startx; srcx < endx; srcx += tileCols) 
    {
      int64_t destx = srcx - x;
      int64_t xcopy = tileCols;
      int64_t srcx2 = 0;
      if (x > srcx)
      {
        xcopy += destx;
        srcx2 = x - srcx;
        destx = 0;
      }  
      int64_t bytesRead = 0;
      int64_t tileNum = ((srcy / tileLength) * tilesPerRow) + (srcx / tileCols);
      BYTE* pTileBmp = tileCache.find(this, level, tileNum, &bytesRead);
      if (pTileBmp == 0)
      {
        if (compression == 33003 || compression == 33005)
        {
          memset(compressedTileBmp, 255, uncompressedTileSize);
          bytesRead = (int64_t) TIFFReadRawTile(mTif, (TIFFLIB::ttile_t) tileNum, compressedTileBmp, uncompressedTileSize);
        }
        else
        {
          pTileBmp = new BYTE[uncompressedTileSize];
          memset(pTileBmp, 255, uncompressedTileSize);
          if (dirPtr->planarConfig == 2)
          {
            TIFFLIB::tstrip_t stripNum = TIFFComputeStrip(mTif, (uint32_t) srcy, 0);
            bytesRead = (int64_t) TIFFReadEncodedStrip(mTif, stripNum, pTileBmp, -1); 
          }
          else
          {
            bytesRead = (int64_t) TIFFReadTile(mTif, pTileBmp, (uint32_t) srcx, (uint32_t) srcy, 0, 0);
          }
        }
        if (bytesRead <= 0)
        {
          std::cerr << "Warning: TIFF Read Bytes Read is <= 0." << std::endl;;
        }
        else if (bytesRead > uncompressedTileSize)
        {
          mErrMsg << "Fatal Error: TIFF Read Bytes Read > Uncompressed Tile Size!" << std::endl;
          throw std::runtime_error(mErrMsg.str());
        }
        if (compression == 33003 || compression == 33005)
        {
          pTileBmp = new BYTE[uncompressedTileSize];
          memset(pTileBmp, 255, uncompressedTileSize);
          if (bytesRead > 0 && jp2k_decode(pTileBmp, tileCols, tileLength, compressedTileBmp, (int32_t) bytesRead, compressionType)==false)
          {
            std::cerr << "Warning: JPEG2000 decode failed on Tiff Level: " << level << " Tile: " << tileNum << "! Resulting Tile will be all white." << std::endl;
            bytesRead = 0;
          }
        }
        tileCache.add(this, level, tileNum, uncompressedTileSize, pTileBmp);
      }
      if (bytesRead > 0) 
      {
        safeBmpInit(&srcTile, pTileBmp, tileCols, tileLength); 
        safeBmpCpy(pDestBmp, destx, desty, &srcTile, srcx2, srcy2, xcopy, ycopy);
        success = true;
      }
    }  
  } 
  if (compressedTileBmp)
  {
    delete[] compressedTileBmp;
    compressedTileBmp = 0;
  }
  if (success)
  {
    *pReadCols = cx;
    *pReadRows = cy;
  }
  return success;
}


bool Tiff::createFile(const std::string& newFileName)
{
  if (mValidObject)
  {
    tiffCleanup();
    baseCleanup();
    tiffClearAttribs();
    baseClearAttribs();
  }
  setFileName(newFileName);
    
  try {
    mTif = TIFFLIB::TIFFOpen(mFileName.c_str(), "wb8");
    if (!mTif) {
      mErrMsg << "Error opening '" << mFileName << "': ";
      throw std::runtime_error(mErrMsg.str());
    }
  } 
  catch (std::runtime_error &e) 
  {
    (void) e;
    if (mTif) TIFFClose(mTif);
    mTif = 0;
    return false;
  }
  return true;
}


bool Tiff::setAttributes(int newSamplesPerPixel, int newBitsPerSample, int64_t newImageCols, int64_t newImageRows, int newTileCols, int newTileRows, int newTileDepth, int quality)
{
  mActualCols = newImageCols;
  mActualRows = newImageRows;
  mTileCols = newTileCols;
  mTileRows = newTileRows;
  mBitCount = newBitsPerSample;
  mSamplesPerPixel = newSamplesPerPixel;
  mQuality = quality;
  uint32_t u32TifImageCols = (uint32_t) newImageCols;
  uint32_t u32TifImageLength = (uint32_t) newImageRows;
  uint32_t u32TileCols = (uint32_t) newTileCols;
  uint32_t u32TileLength = (uint32_t) newTileRows;
  uint32_t u32TileDepth = (uint32_t) newTileDepth;
  uint16_t u16BitsPerSample = (uint16_t) newBitsPerSample;
  uint16_t u16SamplesPerPixel = (uint16_t) newSamplesPerPixel;
  uint16_t photometric = PHOTOMETRIC_RGB;
  uint16_t planarConfig = 1;

  if (mTif)
  {
    try 
    {
      TIFFSetField(mTif, TIFFTAG_IMAGEWIDTH, u32TifImageCols);
      TIFFSetField(mTif, TIFFTAG_IMAGELENGTH, u32TifImageLength);  
      TIFFSetField(mTif, TIFFTAG_BITSPERSAMPLE, u16BitsPerSample);
      TIFFSetField(mTif, TIFFTAG_SAMPLESPERPIXEL, u16SamplesPerPixel);
      TIFFSetField(mTif, TIFFTAG_PHOTOMETRIC, photometric);
      if (mTileCols > 0 && mTileRows > 0)
      {
        TIFFSetField(mTif, TIFFTAG_TILEWIDTH, u32TileCols);
        TIFFSetField(mTif, TIFFTAG_TILELENGTH, u32TileLength);
        TIFFSetField(mTif, TIFFTAG_TILEDEPTH, u32TileDepth);
      }
      TIFFSetField(mTif, TIFFTAG_PLANARCONFIG, planarConfig);
      TIFFSetField(mTif, TIFFTAG_COMPRESSION, COMPRESSION_JPEG);
      TIFFSetField(mTif, TIFFTAG_JPEGQUALITY, quality);    
      TIFFSetField(mTif, TIFFTAG_JPEGCOLORMODE, JPEGCOLORMODE_RGB);
    }
    catch (std::bad_alloc &e) 
    {
      (void) e;
      if (mTif) TIFFClose(mTif);
      mTif = 0;
      mErrMsg << "Insufficient memory to decompress '" << mFileName;
      mErrMsg << "' into memory";
      throw std::runtime_error(mErrMsg.str());
    } 
    catch (std::runtime_error &e) 
    {
      (void) e;
      if (mTif) TIFFClose(mTif);
      mTif = 0;
      return false;
    }
  }
  else
  {
    return false;
  }
  return true;
}


bool Tiff::writeEncodedTile(BYTE* buff, int64_t x, int64_t y, int64_t z)
{
  bool success = false;
  if (mTif)
  {
    TIFFLIB::ttile_t tile = TIFFComputeTile(mTif, (uint32_t) x, (uint32_t) y, (uint32_t) z, 0);
    TIFFLIB::tsize_t saved = TIFFWriteEncodedTile(mTif, tile, (TIFFLIB::tdata_t) buff, (TIFFLIB::tsize_t) (mTileCols*mTileRows*mSamplesPerPixel));
    if (saved==(mTileCols*mTileRows*mSamplesPerPixel))
    {
      success= true;
    }
  }
  return success;
}


bool Tiff::writeImage(BYTE* buff)
{
  bool success = false;
  if (mTif)
  {
    TIFFLIB::tsize_t stripSize = mActualCols*mActualRows*mSamplesPerPixel;
    TIFFSetField(mTif, TIFFTAG_ROWSPERSTRIP, mActualRows);
    TIFFLIB::tsize_t wroteSize = TIFFWriteEncodedStrip(mTif, 0, (void*) buff, stripSize);
    if (stripSize == wroteSize)
    {
      success=true;
    }
  }
  return success;
}


bool Tiff::writeDirectory()
{
  bool success = false;
  if (mTif)
  {
    success = (TIFFWriteDirectory(mTif) == 1) ? true : false;
  }
  return success;
}


bool Tiff::setDescription(std::string& strAttributes, int baseCols, int baseRows)
{
  std::ostringstream oss;
  int retval=0;
  if (mTif)
  {
    oss << "Aperio Image" << "\r\n";
    oss << baseCols << "x" << baseRows << " ";
    if (mTileCols > 0 && mTileRows > 0)
    {
      oss << "(" << mTileCols << "x" << mTileRows << ") ";
    }
    oss << "-> " << mActualCols << "x" << mActualRows;
    if (mTileCols > 0 && mTileRows > 0)
    {
      oss << " JPEG/RGB Q=" << mQuality;
    }
    else
    {
      oss << " - ";
    }
    oss << strAttributes;
    std::string attr = oss.str();
    retval=TIFFSetField(mTif, TIFFTAG_IMAGEDESCRIPTION, attr.c_str());
  }
  return (retval == 1 ? true : false);
}


bool Tiff::setThumbNail()
{
  if (mTif)
  {
    TIFFSetField(mTif, TIFFTAG_SUBFILETYPE, FILETYPE_REDUCEDIMAGE);
    return true;
  }
  return false;
}
