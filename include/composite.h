/*******************************************************************************
Copyright (c) 2005-2016, Paul F. Richards

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
#ifndef __COMPOSITE_FILE_H
#define __COMPOSITE_FILE_H

#include "imagesupport.h"
#include "tiffsupport.h"
#include "virtualslide.h"
#include "blendbkgd.h"

inline char separator()
{
#if defined _WIN32 || defined __CYGWIN__ || defined _WIN64
  return '\\';
#else
  return '/';
#endif
}


struct Pt
{
  int64_t x, y;
};

class JpgXY
{
public:
  int64_t mxPixel, myPixel;
};

class JpgXYSortForX
{
public:
  bool operator() (const JpgXY& jpgXY1, const JpgXY& jpgXY2);
};

class JpgFileXY
{
public:
  int64_t mx, my;
  int64_t mxPixel, myPixel;
  int64_t mxSortedIndex;
  std::string mFileName[2][4];
  std::string mBaseFileName;
  std::vector<Pt> border;
  bool operator < (const JpgFileXY& jpgFile) const;
  bool mzStack[2][4];
};

class JpgFileXYSortForX
{
public:
  bool operator() (const JpgFileXY& jpgFile1, const JpgFileXY& jpgFile2);
};

class JpgFileXYSortForY
{
public:
  bool operator() (const JpgFileXY& jpgFile1, const JpgFileXY& jpgFile2);
};


class IniConf 
{
public:
  const char* mName;
  bool mFound;
  double mxAdj, myAdj;
  int64_t mTotalWidth, mTotalHeight;
  bool mIsPreviewSlide;
public:
  IniConf();
};


class JpgIniConf : public IniConf 
{
public:
  int64_t mPixelWidth, mPixelHeight;
  int64_t mDetailedWidth, mDetailedHeight;
  int64_t mOrgDetailedWidth, mOrgDetailedHeight;
  int64_t mTotalTiles;
  int64_t mxMin, mxMax, myMin, myMax;
  int64_t mxDiffMin, myDiffMin;
  int64_t mxStepSize, myStepSize;
  int64_t mxAxis, myAxis;
  bool mxKnowStepSize, myKnowStepSize, mKnowStepSizes;
  std::vector<JpgFileXY> mxyArr;
  std::vector<JpgXY> mxSortedArr;
  bool mzStackExists[2][4];
  int mQuality;
public:
  JpgIniConf();
};


class JpgFileXYSortForXAdj
{
public:
  bool operator() (const IniConf *iniConf1, const IniConf *iniConf2);
};

bool drawXHighlight(BYTE *pBmp, int samplesPerPixel, int64_t y1, int64_t x1, int64_t x2, int64_t width, int64_t height, int thickness, int position);
bool drawYHighlight(BYTE *pBmp, int samplesPerPixel, int64_t x1, int64_t y1, int64_t y2, int64_t width, int64_t height, int thickness, int position);



class CompositeSlide : public VirtualSlide {
protected:
  static const char* mMiniNames[4][4];
  double mxStart, myStart;
  int64_t mxMax, mxMin, myMax, myMin;
  std::vector<JpgIniConf*> mEtc; 
public:
  CompositeSlide() { compositeClearAttribs(); } 
  virtual ~CompositeSlide() { compositeCleanup(); }
  void compositeClearAttribs();
  void compositeCleanup();
  void close() { compositeCleanup(); baseCleanup(); compositeClearAttribs(); baseClearAttribs(); }
  bool open(const std::string& inputDir, int options, int orientation, int debugLevel = 0, int64_t bestXOffset = -1, int64_t bestYOffset = -1, safeBmp **pImageL2 = NULL); 
  bool read(safeBmp *pDestBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t width, int64_t height, int64_t *readWidth, int64_t *readHeight);

  #ifndef USE_MAGICK
  bool findXYOffset(int lowerLevel, int higherLevel, int64_t *bestXOffset0, int64_t *bestYOffset0, int64_t *bestXOffset1, int64_t *bestYOffset1, int optUseCustomOffset, int debugLevel, std::fstream& logFile);
  #endif

  #ifndef USE_MAGICK
  bool loadFullImage(int level, safeBmp **ptpFullImage, cv::Mat **ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile);
  #else
  bool loadFullImage(int level, safeBmp **ptpImageL2, void **ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile);
  #endif
 
  bool checkLevel(int level);
  bool checkZLevel(int level, int direction, int zLevel);

  void blendLevelsRegionScan(BlendSection** yFreeMap, int64_t ySize, int orientation);
  bool setOrientation(int orientation, std::fstream& logFile);

  static bool testHeader(BYTE*, int64_t);
  bool drawBorder(BYTE *pBuff, int samplesPerPixel, int64_t x, int64_t y, int64_t width, int64_t height, int level);
  std::vector<JpgFileXY>* getTileXYArray(int level); 
  bool isPreviewSlide(int level); 
  int getTotalLevels();
  int getTotalZLevels();
  int getTotalBottomZLevels();
  int getTotalTopZLevels();
  int getQuality(int level);
  int64_t getPixelWidth(int level);
  int64_t getPixelHeight(int level);
  int64_t getActualWidth(int level);
  int64_t getActualHeight(int level);
  int64_t getLevelWidth(int level);
  int64_t getLevelHeight(int level);
  double getXAdj(int level);
  double getYAdj(int level);
  int64_t getTotalTiles(int level);
  bool parseMagStr(std::string magLine);
};


#ifndef USE_MAGICK
class CVMatchCompare
{
public:
  bool operator() (const cv::DMatch& match1, const cv::DMatch& match2);
};
#endif

#endif
