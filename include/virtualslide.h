#ifndef __VIRTUALSLIDE_FILE_H
#define __VIRTUALSLIDE_FILE_H 

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <cstdint>
#include "safebmp.h"

class VirtualSlide {
protected:
  bool mValidObject;
  int64_t mBaseCols, mBaseRows;
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
  virtual bool open(const std::string& inputDir, int options, int debugLevel = 0) = 0; 
  virtual bool allocate(safeBmp* pBmp, int level, int64_t x, int64_t y, int64_t cols, int64_t rows, bool useActualCols = true);

  virtual bool read(safeBmp* pDestBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t cols, int64_t rows, int64_t *readCols, int64_t *readRows) = 0;

  virtual safeBmp* loadFullImage(int level, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::ofstream& logFile) = 0;

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
  long long getBaseCols();
  long long getBaseRows();
  std::string getCopyrightTxt();
  std::string getHumanDesc(); 
  virtual int64_t getActualCols(int level) = 0; 
  virtual int64_t getActualRows(int level) = 0; 
  virtual int64_t getLevelCols(int level) = 0;
  virtual int64_t getLevelRows(int level) = 0;
  virtual double getXAdj(int level) = 0; 
  virtual double getYAdj(int level) = 0; 
  virtual int64_t getTotalTiles(int level) = 0; 
  virtual void setXYSwitched(int orientation);
};

#endif
