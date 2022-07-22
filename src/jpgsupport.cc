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
#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <new>
#include <cstdio>
#include <memory>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <iostream>
#include "jpgsupport.h"


void Jpg::jpgClearAttribs()
{
  safeBmpClear(&mFullSrc);
}


void Jpg::jpgCleanup()
{
  safeBmpFree(&mFullSrc);
}


bool Jpg::testHeader(BYTE* header, int)
{
  if (header[0] == 0xFF && header[1] == 0xD8)
    return true;
  else
    return false;
}


bool Jpg::open(const std::string& newFileName, bool setGrayScale)
{
  std::string errStr;

  (void) setGrayScale;
  if (mValidObject)
  {
    jpgCleanup();
    baseCleanup();
    jpgClearAttribs();
    baseClearAttribs();
  }
  mFileName = newFileName;
  if (safeJpgRead(&mFullSrc, newFileName, 255, &errStr) == 0)
  {
    mSamplesPerPixel = 3;
    mActualCols = mFullSrc.cols;
    mActualRows = mFullSrc.rows;
    mBitCount = mSamplesPerPixel * 8;
    mValidObject=true;       
  }
  else
  {
    mErrMsg << errStr;
    return false;
  }
  return true;
}


bool Jpg::read(safeBmp *pBmpDest, int64_t x, int64_t y, int64_t cols, int64_t rows)
{
  if (mValidObject==false) return false;
  mReadCols=0;
  mReadRows=0;
  mErrMsg.str("");
  if (x<0 || y<0 || x>mActualCols || y>mActualRows || rows<=0 || cols<=0) 
  {
    std::cerr << "In jpeg::read parameters out of bounds." << std::endl;
    return false;
  }
  if (x+cols > mActualCols)
  {
    std::cerr << "In jpeg::read, cols truncated. Actual cols=" << mActualCols;
    std::cerr << " x=" << x << " cols=" << cols << std::endl;
    cols = mActualCols - x;
  }
  if (y+rows > mActualRows)
  {
    std::cerr << "In jpeg::read, rows truncated. Actual rows=" << mActualRows;
    std::cerr << " y=" << y << " rows=" << rows << std::endl;
    rows = mActualRows - y;
  }
  safeBmpCpy(pBmpDest, 0, 0, &mFullSrc, x, y, cols, rows);
  mReadCols = cols;
  mReadRows = rows;
  return true;
}
