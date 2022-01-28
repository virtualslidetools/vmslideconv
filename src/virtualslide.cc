#include "virtualslide.h"

bool VirtualSlide::isValidObject()
{
  return mValidObject;
}


void VirtualSlide::baseClearAttribs()
{
  mValidObject = false;
  mBaseWidth = 0;
  mBaseHeight = 0;
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

long long VirtualSlide::getBaseWidth() 
{ 
  return (mValidObject == true ? mBaseWidth : 0); 
}

long long VirtualSlide::getBaseHeight() 
{ 
  return (mValidObject == true ? mBaseHeight : 0); 
}

std::string VirtualSlide::getHumanDesc()
{ 
  return mHumanDesc; 
}

std::string VirtualSlide::getCopyrightTxt()
{ 
  return mCopyrightTxt; 
}

bool VirtualSlide::allocate(safeBmp* pBmp, int level, int64_t x, int64_t y, int64_t width, int64_t height, bool useActualWidth)
{
  if (mValidObject == false || level<0 || level > getTotalLevels() || checkLevel(level) == false)
  {
    return false;
  }
  int64_t fullWidth = 0;
  int64_t fullHeight = 0;
  if (useActualWidth)
  {
    fullWidth = getActualWidth(level);
    fullHeight = getActualHeight(level);
  }
  else
  {
    fullWidth = getLevelWidth(level);
    fullHeight = getLevelHeight(level);
  }
  if (x > fullWidth || y > fullHeight)
  {
    std::cerr << "x or y out of bounds: x=" << x << " y=" << y;
    return false;
  }
  if (width <= 0 || height <= 0)
  {
    std::cerr << "width or height out of bounds: width=" << width << " height=" << height;
    return false;
  }
  int samplesPerPixel = 3;
  /*
  if (setGrayScale || mGrayScale)
  {
    samplesPerPixel = 1;
  }
  */
  int64_t maxWidth = width;
  int64_t maxHeight = height;
  if (x + width > fullWidth)
  {
    maxWidth = fullWidth - x;
  }
  if (y + height > fullHeight)
  {
    maxHeight = fullHeight - y;
  }

  int64_t bmpSize = maxWidth * maxHeight * samplesPerPixel;
  if (bmpSize > 512 * 1024 * 1024)
  {
    std::cout << "allocating " << (bmpSize / (1024 * 1024)) << " megabytes in memory." << std::endl;
  }
  BYTE* data = safeBmpAlloc2(pBmp, maxWidth, maxHeight);
  return (data ? true : false);
}


void VirtualSlide::setXYSwitched(int orientation)
{
  mXYSwitched = ((orientation == 90 || orientation == -90 || orientation == 270) ? true : false);
}

