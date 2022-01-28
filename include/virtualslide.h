#ifndef __VIRTUALSLIDE_FILE_H
#define __VIRTUALSLIDE_FILE_H 

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>
#include "safebmp.h"

#ifndef USE_MAGICK
#include "opencv2/core.hpp"
#include "opencv2/features2d.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#else
namespace Magick
{
#include <MagickCore/MagickCore.h>
#include <MagickWand/MagickWand.h>
}
const char* getMagickCoreCoderPath();
const char* getMagickCoreFilterPath();
const char* getMagickCoreLibraryPath();
#endif 

class VirtualSlide {
protected:
  bool mValidObject;
  int64_t mBaseWidth, mBaseHeight;
  uint8_t mBkgColor;
  int mTotalZLevels, mTotalTopZLevels, mTotalBottomZLevels;
  bool mGrayScale;
  double mMag;
  std::string mMagStr;
  bool mMagFound;
  int64_t mBestXOffset, mBestYOffset;
  bool mOptBorder;
  int mOrientation;
  bool mXYSwitched;
  int mOptDebug;
  std::string mCopyrightTxt;
  std::string mHumanDesc;
public:
  VirtualSlide() { baseClearAttribs(); }
  virtual ~VirtualSlide() { baseCleanup(); }
  bool isValidObject();
  void baseClearAttribs();
  void baseCleanup() { mValidObject = false; }
  virtual void close() = 0;
  virtual bool open(const std::string& inputDir, int options, int orientation, int debugLevel = 0, int64_t bestXOffset = -1, int64_t bestYOffset = -1, safeBmp **pImageL2 = NULL) = 0; 
  virtual bool allocate(safeBmp* pBmp, int level, int64_t x, int64_t y, int64_t width, int64_t height, bool useActualWidth = true);

  virtual bool read(safeBmp* pDestBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t width, int64_t height, int64_t *readWidth, int64_t *readHeight) = 0;

  #ifndef USE_MAGICK
  virtual bool findXYOffset(int lowerLevel, int higherLevel, int64_t *bestXOffset0, int64_t *bestYOffset0, int64_t *bestXOffset1, int64_t *bestYOffset1, int optUseCustomOffset, int debugLevel, std::fstream& logFile) = 0;
  #endif

  #ifndef USE_MAGICK
  virtual bool loadFullImage(int level, safeBmp **ptpFullImage, cv::Mat **ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile) = 0;
  #else
  virtual bool loadFullImage(int level, safeBmp **ptpImageL2, void **ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile) = 0;
  #endif
  
  virtual bool checkLevel(int level) = 0;
  virtual bool checkZLevel(int level, int direction, int zLevel) = 0; 
  virtual int getTotalLevels() = 0;
  virtual int getTotalZLevels() = 0; 
  virtual int getTotalBottomZLevels() = 0; 
  virtual int getTotalTopZLevels() = 0;
  virtual int getQuality(int level) = 0; 
  double getMag();
  std::string getMagStr();
  bool getMagFound();
  long long getBaseWidth();
  long long getBaseHeight();
  std::string getCopyrightTxt();
  std::string getHumanDesc(); 
  virtual int64_t getActualWidth(int level) = 0; 
  virtual int64_t getActualHeight(int level) = 0; 
  virtual int64_t getLevelWidth(int level) = 0;
  virtual int64_t getLevelHeight(int level) = 0;
  virtual double getXAdj(int level) = 0; 
  virtual double getYAdj(int level) = 0; 
  virtual int64_t getTotalTiles(int level) = 0; 
  virtual bool setOrientation(int orientation, std::fstream& logFile) = 0;
  virtual void setXYSwitched(int orientation);
};

#endif
