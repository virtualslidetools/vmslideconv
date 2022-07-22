#include "virtualslide.h"

bool VirtualSlide::isValidObject()
{
  return mValidObject;
}


void VirtualSlide::baseClearAttribs()
{
  mValidObject = false;
  mBaseCols = 0;
  mBaseRows = 0;
  mBkgColor = 255;
  mTotalZLevels = 0;
  mTotalTopZLevels = 0;
  mTotalBottomZLevels = 0;
  mGrayScale = false;
  mMag = 0;
  mMagStr = "";
  mMagFound = false;
  mBestXOffset = 0;
  mBestYOffset = 0;
  mOptBorder = false;
  mOrientation = 0;
  mXYSwitched = false;
  mOptDebug = 0;
  mCopyrightTxt = "";
  mHumanDesc = "";
}


double VirtualSlide::getMag() 
{ 
  return mMag; 
}


std::string VirtualSlide::getMagStr()
{
  return mMagStr;
}


bool VirtualSlide::getMagFound()
{
  return mMagFound;
}

long long VirtualSlide::getBaseCols() 
{ 
  return (mValidObject == true ? mBaseCols : 0); 
}

long long VirtualSlide::getBaseRows() 
{ 
  return (mValidObject == true ? mBaseRows : 0); 
}

std::string VirtualSlide::getHumanDesc()
{ 
  return mHumanDesc; 
}

std::string VirtualSlide::getCopyrightTxt()
{ 
  return mCopyrightTxt; 
}

bool VirtualSlide::allocate(safeBmp* pBmp, int level, int64_t x, int64_t y, int64_t cols, int64_t rows, bool useActualCols)
{
  if (mValidObject == false || level<0 || level > getTotalLevels() || checkLevel(level) == false)
  {
    return false;
  }
  int64_t fullCols = 0;
  int64_t fullRows = 0;
  if (useActualCols)
  {
    fullCols = getActualCols(level);
    fullRows = getActualRows(level);
  }
  else
  {
    fullCols = getLevelCols(level);
    fullRows = getLevelRows(level);
  }
  if (x > fullCols || y > fullRows)
  {
    std::cerr << "x or y out of bounds: x=" << x << " y=" << y;
    return false;
  }
  if (cols <= 0 || rows <= 0)
  {
    std::cerr << "cols or rows out of bounds: cols=" << cols << " rows=" << rows;
    return false;
  }
  int samplesPerPixel = 3;
  int64_t maxCols = cols;
  int64_t maxRows = rows;
  if (x + cols > fullCols)
  {
    maxCols = fullCols - x;
  }
  if (y + rows > fullRows)
  {
    maxRows = fullRows - y;
  }

  int64_t bmpSize = maxCols * maxRows * samplesPerPixel;
  if (bmpSize > 512 * 1024 * 1024)
  {
    std::cout << "allocating " << (bmpSize / (1024 * 1024)) << " megabytes in memory." << std::endl;
  }
  BYTE* data = safeBmpAlloc2(pBmp, maxCols, maxRows);
  return (data ? true : false);
}


void VirtualSlide::setXYSwitched(int orientation)
{
  mXYSwitched = ((orientation == 90 || orientation == -90 || orientation == 270) ? true : false);
}

