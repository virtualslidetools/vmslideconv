/*************************************************************************
Initial author: Paul F. Richards (paulrichards321@gmail.com) 2016-2017 
https://github.com/paulrichards321/oly2gmap

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

#include <iostream>
#include <fstream>
#include <cstring>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstdint>
#include <cctype>
#include <cassert>
#include <sys/stat.h>

#if defined(_WIN32) || defined(_WIN64)
#include "console-mswin.h"
#include "getopt-mswin.h"
#else
#include "console-unix.h"
#include <unistd.h>
#include <getopt.h>
#endif
#include "vmslideconv.h"


std::string bool2Txt(bool cond)
{
  std::string result = "false";
  if (cond) result = "true";
  return result;
}

std::string boolInt2Txt(int cond)
{
  std::string result = "false";
  if (cond) result = "true";
  return result;
}


int getBoolOpt(const char *txtoptarg)
{
  const char *available[14] = { 
    "1", 
    "true", "TRUE", 
    "enable", "ENABLE", 
    "on", "ON", 
    "0", 
    "false", "FALSE", 
    "disable", "DISABLE", 
    "off", "OFF" 
  };
  if (txtoptarg == NULL) 
  {
    return 1;
  }
  std::string txtoptarg2 = txtoptarg;
  for (int i = 0; i < 14; i++)
  {
    if (txtoptarg2.find(available[i]) != std::string::npos)
    {
      if (i < 7) return 1;
      else return 0;
    }
  }
  return 1;
}


int getBoolOpt(const char *txtoptarg, bool& invalidOpt)
{
  const char *available[14] = { 
    "1", 
    "true", "TRUE", 
    "enable", "ENABLE", 
    "on", "ON", 
    "0", 
    "false", "FALSE", 
    "disable", "DISABLE", 
    "off", "OFF" 
  };
  (void) invalidOpt;
  if (txtoptarg == NULL) 
  {
    return 1;
  }
  std::string txtoptarg2 = txtoptarg;
  for (int i = 0; i < 14; i++)
  {
    if (txtoptarg2.find(available[i]) != std::string::npos)
    {
      if (i < 7) return 1;
      else return 0;
    }
  }
  return 1;
}


int getIntOpt(const char *txtoptarg, bool& invalidOpt)
{
  unsigned int i=0;
  if (txtoptarg==NULL)
  {
    invalidOpt = true;
    return 0;
  }
  std::string txtoptarg2 = txtoptarg;
  while (i < txtoptarg2.length() && (txtoptarg2[i]==' ' || txtoptarg2[i]==':' || txtoptarg2[i]=='=' || txtoptarg2[i]=='\t')) i++;
  txtoptarg2 = txtoptarg2.substr(i);
  if (txtoptarg2.length() > 0 && isdigit(txtoptarg2[0]))
  {
    return atoi(txtoptarg2.c_str());
  }
  invalidOpt = true;
  return 0;
}


double getDoubleOpt(const char *txtoptarg, bool& invalidOpt)
{
  unsigned int i=0;
  if (txtoptarg==NULL)
  {
    invalidOpt = true;
    return 0;
  }
  std::string txtoptarg2 = txtoptarg;
  while (i < txtoptarg2.length() && (txtoptarg2[i]==' ' || txtoptarg2[i]==':' || txtoptarg2[i]=='=' || txtoptarg2[i]=='\t')) i++;
  txtoptarg2 = txtoptarg2.substr(i);
  if (txtoptarg2.length() > 0 && isdigit(txtoptarg2[0]))
  {
    return atof(txtoptarg2.c_str());
  }
  invalidOpt = true;
  return 0;
}


std::string getStrOpt(const char *txtoptarg, bool& invalidOpt)
{
  std::string txtoptarg2 = "";
  unsigned int i=0;
  if (txtoptarg==NULL)
  {
    invalidOpt = true;
    return txtoptarg2;
  }
  txtoptarg2 = txtoptarg;
  while (i < txtoptarg2.length() && (txtoptarg2[i]==' ' || txtoptarg2[i]==':' || txtoptarg2[i]=='=' || txtoptarg2[i]=='\t' || txtoptarg2[i] == '\'' || txtoptarg2[i] == '"')) i++;
  if (i < txtoptarg2.length())
  {
    txtoptarg2 = txtoptarg2.substr(i);
    i = (unsigned int) txtoptarg2.length();
    while (i > 0 && (txtoptarg2[i]==' ' || txtoptarg2[i]=='\t' || txtoptarg2[i] == '\'' || txtoptarg2[i] == '"')) i--;
    if (i > 0)
    {
      txtoptarg2 = txtoptarg2.substr(0, i);
      return txtoptarg2;
    }
  }
  invalidOpt = true;
  txtoptarg2 = "";
  return txtoptarg2;
}


typedef struct slidelevel_t
{
  bool optInOlympusIni;
  bool optInAperioSvs;
  bool optOutTif;
  bool optOutGoogle;
  int optDebug;
  int optQuality;
  bool optUseGamma;
  double optGamma;
  int64_t optMaxMem;
  bool optLog;
  int optBlend;
  int orientation;
  int olympusLevel;
  int readDirection;
  int readZLevel;
  int outLevel;
  bool center;
  bool tiled;
  int64_t srcTotalCols;
  int64_t srcTotalRows;
  int64_t srcTotalColsL2;
  int64_t srcTotalRowsL2;
  int64_t L2Size;
  int64_t destTotalCols, destTotalCols2;
  int64_t destTotalRows, destTotalRows2;
  int finalOutputCols, finalOutputCols2;
  int finalOutputRows, finalOutputRows2;
  int inputTileCols;
  int inputTileRows;
  int totalSubTiles;
  int xSubTile, ySubTile;
  int readSubTiles;
  int64_t readColsL2;
  int64_t readRowsL2;
  int64_t preOrientResultCols;
  int64_t preOrientResultRows;
  int64_t readCols;
  int64_t readRows;
  int64_t grabColsRead;
  int64_t grabRowsRead;
  int64_t colsPreOrient;
  int64_t rowsPreOrient;
  double magnifyX;
  double magnifyY;
  double destTotalColsDec;
  double destTotalRowsDec;
  double xScale, yScale;
  double xScaleL2, yScaleL2;
  double xScaleReverse, yScaleReverse;
  double xScaleResize, yScaleResize;
  double xBlendFactor;
  double yBlendFactor;
  double grabColsA, grabColsB;
  double grabRowsA, grabRowsB;
  double grabColsL2, grabRowsL2;
  double xSrcPreOrient;
  double ySrcPreOrient;
  double xSrcRead;
  double ySrcRead;
  double xSrc;
  double ySrc;
  bool readOkL2;
  int64_t inputTileColsRead;
  int64_t inputTileRowsRead;
  int64_t inputSubTileColsRead;
  int64_t inputSubTileRowsRead;
  int64_t xMargin;
  int64_t yMargin;
  int scaleMethod;
  int scaleMethodL2;
  int64_t totalXSections, totalYSections;
  unsigned char bkgdColor;
  BlendSection **ySubSections;
  safeBmp* pBitmapSrc;
  safeBmp* pPreOrientBitmap;
  safeBmp* pBitmapL2;
  safeBmp* pBitmapBlended;
  safeBmp* pBitmapFinal;
  safeBmp wholeTile;
  safeBmp preOrientBitmap;
  safeBmp subTileBitmap;
  safeBmp subTileScaled;
  safeBmp sizedBitmap;
  safeBmp sizedBitmap2;
  safeBmp bitmapBlended;
  bool fillin;
  int64_t xLevelOffset;
  int64_t yLevelOffset;
  int64_t xStartTile;
  int xCenter;
  int yCenter;
  double xStartSrc;
  double yStartSrc;
  double xSrcStart2;
  double ySrcStart2;
  int64_t xOutTile, xTileMap;
  int64_t yOutTile, yTileMap;
  int64_t xEndTile, yEndTile;
  int64_t xDest;
  int64_t yDest;
  int64_t outputLvlTotalCols;
  int64_t outputLvlTotalRows;
  safeBmpZoomRes *pZoomRes;
  std::string *pTileName;
  int writeOutputCols;
  int writeOutputRows;
  int perc, percOld;
  bool onePercHit;
  bool error;
} SlideLevel;


class SlideConvertor
{
protected:
  bool mValidObject;
  bool mOptInOlympusIni;
  bool mOptInAperioSvs;
  bool mOptOutTif;
  bool mOptOutGoogle;
  bool mOptBlend;
  int mOptDebug;
  bool mOptZStack;
  bool mOptLog;
  int64_t mOptMaxMem;
  bool mOptUseGamma;
  double mOptGamma;
  int mOrientation;
  VirtualSlide *slide;
  Tiff *mTif;
  ZipFile *mZip;
  std::ofstream *logFile;
  std::string errMsg;
  std::string mOutputFile;
  std::string mOutputDir;
  std::string mFileNameOnly;
  std::string mYRoot, mYRootZip;
  int64_t mBaseTotalCols, mBaseTotalRows;
  int64_t mBaseTotalCols2, mBaseTotalRows2;
  int64_t mBaseActualCols, mBaseActualRows;
  int64_t mBaseActualCols2, mBaseActualRows2;
  int mBaseLevel;
  bool mCenter;
  int mOptQuality;
  int mStep, mZSteps;
  int mLastZLevel, mLastDirection;
  int mTopOutLevel;
  int64_t mMaxSide;
  safeBmp *mpImageL2;
  int64_t mTotalYSections;
  BlendSection **mySubSections;
  bool mUseableLvl1;
  int mxLvl1Divisor, myLvl1Divisor;
public:
  #if defined(_WIN32) || defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW__)
  static const char mPathSeparator='\\';
  #else
  static const char mPathSeparator='/';
  #endif
public:
  SlideConvertor();
  ~SlideConvertor() { closeRelated(); }
  void closeRelated();
  std::string getErrMsg() { return errMsg; }
  void setGamma(bool optUseGamma, double optGamma) { mOptUseGamma = optUseGamma; mOptGamma = optGamma; }
  void setDebugLevel(int optDebug) { mOptDebug = optDebug; }
  void setQuality(int optQuality) { mOptQuality = optQuality; }
  void setMaxMem(int64_t optMaxMem) { mOptMaxMem = optMaxMem; }
  int open(std::string inputFile, std::string outputFile, std::string hostname, int options, int orientation);
  bool my_mkdir(std::string name);
  void calcCenters(int outLevel, int64_t& xCenter, int64_t& yCenter);
  int convert();
  int outputLevel(int olympusLevel, int magnification, int outLevel, int options, int64_t readColsL2, int64_t readRowsL2, safeBmp *pBitmapL2);
  int checkFullL2(int64_t *pReadColsL2, int64_t *pReadRowsL2, safeBmp **pFullL2);
  int convert2Tif();
  int convert2Gmap();
  void calcTilesPrep(SlideLevel& l);
  void outputNextRow(SlideLevel& l);
  void outputNextTile(SlideLevel& l);
  void readAndProcessNextSubTile(SlideLevel &l);
  void processReadSubTile(SlideLevel& l);
  void blendL2WithSrc(SlideLevel &l);
  void processGamma(SlideLevel& l);
  void printPercDone(SlideLevel& l);
};


bool SlideConvertor::my_mkdir(std::string name)
{
  struct stat info;
  std::string errorMkdir;
  if (stat(name.c_str(), &info) != 0)
  {
    if (!platform_mkdir(name, &errorMkdir))
    {
      std::cout << "Fatal Error: Cannot create directory: " << name << ": " << errorMkdir << " Quiting!";
      return false;
    }
  }
  else if (!(info.st_mode & S_IFDIR))
  {
    std::cout << "Fatal Error: " << name << " exists already, and is not a directory! Quiting!";
    return false;
  }
  return true;
}


SlideConvertor::SlideConvertor()
{
  slide = 0;
  mTif = 0;
  mZip = 0;
  logFile = 0;
  mCenter = false;
  mValidObject = false;
  mStep=0;
  mLastZLevel=-1;
  mLastDirection=-1;
  mZSteps=0;
  mBaseTotalCols=0;
  mBaseTotalCols2=0;
  mBaseTotalRows=0;
  mBaseTotalRows2=0;
  mBaseLevel=0;
  mOptZStack=getBoolOpt(SLIDE_DEF_ZSTACK);
  mOptQuality=SLIDE_DEF_QUALITY;
  mpImageL2=NULL;
  mTotalYSections = 0;
  mySubSections = NULL;
  mOrientation = 0;
  mUseableLvl1 = false;
  mxLvl1Divisor = 0;
  myLvl1Divisor = 0;
}


void SlideConvertor::calcCenters(int outLevel, int64_t &xCenter, int64_t &yCenter)
{
  int64_t baseCols = slide->getLevelCols(mBaseLevel);
  int64_t baseRows = slide->getLevelRows(mBaseLevel);
  int64_t gmapMaxSide = (1 << mTopOutLevel) * 256;
  int64_t xBaseCenter = (gmapMaxSide - baseCols) / 2;
  int64_t yBaseCenter = (gmapMaxSide - baseRows) / 2;
  
  int64_t xTopCenter = xBaseCenter >> mTopOutLevel;
  xCenter = xTopCenter << outLevel;

  int64_t yTopCenter = yBaseCenter >> mTopOutLevel;
  yCenter = yTopCenter << outLevel;
}


void SlideConvertor::processGamma(SlideLevel &l)
{
  safeBmp* pSrc = l.pBitmapFinal;
  BYTE* pBmpData = pSrc->data;
  double invGamma = 1.0f/l.optGamma;

  if (pSrc == NULL || pSrc->data == NULL) return;

  int64_t strideCols = pSrc->cols * 3;
  int64_t rows = pSrc->rows;

  for (int64_t y=0; y < rows; y++)
  {
    BYTE * pBmpData2 = pBmpData + (y * strideCols);
    for (int64_t x=0; x < strideCols; x++)
    {
      double gamma = (double) *pBmpData2 / (double) 255.0f;
      double powed = pow(gamma, invGamma); 
      int value = (int) round(powed * 255);
      if (value > 255) value=255;
      *pBmpData2 = (BYTE) value;
      pBmpData2++;
    }
  }
}


// Scale the larger complete L2 image into a tiled smaller 
// mini version
// of it if L2 scaling is requested and the L2 pyramid level was
// read success
void SlideConvertor::blendL2WithSrc(SlideLevel &l)
{
  safeBmp bitmapL2Mini, scaledL2Mini, scaledL2Mini2;
  safeBmp *pCurrentL2Bmp = NULL;

  safeBmpClear(&bitmapL2Mini);
  safeBmpClear(&scaledL2Mini);
  safeBmpClear(&scaledL2Mini2);

  int64_t scaledL2Cols = (int64_t) round(l.grabColsL2);
  int64_t scaledL2Rows = (int64_t) round(l.grabRowsL2);

  int64_t xSrcStartL2=(int64_t) round(l.xSrc * l.xScaleL2);
  int64_t ySrcStartL2=(int64_t) round(l.ySrc * l.yScaleL2);
  int64_t xDestStartL2=0, yDestStartL2=0;

  if (xSrcStartL2 < 0)
  {
    xDestStartL2 = abs(xSrcStartL2);
    xSrcStartL2 = 0;
  }
  if (ySrcStartL2 < 0)
  {
    yDestStartL2 = abs(ySrcStartL2);
    ySrcStartL2 = 0;
  }
  safeBmpAlloc2(&bitmapL2Mini, scaledL2Cols, scaledL2Rows);
  safeBmpByteSet(&bitmapL2Mini, l.bkgdColor);
  safeBmpCpy(&bitmapL2Mini, xDestStartL2, yDestStartL2, l.pBitmapL2, xSrcStartL2, ySrcStartL2, scaledL2Cols, scaledL2Rows);
  double xScaleResize = (double) l.inputTileCols / (double) l.grabColsL2;
  double yScaleResize = (double) l.inputTileRows / (double) l.grabRowsL2;

  if (xScaleResize != 1.0 || yScaleResize != 1.0 || l.finalOutputCols != scaledL2Cols || l.finalOutputRows != scaledL2Rows)
  {
    safeBmpZoom2(&scaledL2Mini, &bitmapL2Mini, l.finalOutputCols, l.finalOutputRows, xScaleResize, yScaleResize, l.scaleMethodL2, l.pZoomRes);
    pCurrentL2Bmp = &scaledL2Mini;
  }
  else
  {
    pCurrentL2Bmp = &bitmapL2Mini;
  }

  if (l.finalOutputCols != l.finalOutputCols2 || l.finalOutputRows != l.finalOutputRows2)
  {
    safeBmpAlloc2(&scaledL2Mini2, (int64_t) l.finalOutputCols2, (int64_t) l.finalOutputRows2);
    safeBmpByteSet(&scaledL2Mini2, l.bkgdColor);
    safeBmpCpy(&scaledL2Mini2, 0, 0, pCurrentL2Bmp, 0, 0, l.finalOutputCols, l.finalOutputRows);
    pCurrentL2Bmp = &scaledL2Mini2;
  }
  if (l.optDebug > 1)
  {
    std::string jpgErrMsg;
    std::string l2TileName;
    std::stringstream ss;
    ss << "vmslideconv.l2" << mPathSeparator << "l2.l" << l.olympusLevel << "x" << l.xSrc << "y" << l.ySrc << ".jpg";
    l2TileName=ss.str();
    int writeErrors=safeJpgWrite(pCurrentL2Bmp, l2TileName, l.optQuality, &jpgErrMsg);
    if (writeErrors > 0) 
    {
      std::cout << "Error writing debug file '" << l2TileName << "' errMsg: " << jpgErrMsg << std::endl;
    }
  }
  safeBmpCpy(l.pBitmapBlended, 0, 0, l.pBitmapSrc, 0, 0, l.pBitmapBlended->cols, l.pBitmapBlended->rows);
  xScaleResize = (double) mBaseTotalCols / (double) l.srcTotalCols;
  yScaleResize = (double) mBaseTotalRows / (double) l.srcTotalRows;
  BlendArgs blendArgs;
  blendArgs.xSrc = l.xSrc;
  blendArgs.ySrc = l.ySrc;
  blendArgs.grabColsB = l.grabColsB;
  blendArgs.grabRowsB = l.grabRowsB;
  blendArgs.xMargin = 0;
  blendArgs.yMargin = 0;
  if (blendArgs.xSrc < 0)
  {
    blendArgs.grabColsB += blendArgs.xSrc;
    blendArgs.xSrc = 0;
    blendArgs.xMargin = l.xCenter;
  }
  if (blendArgs.ySrc < 0)
  {
    blendArgs.grabRowsB += blendArgs.ySrc;
    blendArgs.ySrc = 0;
    blendArgs.yMargin = l.yCenter;
  }
  blendArgs.grabColsB *= xScaleResize;
  blendArgs.grabRowsB *= yScaleResize;
  blendArgs.xSrc *= xScaleResize;
  blendArgs.ySrc *= yScaleResize;
  blendArgs.pSafeDest = l.pBitmapBlended;
  blendArgs.pSafeSrcL2 = pCurrentL2Bmp;
  blendArgs.xFactor = l.xBlendFactor;
  blendArgs.yFactor = l.yBlendFactor;
  blendArgs.yFreeMap = l.ySubSections;
  blendArgs.ySize = l.totalYSections;
  blendLevels(&blendArgs);
  l.pBitmapFinal = l.pBitmapBlended;
  
  safeBmpFree(&bitmapL2Mini);
  safeBmpFree(&scaledL2Mini);
  safeBmpFree(&scaledL2Mini2);
}


void SlideConvertor::processReadSubTile(SlideLevel& l)
{
  // rotate the bitmap if needed
  if (l.orientation == 0)
  {
    l.readCols = l.preOrientResultCols;
    l.readRows = l.preOrientResultRows;
  }
  else
  {
    safeBmpRotate(l.pBitmapSrc, l.pPreOrientBitmap, l.orientation);
    if (l.orientation == 90 || l.orientation == -90 || l.orientation == 270)
    {
      l.readCols = l.preOrientResultRows;
      l.readRows = l.preOrientResultCols;
    }
    else
    {
      l.readCols = l.preOrientResultCols;
      l.readRows = l.preOrientResultRows;
    }
  }
  // Check if read rows and cols returned from composite read
  // came back smaller than expected, resize the bitmap if so
  if (l.readCols != l.grabColsRead || l.readRows != l.grabRowsRead)
  {
    safeBmpAlloc2(&l.sizedBitmap, l.grabColsRead, l.grabRowsRead);
    safeBmpByteSet(&l.sizedBitmap, l.bkgdColor);

    int64_t copyCols=l.readCols;
    if (copyCols > l.grabColsRead) copyCols=l.grabColsRead;
    int64_t copyRows=l.readRows;
    if (copyRows > l.grabRowsRead) copyRows=l.grabRowsRead;
    safeBmpCpy(&l.sizedBitmap, 0, 0, l.pBitmapSrc, 0, 0, copyCols, copyRows);
    l.pBitmapSrc = &l.sizedBitmap;
    l.pBitmapFinal = &l.sizedBitmap;
  }
  // Check if the grabbed data needs to be scaled in or out
  // If we are at the very bottom of the image pyramid, this part will
  // be skipped because no scaling will be done
  if (l.grabColsRead!=l.inputSubTileColsRead || l.grabRowsRead!=l.inputSubTileRowsRead)
  {
    double xScaleResize = (double) l.inputSubTileColsRead / (double) l.grabColsRead;
    double yScaleResize = (double) l.inputSubTileRowsRead / (double) l.grabRowsRead;
    safeBmpZoom2(&l.subTileScaled, l.pBitmapSrc, l.inputSubTileColsRead, l.inputSubTileRowsRead, xScaleResize, yScaleResize, l.scaleMethod, l.pZoomRes);
    l.pBitmapSrc = &l.subTileScaled;
    l.pBitmapFinal = &l.subTileScaled;
  }
  // Check if the current input tile is smaller than what is needed
  // to process the L2 background with it. This can occur
  // if the grabbed original composite source image has different
  // dimensions (cx,cy) than what is need because of a margin
  // that is need to center the image the side of the grabbed image
  if (l.totalSubTiles==1 && (l.inputSubTileColsRead!=l.inputTileCols || l.inputSubTileRowsRead!=l.inputTileRows))
  {
    safeBmpAlloc2(&l.sizedBitmap2, l.inputTileCols, l.inputTileRows);
    safeBmpByteSet(&l.sizedBitmap2, l.bkgdColor);
    int64_t copyCols = l.inputSubTileColsRead;
    if (l.xMargin + copyCols > l.inputTileCols)
    {
      copyCols -= (l.xMargin + copyCols) - l.inputTileCols;
    }
    int64_t copyRows = l.inputSubTileRowsRead;
    if (l.yMargin + copyRows > l.inputTileRows)
    {
      copyRows -= (l.yMargin + copyRows) - l.inputTileRows;
    }
    if (copyCols > 0 && copyRows > 0)
    {
      safeBmpCpy(&l.sizedBitmap2, l.xMargin, l.yMargin, l.pBitmapSrc, 0, 0, copyCols, copyRows);
    }
    l.pBitmapSrc = &l.sizedBitmap2;
    l.pBitmapFinal = &l.sizedBitmap2;
  }          
  else if (l.totalSubTiles > 1)
  {
    int64_t copyCols = l.inputSubTileColsRead;
    if (l.xMargin + copyCols > l.inputTileCols)
    {
      copyCols -= (l.xMargin + copyCols) - l.inputTileCols;
    }
    int64_t copyRows = l.inputSubTileRowsRead;
    if (l.yMargin + copyRows > l.inputTileRows)
    {
      copyRows -= (l.yMargin + copyRows) - l.inputTileRows;
    }
    int64_t xSubLoc = (int64_t) round((double) l.xSubTile * ((double) l.inputTileCols / (double) l.totalSubTiles));
    int64_t ySubLoc = (int64_t) round((double) l.ySubTile * ((double) l.inputTileRows / (double) l.totalSubTiles));
    xSubLoc += l.xMargin;
    ySubLoc += l.yMargin;
    if (copyCols > 0 && copyRows > 0)
    {
      safeBmpCpy(&l.wholeTile, xSubLoc, ySubLoc, l.pBitmapSrc, 0, 0, copyCols, copyRows);
    }
    l.pBitmapSrc = &l.wholeTile;
    l.pBitmapFinal = &l.wholeTile;
  }
  if (l.optDebug > 1) 
  {
    std::string preTileName;
    std::string jpgErrMsg;
    std::stringstream ss;
    ss << "vmslideconv.z" << mPathSeparator << "z.l" << l.olympusLevel << "x" << l.xSrc << "y" << l.ySrc << ".jpg";
    preTileName=ss.str();
    int writeErrors=safeJpgWrite(l.pBitmapSrc, preTileName, l.optQuality, &jpgErrMsg);
    if (writeErrors > 0)
    {
      std::cout << "Error writing debug file: " << jpgErrMsg << std::endl;
    }
  }
}


void SlideConvertor::readAndProcessNextSubTile(SlideLevel& l)
{
  l.xSrc = l.xSrcStart2 + (l.xSubTile * l.grabColsA);
  if (round(l.xSrc) + round(l.grabColsA) < 1.0f) return;
  
  safeBmpClear(&l.preOrientBitmap);
  safeBmpClear(&l.subTileBitmap);
  safeBmpClear(&l.subTileScaled);
  safeBmpClear(&l.sizedBitmap);
  safeBmpClear(&l.sizedBitmap2);
            
  l.xSrcRead = l.xSrc;
  l.ySrcRead = l.ySrc;
  double grabColsReadDec = l.grabColsA;
  double grabRowsReadDec = l.grabRowsA;
  l.inputSubTileColsRead = (int64_t) round((double) l.inputTileCols / (double) l.totalSubTiles);
  l.inputSubTileRowsRead = (int64_t) round((double) l.inputTileRows / (double) l.totalSubTiles);
  l.xMargin = 0;
  l.yMargin = 0;
  if (l.xSrc < 0.0)
  {
    grabColsReadDec=l.grabColsA + l.xSrc;
    l.xSrcRead = 0.0;
    l.xMargin = l.xCenter - (l.xSubTile * l.inputSubTileColsRead);
  }
  if (l.ySrc < 0.0)
  {
    grabRowsReadDec=l.grabRowsA + l.ySrc;
    l.ySrcRead = 0.0;
    l.yMargin = l.yCenter - (l.ySubTile * l.inputSubTileRowsRead);
  }
  if (l.xSrcRead + grabColsReadDec > (double) l.srcTotalCols)
  {
    grabColsReadDec = (double) l.srcTotalCols - l.xSrcRead;
    l.inputSubTileColsRead = (int64_t) round(grabColsReadDec * l.xScaleReverse);
  }
  if (l.ySrcRead + grabRowsReadDec > (double) l.srcTotalRows)
  {
    grabRowsReadDec = (double) l.srcTotalRows - l.ySrcRead;
    l.inputSubTileRowsRead = (int64_t) round(grabRowsReadDec * l.yScaleReverse);
  }
  if (l.xSrc < 0.0 || l.ySrc < 0.0)
  {
    l.inputSubTileColsRead = (int64_t) round(grabColsReadDec * l.xScaleReverse);
    l.inputSubTileRowsRead = (int64_t) round(grabRowsReadDec * l.yScaleReverse);
  }
  l.grabColsRead = (int64_t) round(grabColsReadDec);
  l.grabRowsRead = (int64_t) round(grabRowsReadDec);
  if (l.grabColsRead <= 0 || l.grabRowsRead <= 0) return;
  bool allocOk = slide->allocate(&l.subTileBitmap, l.olympusLevel, (int64_t) round(l.xSrcRead), (int64_t) round(l.ySrcRead), l.grabColsRead, l.grabRowsRead, false);
  if (allocOk == false) 
  {
    std::cerr << "Fatal error: Failed to allocate subtile bitmap!" << std::endl; 
    exit(1);
  }
  if (mOrientation == 0)
  {
    l.xSrcPreOrient = l.xSrcRead;
    l.ySrcPreOrient = l.ySrcRead;
    l.colsPreOrient = l.grabColsRead;
    l.rowsPreOrient = l.grabRowsRead;
  }
  else if (mOrientation == 90)
  {
    l.xSrcPreOrient = l.ySrcRead;
    l.ySrcPreOrient = ((double) l.srcTotalCols - l.xSrcRead) - (double) l.grabColsRead;
    l.colsPreOrient = l.grabRowsRead;
    l.rowsPreOrient = l.grabColsRead;
  }
  else if (mOrientation == -90 || mOrientation == 270)
  {
    l.xSrcPreOrient = ((double) l.srcTotalRows - l.ySrcRead) - (double) l.grabRowsRead;
    l.ySrcPreOrient = l.xSrcRead;
    l.colsPreOrient = l.grabRowsRead;
    l.rowsPreOrient = l.grabColsRead;
  }
  else if (mOrientation == 180)
  {
    l.xSrcPreOrient = ((double) l.srcTotalCols - l.xSrcRead) - (double) l.grabColsRead;
    l.ySrcPreOrient = ((double) l.srcTotalRows - l.ySrcRead) - (double) l.grabRowsRead;
    l.colsPreOrient = l.grabColsRead;
    l.rowsPreOrient = l.grabRowsRead;
  }
  if (mOrientation == 0)
  {
    l.pPreOrientBitmap = &l.subTileBitmap;
  }
  else
  {
    allocOk = slide->allocate(&l.preOrientBitmap, l.olympusLevel, (int64_t) round(l.xSrcPreOrient), (int64_t) round(l.ySrcPreOrient), l.colsPreOrient, l.rowsPreOrient, true);
    if (allocOk == false)
    {
      std::cerr << "Fatal error: Failed to allocate subtile bitmap!" << std::endl; 
      exit(1);
    }
    l.pPreOrientBitmap = &l.preOrientBitmap;
    safeBmpByteSet(&l.preOrientBitmap, l.bkgdColor);
  }
  l.pBitmapSrc = &l.subTileBitmap;
  l.pBitmapFinal = &l.subTileBitmap;
  safeBmpByteSet(&l.subTileBitmap, l.bkgdColor);
            
  l.readCols=0;
  l.readRows=0;
  if (l.pBitmapSrc->data == NULL)
  {
    std::cerr << "Fatal error: Failed to allocate subtile bitmap!" << std::endl; 
    exit(1);
  }
  if (l.optDebug > 2)
  {
    *logFile << " slide->read(x=" << l.xSrcPreOrient << " y=" << l.ySrcPreOrient << " grabColsA=" << l.colsPreOrient << " grabRowsA=" << l.rowsPreOrient << " olympusLevel=" << l.olympusLevel << "); " << std::endl;
  }
  bool readOkSrc = slide->read(l.pPreOrientBitmap, l.olympusLevel, l.readDirection, l.readZLevel, (int64_t) round(l.xSrcPreOrient), (int64_t) round(l.ySrcPreOrient), l.colsPreOrient, l.rowsPreOrient, &l.preOrientResultCols, &l.preOrientResultRows);
  if (readOkSrc)
  {
    l.readSubTiles++;
  }
  else
  {
    std::cerr << "Failed to read level " << l.olympusLevel << " tile @ x=" << l.xSrcPreOrient << " y=" << l.ySrcPreOrient << " cols=" << l.colsPreOrient << " rows=" << l.rowsPreOrient << std::endl;
  }
  if (l.optDebug > 2)
  {
    std::cout << "readCols: " << l.preOrientResultCols << " readRows: " << l.preOrientResultRows<< " grabCols: " << l.colsPreOrient << " grabRows: " << l.rowsPreOrient << std::endl;
  }
  if (readOkSrc)
  {
    if (l.optDebug > 2)
    {
      std::stringstream ss;
      ss << "vmslideconv.a" << mPathSeparator << "a" << l.olympusLevel << "y" << l.ySrcPreOrient << "x" << l.xSrcPreOrient << ".jpg";
      std::string fname = ss.str();
      std::string jpgErrMsg;
      safeJpgWrite(l.pPreOrientBitmap, fname, 90, &jpgErrMsg);
    }
    processReadSubTile(l);
  }
  if (l.totalSubTiles > 1)
  {
    safeBmpFree(&l.preOrientBitmap);
    safeBmpFree(&l.subTileBitmap);
    safeBmpFree(&l.subTileScaled);
    safeBmpFree(&l.sizedBitmap);
    safeBmpFree(&l.sizedBitmap2);
  }
  l.xSrc += l.grabColsA;
}


void SlideConvertor::printPercDone(SlideLevel& l)
{
  l.perc=(int)(((double) l.yDest / (double) l.outputLvlTotalRows) * 100);
  if (l.perc>100)
  {
    l.perc=100;
  }
  if (l.perc>l.percOld)
  {
    l.percOld=l.perc;
    retractCursor();
    std::cout << l.perc << "% done...    " << std::flush;
  }
  else if (l.onePercHit==false)
  {
    retractCursor();
    std::cout << l.perc << "% done...    " << std::flush;
    l.onePercHit = true;
  }
}


void SlideConvertor::outputNextTile(SlideLevel& l)
{
  std::ostringstream tileNameStream, tileNameStreamZip;
  std::string fullTilePath;
  l.xSrc = (l.xOutTile * l.grabColsB) + l.xStartSrc;
  if (round(l.xSrc) + round(l.grabColsB) < 1.0f)
  {
    std::cerr << "For some reason we have hit this! l.xSrc=" << l.xSrc << " l.xStartSrc=" << l.xStartSrc << " l.grabColsB=" << l.grabColsB << std::endl;
    return;
  }
  tileNameStream << mYRoot << mPathSeparator << l.xTileMap << ".jpg";
  tileNameStreamZip << mYRootZip << ZipFile::mZipPathSeparator << l.xTileMap << ".jpg";
  fullTilePath = tileNameStream.str();
  l.pTileName = &fullTilePath;
  l.inputSubTileColsRead = (int64_t) round((double) l.inputTileCols / (double) l.totalSubTiles);
  l.inputSubTileRowsRead = (int64_t) round((double) l.inputTileRows / (double) l.totalSubTiles);
  l.readSubTiles=0;
  if (l.totalSubTiles > 1)
  {
    safeBmpByteSet(&l.wholeTile, l.bkgdColor);
  }
  l.pBitmapSrc = NULL;
  l.xSrcStart2=l.xSrc;
  l.ySrcStart2=l.ySrc;

  safeBmpClear(&l.preOrientBitmap);
  safeBmpClear(&l.subTileBitmap);
  safeBmpClear(&l.subTileScaled);
  safeBmpClear(&l.sizedBitmap);
  safeBmpClear(&l.sizedBitmap2);

  // this tile needs to be the same with and length as final output cols and length
  l.ySubTile=0;
  while (l.ySubTile < l.totalSubTiles && round(l.ySrc) < l.srcTotalRows)
  {
    l.xSrc = l.xSrcStart2;
    l.ySrc = l.ySrcStart2 + (l.ySubTile * l.grabRowsA);
    if (round(l.ySrc) + round(l.grabRowsA) < 1.0f) 
    {
      l.ySubTile++;
      continue;
    }
    for (l.xSubTile=0; l.xSubTile < l.totalSubTiles && round(l.xSrc) < l.srcTotalCols; l.xSubTile++)
    {
      readAndProcessNextSubTile(l);
    }
    l.ySubTile++;
    l.ySrc = l.ySrcStart2 + (l.ySubTile * l.grabRowsA);
    printPercDone(l);
  }
  l.xSrc = l.xSrcStart2;
  l.ySrc = l.ySrcStart2;
  l.xSrcRead = l.xSrc;
  l.ySrcRead = l.ySrc;
  if (l.readSubTiles > 0) 
  {
    bool writeOk=false;
    if (l.readOkL2 && l.optBlend)
    {
      blendL2WithSrc(l);  
    }
    if (l.optUseGamma)
    {
      processGamma(l);
    }
    if (l.optOutGoogle)
    {
      BYTE* pJpegBytes = NULL;
      unsigned long outSize = 0;
      std::string tileName = tileNameStreamZip.str();
      int compressErrors = safeJpgCompress(l.pBitmapFinal, &pJpegBytes, l.optQuality, &errMsg, &outSize);
      if ((int) outSize <= 0)
      {
        std::cout << "Outsize is <= 0!!!" << std::endl;
      }
      if (compressErrors == 0 && mZip->addFile(tileName, pJpegBytes, outSize)==OLY_ZIP_OK)
      {
        writeOk=true;
      }
      else
      {
        l.error=true;
      }
      safeJpgCompressFree(&pJpegBytes);
    }
    else if (l.optOutTif)
    {
      if (l.tiled)
      {
        writeOk=mTif->writeEncodedTile(l.pBitmapFinal->data, (unsigned int) l.xDest, (unsigned int) l.yDest, 1);
      }
      else
      {
        std::stringstream ss;
        ss << "vmslideconv.l" << mPathSeparator << "l" << l.olympusLevel << "mag" << l.magnifyX << ".jpg";
        std::string fname = ss.str();
        std::string jpgErrMsg;
        safeJpgWrite(l.pBitmapFinal, fname, 90, &jpgErrMsg); 
        writeOk=mTif->writeImage(l.pBitmapFinal->data);
      }
    }
    if (writeOk==false)
    {
      if (l.optOutGoogle)
      {
        std::cerr << "Failed to write jpeg tile '" << *l.pTileName << "' reason: " << mZip->getErrorMsg() << std::endl;
      }
      else if (l.optOutTif && l.tiled)
      {
        mTif->getErrMsg(errMsg);
        std::cerr << "Failed to write tif tile x=" << l.xDest << " y=" << l.yDest << " reason: " << errMsg << std::endl;
      }
      else if (l.optOutTif && !l.tiled)
      {
        mTif->getErrMsg(errMsg);
        std::cerr << "Failed to write tif image at tif level=" << l.outLevel << " reason: " << errMsg << std::endl;
      }
      l.error = true;
    }
  }
  if (l.totalSubTiles == 1)
  {
    safeBmpFree(&l.preOrientBitmap);
    safeBmpFree(&l.subTileBitmap);
    safeBmpFree(&l.subTileScaled);
    safeBmpFree(&l.sizedBitmap);
    safeBmpFree(&l.sizedBitmap2);
  }
  l.xDest += l.finalOutputCols2;
  l.xTileMap++;
}


void SlideConvertor::outputNextRow(SlideLevel& l)
{
  std::ostringstream yRootStream, yRootStreamZip;
  std::string dirPart1, dirPartZip1;
  std::string dirPart2, dirPartZip2;
      
  yRootStream << mFileNameOnly;
  dirPart1 = yRootStream.str();
  yRootStream << mPathSeparator << l.outLevel;
  dirPart2 = yRootStream.str();
  yRootStream << mPathSeparator << l.yTileMap;
  mYRoot = yRootStream.str();
  if (l.optDebug > 1)
  {
    // Create the google maps directory structure up to the the y tile
    if (!my_mkdir(dirPart1) || !my_mkdir(dirPart2) || !my_mkdir(mYRoot))
    {
      std::cerr << "Failed to add create directory '" << mYRoot << "'. Stopping!" << std::endl;
      l.error = true;
      return;
    }
  }
  yRootStreamZip << mFileNameOnly;
  dirPartZip1 = yRootStreamZip.str();
  yRootStreamZip << ZipFile::mZipPathSeparator << l.outLevel;
  dirPartZip2 = yRootStreamZip.str();
  yRootStreamZip << ZipFile::mZipPathSeparator << l.yTileMap;
  mYRootZip = yRootStreamZip.str();
  if (l.optOutGoogle) 
  {
    // Create the google maps directory structure up to the the y tile
    if (mZip->addDir(dirPartZip1)==-1 || mZip->addDir(dirPartZip2)==-1 || mZip->addDir(mYRootZip)==-1)
    {
      std::cerr << "Failed to add zip directory '" << mYRoot << "' to archive. Reason: " << mZip->getErrorMsg() << std::endl << "Stopping!" << std::endl;
      l.error = true;
      return;
    }
  }
  l.ySrc = (l.yOutTile * l.grabRowsB) + l.yStartSrc;
  l.xSrc = l.xStartSrc;
  l.xDest = 0;
  l.xTileMap = l.xStartTile;
  if (round(l.ySrc) + round(l.grabRowsB) < 1.0f)
  {
    std::cerr << "For some reason we have hit this! l.ySrc=" << l.ySrc << " l.yStartSrc=" << l.yStartSrc << " l.grabRowsB=" << l.grabRowsB << std::endl;
    l.yOutTile++;
    return;
  }
  for (l.xOutTile=0; l.xOutTile < l.xEndTile && round(l.xSrc) < l.srcTotalCols && l.error==false; l.xOutTile++) 
  {
    outputNextTile(l);
  }
  l.yDest += l.finalOutputRows2;
  l.yTileMap++;
  l.yOutTile++;
  l.ySrc = (l.yOutTile * l.grabRowsB) + l.yStartSrc;
  printPercDone(l);
}


void SlideConvertor::calcTilesPrep(SlideLevel& l)
{
  l.srcTotalCols = slide->getLevelCols(l.olympusLevel);
  l.srcTotalRows = slide->getLevelRows(l.olympusLevel);
  if (l.fillin && slide->checkLevel(2))
  {
    l.srcTotalColsL2 = slide->getLevelCols(2);
    l.srcTotalRowsL2 = slide->getLevelRows(2);
  }
  else if (l.fillin && slide->checkLevel(3))
  {
    l.srcTotalColsL2 = slide->getLevelCols(3);
    l.srcTotalRowsL2 = slide->getLevelRows(3);
  }
  else
  {
    l.fillin = false;
  }
  l.destTotalColsDec = (double) mBaseTotalCols / (double) l.magnifyX;
  l.destTotalRowsDec = (double) mBaseTotalRows / (double) l.magnifyY;
  l.destTotalCols = (int64_t) round(l.destTotalColsDec);
  l.destTotalRows = (int64_t) round(l.destTotalRowsDec);
  l.destTotalCols2 = (int64_t) round(mBaseTotalCols2 / l.magnifyX);
  l.destTotalRows2 = (int64_t) round(mBaseTotalRows2 / l.magnifyY);
  l.xScale=(double) l.srcTotalCols / (double) l.destTotalColsDec;
  l.yScale=(double) l.srcTotalRows / (double) l.destTotalRowsDec;
  l.xScaleReverse=(double) l.destTotalColsDec / (double) l.srcTotalCols;
  l.yScaleReverse=(double) l.destTotalRowsDec / (double) l.srcTotalRows;

  l.scaleMethodL2 = SAFEBMP_BEST_ENLARGE;
  l.L2Size=l.readColsL2 * l.readRowsL2 * 3;
  l.readOkL2=false;
  if (l.fillin && l.L2Size > 0 && l.pBitmapL2)
  {
    l.readOkL2=true;
    l.xScaleResize=(double) l.destTotalCols / (double) l.srcTotalColsL2;
    l.yScaleResize=(double) l.destTotalRows / (double) l.srcTotalRowsL2;
    if (l.xScaleResize < 1.0 || l.yScaleResize < 1.0)
    {
      l.scaleMethodL2=SAFEBMP_BEST_SHRINK;
    }
  }
  l.xScaleL2=(double) l.srcTotalColsL2 / (double) l.srcTotalCols;
  l.yScaleL2=(double) l.srcTotalRowsL2 / (double) l.srcTotalRows;
  if (l.tiled)
  {
    l.finalOutputCols=256;
    l.finalOutputRows=256;
    l.finalOutputCols2=256;
    l.finalOutputRows2=256;
    l.inputTileCols=256;
    l.inputTileRows=256;
    l.grabColsA=(double) l.inputTileCols * l.xScale;
    l.grabRowsA=(double) l.inputTileRows * l.yScale;
    l.grabColsB=(double) l.finalOutputCols * l.xScale;
    l.grabRowsB=(double) l.finalOutputRows * l.yScale;
    l.grabColsL2=ceil(256.0 * (double) l.srcTotalColsL2 / (double) l.destTotalCols);
    l.grabRowsL2=ceil(256.0 * (double) l.srcTotalRowsL2 / (double) l.destTotalRows);
  }
  else
  {
    l.finalOutputCols=(int) l.destTotalCols;
    l.finalOutputRows=(int) l.destTotalRows;
    l.finalOutputCols2=(int) l.destTotalCols2;
    l.finalOutputRows2=(int) l.destTotalRows2;
    l.inputTileCols=(int) l.destTotalCols2;
    l.inputTileRows=(int) l.destTotalRows2;
    l.grabColsA=(double) l.srcTotalCols;
    l.grabRowsA=(double) l.srcTotalRows;
    l.grabColsB=(double) l.srcTotalCols;
    l.grabRowsB=(double) l.srcTotalRows;
    l.grabColsL2=(double) l.srcTotalColsL2;
    l.grabRowsL2=(double) l.srcTotalRowsL2;
  }
  l.totalSubTiles = 1;
  int64_t totalGrabBytes = (int64_t) round(l.grabColsB) * (int64_t) round(l.grabRowsB) * 3;
  if (totalGrabBytes > l.optMaxMem)
  {
    do
    {
      l.totalSubTiles *= 2;
      totalGrabBytes = (int64_t) (ceil((double) l.grabColsB / (double) l.totalSubTiles) * ceil((double) l.grabRowsB / (double) l.totalSubTiles) * 3);
    }
    while (totalGrabBytes > l.optMaxMem);
    std::cout << "Using max memory " << (totalGrabBytes / (1024 * 1024)) << "mb max cols=" << round(l.grabColsB) << " x rows=" << round(l.grabRowsB) << " for pixel resizer." << std::endl;
    l.grabColsA = l.grabColsB / (double) l.totalSubTiles;
    l.grabRowsA = l.grabRowsB / (double) l.totalSubTiles;
  } 
  if (l.center && l.optOutGoogle)
  {
    calcCenters(l.outLevel, l.xLevelOffset, l.yLevelOffset);
  
    l.xStartTile = l.xLevelOffset / 256;
    l.xCenter = l.xLevelOffset % 256;
    l.xStartSrc = (double)(-l.xCenter) * l.xScale;
    l.outputLvlTotalCols = (int64_t) ceil((double) (l.xCenter + l.destTotalCols) / 256.0) * 256;

    l.yTileMap = l.yLevelOffset / 256;
    l.yCenter = l.yLevelOffset % 256;
    l.yStartSrc = (double)(-l.yCenter) * l.yScale;
    l.outputLvlTotalRows = (int64_t) ceil((double) (l.yCenter + l.destTotalRows) / 256.0) * 256;
  }
  else
  {
    l.xStartTile = 0;
    l.xCenter = 0;
    l.xStartSrc = 0.0;
    
    l.yTileMap = 0;
    l.yCenter = 0;
    l.yStartSrc = 0.0;
    if (l.tiled)
    {
      l.outputLvlTotalCols = (int64_t) ceil((double) l.destTotalCols / l.inputTileCols) * l.inputTileCols;
      l.outputLvlTotalRows = (int64_t) ceil((double) l.destTotalRows / l.inputTileRows) * l.inputTileRows;
    }
    else
    {
      l.outputLvlTotalCols = l.destTotalCols2;
      l.outputLvlTotalRows = l.destTotalRows2;
    } 
  }
  if (l.tiled)
  {
    l.xEndTile = (int64_t) ceil((double) l.outputLvlTotalCols / (double) l.inputTileCols);
    l.yEndTile = (int64_t) ceil((double) l.outputLvlTotalRows / (double) l.inputTileRows);
  }
  else
  {
    l.xEndTile = 1;
    l.yEndTile = 1;
  }  

  if (l.xScaleReverse < 1.0 || l.yScaleReverse < 1.0)
  {
    l.scaleMethod=SAFEBMP_BEST_SHRINK;    
  }
  else
  {
    l.scaleMethod=SAFEBMP_BEST_ENLARGE;
  }
}


int SlideConvertor::outputLevel(int olympusLevel, int magnification, int outLevel, int options, int64_t readColsL2, int64_t readRowsL2, safeBmp *pBitmapL2)
{
  SlideLevel l;
  std::ostringstream output;

  memset(&l, 0, sizeof(l));

  l.optInOlympusIni = mOptInOlympusIni;
  l.optOutGoogle = mOptOutGoogle;
  l.optOutTif = mOptOutTif;
  l.optBlend = mOptBlend;
  l.optDebug = mOptDebug;
  l.optMaxMem = mOptMaxMem;
  l.optGamma = mOptGamma;
  l.optUseGamma = mOptUseGamma;
  l.optLog = mOptLog;
  l.orientation = mOrientation;
  l.tiled = (options & LEVEL_TILED) ? true : false;
  l.center = mCenter;
  l.olympusLevel = olympusLevel;
  l.magnifyX=magnification;
  l.magnifyY=magnification;
  l.outLevel=outLevel;
  l.readColsL2 = readColsL2;
  l.readRowsL2 = readRowsL2;
  l.pBitmapL2 = pBitmapL2;
  l.bkgdColor=255;

  l.xBlendFactor = l.magnifyX; 
  l.yBlendFactor = l.magnifyY;
  l.fillin = (mOptBlend && l.olympusLevel < 2) ? true : false;

  calcTilesPrep(l);
  // Get the quality from the composite level (this does work as long as
  // the ini file specifies it (some ini files do, some don't)
  // Make sure the quality is at minimum the quality specific on the
  // command line
  l.optQuality=slide->getQuality(l.olympusLevel);
  if (l.optQuality == 0 || l.optQuality < mOptQuality)
  {
    l.optQuality = mOptQuality;
  }
  if (l.optOutGoogle)
  {
    output << "Google Maps Level=" << l.outLevel << " Input Level=" << l.olympusLevel << " Divisor of Base=" << l.magnifyX << std::endl;
    if (l.optLog) *logFile << output.str();
    std::cout << output.str();
  }
  else if (l.optOutTif)
  {
    output << "Tiff Level=" << l.outLevel << " Input Level=" << l.olympusLevel << " Divisor of Base=" << l.magnifyX << std::endl;
    if (l.optLog) *logFile << output.str();
    std::cout << output.str();
   
    std::string magStr = slide->getMagStr();
    double totalMag = slide->getMag();
    if (slide->getMagFound() == false || totalMag <= 0)
    {
      totalMag = 40;
      magStr = "40";
    }
    std::ostringstream oss;
    if (mBaseLevel==olympusLevel || l.tiled==false) 
    {
      oss << "|AppMag=" << magStr;
      if (slide->getTotalZLevels() > 1 && mZSteps == 1) 
      {
        oss << "|TotalDepth = " << slide->getTotalZLevels() << "\0";
      }
      else if (slide->getTotalZLevels() > 1 && mZSteps > 1)
      {
        oss << "|OffsetZ = " << (mZSteps-1) << "\0";
      }
    }
    std::string strAttributes=oss.str();
    if (mTif->setAttributes(3, 8, (int) l.destTotalCols2, (int) l.destTotalRows2, (l.tiled==true ? l.finalOutputCols : 0), (l.tiled==true ? l.finalOutputRows : 0), 1, l.optQuality)==false || mTif->setDescription(strAttributes, (int) mBaseTotalCols2, (int) mBaseTotalRows2)==false)
    {
      mTif->getErrMsg(errMsg);
      std::cerr << "Failed to write tif attributes: " << errMsg << std::endl; 
      return 4;
    }
  }
  l.error=false;
  time_t timeStart=0, timeLast=0;
  l.pZoomRes = safeBmpZoomResInit();
  try
  {
    safeBmpClear(&l.wholeTile);
    if (l.totalSubTiles > 1)
    {
      safeBmpAlloc2(&l.wholeTile, l.inputTileCols, l.inputTileRows);
    }
    l.pBitmapBlended = safeBmpAlloc(l.finalOutputCols2, l.finalOutputRows2);
    if (l.readOkL2 && l.optInOlympusIni && l.optBlend)
    {
      if (mTotalYSections == 0)
      {
        mTotalYSections = mBaseActualRows2;
        mySubSections=new BlendSection*[mTotalYSections];
        memset(mySubSections, 0, mTotalYSections*sizeof(BlendSection*));
        for (int64_t y = 0; y < mTotalYSections; y++)
        {
          if (mySubSections[y] != NULL)
          {
            std::cout << "NOT NULL @ y=" << y << std::endl;
          }
        }
        std::cout << "Done check!" << std::endl;
        ((CompositeSlide*)slide)->blendLevelsRegionScan(mySubSections, mTotalYSections, mOrientation);
      }
      l.totalYSections = mTotalYSections;
      l.ySubSections=mySubSections;
    }
    if (l.optLog)
    {
      *logFile << " xScale=" << l.xScale << " yScale=" << l.yScale;
      *logFile << " srcTotalCols=" << l.srcTotalCols << " srcTotalRows=" << l.srcTotalRows;
      *logFile << " destTotalCols=" << l.destTotalCols << " destTotalRows=" << l.destTotalRows;
      *logFile << std::endl;
    }
    if (l.optDebug > 1)
    {
      my_mkdir("vmslideconv.a");
      my_mkdir("vmslideconv.l2");
      my_mkdir("vmslideconv.l");
      my_mkdir("vmslideconv.z");
    }
    
    l.perc=0, l.percOld=0;
    l.onePercHit=false;
    timeStart = time(NULL);
    l.ySrc=l.yStartSrc;
    
    retractCursor();
    std::cout << "0% done...    " << std::flush;

    // Keep looping until the current composite pyramid level is read
    // or the resulting output pyramid is done or on error
    l.yOutTile = 0;
    while (l.yOutTile < l.yEndTile && round(l.ySrc) < l.srcTotalRows && l.error == false)
    {
      outputNextRow(l);
    }
    if (l.optOutTif)
    {
      bool success = mTif->writeDirectory();
      if (success == false)
      {
        const char *tifDirErrorMsg = "Fatal Error: Failed to write tif directory: ";
        mTif->getErrMsg(errMsg);
        std::cerr << tifDirErrorMsg << errMsg << std::endl;
        if (l.optLog) *logFile << tifDirErrorMsg << errMsg << std::endl;
        l.error = true;
      }
    }
  }
  catch (std::bad_alloc& ba)
  {
    (void) ba;
    const char *msg = "Fatal Error: Failed to allocate memory! Cannot continue!";
    std::cout << msg << std::endl;
    if (l.optLog) *logFile << msg << std::endl;
    l.error = true;
    exit(1);
  }
  safeBmpFree(l.pBitmapBlended);
  safeBmpFree(&l.wholeTile);
  safeBmpZoomResFree(l.pZoomRes);
  l.pBitmapBlended = NULL;
  timeLast = time(NULL);
  if (l.error==false)
  {
    std::cout << "Took " << timeLast - timeStart << " seconds for this level." << std::endl;
  }
  return (l.error==true ? 1 : 0); 
}


int SlideConvertor::checkFullL2(int64_t *pReadColsL2, int64_t *pReadRowsL2, safeBmp **pFullL2)
{
  *pFullL2 = NULL;
  *pReadColsL2=0;
  *pReadRowsL2=0;

  int64_t srcTotalColsL2 = 0;
  int64_t srcTotalRowsL2 = 0;
  if (slide->checkLevel(2))
  {
    srcTotalColsL2 = slide->getLevelCols(2);
    srcTotalRowsL2 = slide->getLevelRows(2);
  }
  else if (slide->checkLevel(3))
  {
    srcTotalColsL2 = slide->getLevelCols(3);
    srcTotalRowsL2 = slide->getLevelRows(3);
  }
  else
  {
    return 1;
  }

  if (srcTotalColsL2 <= 0 || srcTotalRowsL2 <= 0) return 1;
  if (!mpImageL2) return 2;
  if (mpImageL2->cols > 0 && mpImageL2->rows > 0 && mpImageL2->data) 
  {
    *pReadColsL2 = mpImageL2->cols;
    *pReadRowsL2 = mpImageL2->rows;
    *pFullL2 = mpImageL2;
    return 0;
  }
  return 1;
}


int SlideConvertor::convert2Tif()
{
  int error = 0;
  safeBmp* pFullL2Bitmap = 0; 
  int64_t readColsL2 = 0;
  int64_t readRowsL2 = 0;
 
  if (mValidObject == false) return 1;
 
  if (mOptBlend)
  {
    int status = checkFullL2(&readColsL2, &readRowsL2, &pFullL2Bitmap);
    switch (status)
    {
      case 0:
        break;
      case 1:
        std::cout << "Slide missing level 2 pyramid, continuing without one." << std::endl;
        mOptBlend = false;
        break;
      case 2:
        std::cout << "Fatal Error: Cannot allocate memory for Olympus level 2 pyramid." << std::endl;
        return 1;
      case 3:
        std::cout << "Failed reading level 2 pyramid, continuing without one." << std::endl;
        mOptBlend = false;
        break;
    }
  }
  int maxDivisor=128;
  while (maxDivisor > 16 && (mBaseTotalCols / maxDivisor < 2000 && mBaseTotalRows / maxDivisor < 2000)) 
  {
    maxDivisor /= 2;
  }; 
  //****************************************************************
  // Output the base level, thumbnail, 4x, 16x, and 32x levels 
  //****************************************************************
  int divisor = 1;
  int step = 1;
  int options = LEVEL_TILED; 
  while (divisor != maxDivisor && error==0)
  {
    int tiled = 1;
    int olympusLevel = 1;
    if (mOptBlend==false)
    {
      for (olympusLevel = 0; olympusLevel < 4; olympusLevel++)
      {
        if (slide->checkLevel(olympusLevel)) break;
      }
    }
    switch (step)
    {
      case 1:
        divisor = 1;
        if (slide->checkLevel(0))
        {
          olympusLevel = 0;
        }
        break;
      case 2:
        divisor = maxDivisor * 2;
        tiled = 0;
        break;
      case 3:
        divisor = 4;
        break;
      case 4:
        divisor = 16;
        break;
      case 5:
        if (maxDivisor / 2 >= 64)
        {
          divisor = maxDivisor / 2;
        }
        else
        {
          divisor = maxDivisor;
        }
        break;
      case 6:
        divisor = maxDivisor;
        break;
    }
    options = LEVEL_TILED * tiled;
    error=outputLevel(olympusLevel, divisor, step, options, readColsL2, readRowsL2, pFullL2Bitmap);
    step++;
  }
  if (error == 0 && step > 1)
  {
    std::cout << std::endl << "All Levels Completed." << std::endl;
  }
  return error;
}


int SlideConvertor::convert2Gmap()
{
  int error = 0;
  safeBmp* pFullL2Bitmap = 0; 
  int64_t readColsL2 = 0;
  int64_t readRowsL2 = 0;
  
  if (mValidObject == false) return 4;

  if (mOptBlend)
  {
    int status = checkFullL2(&readColsL2, &readRowsL2, &pFullL2Bitmap);
    switch (status)
    {
      case 0:
        break;
      case 1:
        std::cout << "Slide missing level 2 pyramid, continuing without one." << std::endl;
        mOptBlend = false;
        break;
      case 2:
        std::cout << "Fatal Error: Cannot allocate memory for Olympus level 2 pyramid." << std::endl;
        return 1;
      case 3:
        std::cout << "Failed reading level 2 pyramid, continuing without one." << std::endl;
        mOptBlend = false;
        break;
    }
  }
  //****************************************************************
  // Output each level, each level is 2^level size 
  //****************************************************************
  int divisor = 1 << mTopOutLevel;
  int outLevel = 0;

  while (outLevel <= mTopOutLevel && error==0)
  {
    int olympusLevel;
    if (mUseableLvl1)
    {
      if (divisor >= mxLvl1Divisor && divisor >= myLvl1Divisor)
      {
        olympusLevel = 1;
      }
      else
      {
        olympusLevel = 0;
      }
    }
    else 
    {
      for (olympusLevel = 0; olympusLevel < 4; olympusLevel++)
      {
        if (slide->checkLevel(olympusLevel)) break;
      }
    }
    error=outputLevel(olympusLevel, divisor, outLevel, LEVEL_TILED, readColsL2, readRowsL2, pFullL2Bitmap);
    divisor /= 2;
    outLevel++;
  }
  if (error==0 && outLevel > mTopOutLevel && outLevel > 0)
  {
    std::cout << std::endl << "All Levels Completed." << std::endl;
  }
  return error;
}


int SlideConvertor::open(std::string inputFile, std::string outputFile, std::string hostname, int options, int orientation)
{
  mValidObject = false;
  closeRelated();
  logFile = new std::ofstream;
  mOptInOlympusIni = options & CONV_IN_OLYMPUS_INI;
  mOptInAperioSvs = options & CONV_IN_APERIO_SVS;
  mOptOutTif = options & CONV_OUT_TIF;
  mOptOutGoogle = options & CONV_OUT_GOOGLE;
  mOrientation = orientation;
  mOptLog = options & CONV_LOG;
  mOptBlend = options & CONV_BLEND;
  int optOutJson = options & CONV_OUT_JSON;
  if (mOptLog)
  {
    logFile->open("vmslideconv.log");
  }
  struct stat statBuf;
  if (stat(inputFile.c_str(), &statBuf) == -1)
  {
    std::cerr << "Fatal Error: Input file or directory '" << inputFile << "' not found!" << std::endl;
    return 1;
  }
  if ((statBuf.st_mode & S_IFDIR))
  {
    mOptInOlympusIni = true;
    mOptInAperioSvs = false;
  }
  else
  {
    // Check filename for .ini extension
    size_t pos = inputFile.find(".INI");
    size_t pos2 = inputFile.find(".ini");
    if (pos2 != std::string::npos)
    {
      pos = pos2;
    }
    if (pos != std::string::npos)
    {
      size_t fnameLen = inputFile.length();
      char nextChar = 0;
      if (pos + 4 < fnameLen)
      {
        nextChar = inputFile[pos + 4];
      }
      if (nextChar == 0 || nextChar == '\'' || nextChar == '\"' || nextChar == ' ' || nextChar == '.')
      {
        mOptInOlympusIni = true;
        mOptInAperioSvs = false;
      }
    }
    // Check the file's header to see if it has a tiff signature
    std::ifstream checkFile;
    checkFile.open(inputFile.c_str(), std::ifstream::binary);
    if (checkFile.good() == false)
    {
      std::cerr << "Fatal Error: Failed to read header from file '" << inputFile << "'." << std::endl;
      return 1;
    }
    char header[4];
    checkFile.read(header, 4);
    if (checkFile.gcount() >= 4 && Tiff::testHeader((BYTE*)header, 4))
    {
      mOptInAperioSvs = true;
      mOptInOlympusIni = false;
    }
    else
    {
      mOptInAperioSvs = false;
      mOptInOlympusIni = true;
    }
    checkFile.close();
  } 

  if (mOptInOlympusIni)
  {
    slide = new CompositeSlide();
  }
  else
  {
    slide = new AperioSlide();
    mOptBlend = false;
  }  
  errMsg="";
  if (slide->open(inputFile.c_str(), options, mOptDebug)==false)
  {
    return 1;
  }
  if (mOptInOlympusIni && mOptBlend && slide->checkLevel(2))
  {
    mpImageL2 = slide->loadFullImage(2, orientation, 1.0, 1.0, false, mOptDebug, *logFile);
  }
  slide->setXYSwitched(orientation);
  mOutputFile = outputFile;
  mOutputDir = outputFile;
  std::size_t lastSlashIndex = mOutputFile.find_last_of("\\/");
  if (lastSlashIndex != std::string::npos)
  {
    if (lastSlashIndex+1 < mOutputFile.length())
    {
      mFileNameOnly = mOutputFile.substr(lastSlashIndex+1);
    }
    else
    {
      std::cerr << "Error: provided output '" << mOutputFile << "' must include destination zip file name." << std::endl;
      return 2;
    }
  }
  else
  {
    mFileNameOnly = mOutputFile;
  }
  std::size_t dot_index = mFileNameOnly.find_last_of(".");
  if (dot_index != std::string::npos)
  {
    mFileNameOnly.erase(dot_index);
  }

  if (mOptOutTif)
  {
    mOutputDir.append("_debug_jpgs");
    mCenter = false;
    mTif = new Tiff();
    if (mTif->createFile(outputFile)==false)
    {
      mTif->getErrMsg(errMsg);
      std::cerr << "Failed to create tiff file '" << outputFile << "'. Reason: " << errMsg << std::endl;
      return 2;
    }
  }
  else if (mOptOutGoogle)
  {
    mZip = new ZipFile();
    if (mZip->openArchive(outputFile.c_str(), OLY_APPEND_STATUS_CREATE) != 0)
    {
      std::cerr << "Failed to create zip file '" << outputFile << "'. Reason: " << mZip->getErrorMsg() << std::endl;
      return 2;
    }
    mZip->setCompression(OLY_DEF_COMPRESS_METHOD, OLY_DEF_COMPRESS_LEVEL);

    mCenter = true;
  }
  else
  {
    return 1;
  }
  for (mBaseLevel=0; mBaseLevel<4; mBaseLevel++)
  {
    if (slide->checkLevel(mBaseLevel))
    {
      mBaseTotalCols = slide->getLevelCols(mBaseLevel);
      mBaseTotalRows = slide->getLevelRows(mBaseLevel);
      mBaseTotalCols2 = (int64_t) ceil((double) mBaseTotalCols / (double) 256.0) * 256;
      mBaseTotalRows2 = (int64_t) ceil((double) mBaseTotalRows / (double) 256.0) * 256;
      mBaseActualCols = slide->getActualCols(mBaseLevel);
      mBaseActualRows = slide->getActualRows(mBaseLevel);
      mBaseActualCols2 = (int64_t) ceil((double) mBaseActualCols / (double) 256.0) * 256;
      mBaseActualRows2 = (int64_t) ceil((double) mBaseActualRows / (double) 256.0) * 256;
      break;
    }
  }
  mStep=0;
  mLastZLevel=-1;
  mMaxSide = 0;
  mTopOutLevel = 0;
  
  std::cout << "baseTotalCols=" << mBaseTotalCols << " baseTotalRows=" << mBaseTotalRows << std::endl;
  if (mBaseTotalCols > 0 && mBaseTotalRows > 0)
  {
    mValidObject = true;
    if (mOptOutGoogle)
    {
      for (mTopOutLevel = 0; mTopOutLevel < 20; mTopOutLevel++)
      {
        mMaxSide = (1 << mTopOutLevel) * 256;
        if (mMaxSide >= mBaseTotalCols && mMaxSide >= mBaseTotalRows)
        {
          std::cout << "Total Google Maps Levels: " << mTopOutLevel << " (because 2^" << mTopOutLevel << "*256=" << mMaxSide << ") " << std::endl;
          break;
        }
      }
    }
  }
  if (mValidObject && mOptOutGoogle && optOutJson)
  {
    std::ofstream jsonFile;
    std::string jsonName = outputFile;
    std::size_t dot_index2 = jsonName.find_last_of(".");
    if (dot_index2 != std::string::npos)
    {
      jsonName.erase(dot_index2);
    }
    jsonName.append(".json");
    jsonFile.open(jsonName.c_str());
    if (jsonFile.is_open())
    {
      jsonFile.exceptions(std::ofstream::failbit | std::ofstream::badbit);
      try
      {
        jsonFile << "{" << std::endl;
        jsonFile << "  " << "\"slideUrlFormat\": \"" << hostname << mFileNameOnly << "/{z}/{y}/{x}.jpg\"," << std::endl;
        jsonFile << "  " << "\"cols\": \"" << mBaseTotalCols << "\"," << std::endl;
        jsonFile << "  " << "\"rows\": \"" << mBaseTotalRows << "\"," << std::endl;
        jsonFile << "  " << "\"levels\": \"" << mTopOutLevel << "\"," << std::endl;
        jsonFile << "  " << "\"slideDepth\": \"" << slide->getMagStr() << "\"," << std::endl;
        jsonFile << "  " << "\"description\": \"" << slide->getHumanDesc() << "\"," << std::endl;
        jsonFile << "  " << "\"copyright\": \"" << slide->getCopyrightTxt() << "\"" << std::endl;
        jsonFile << "}" << std::endl;
        jsonFile.close();
      } 
      catch (std::ofstream::failure &e)
      {
        (void) e;
        std::cerr << "Failed to write json to file '" << jsonName << "'." << std::endl;
        mValidObject = false;
      }
    }
    else
    {
      std::cerr << "Failed to create json description file '" << jsonName << "'." << std::endl;
      mValidObject = false;
    }
  }
  mUseableLvl1 = false;
  mxLvl1Divisor = 0;
  myLvl1Divisor = 0;
  if (slide->checkLevel(0) && slide->checkLevel(1))
  {
    double xLvl1DivisorDbl = (double) slide->getLevelCols(0) / (double) slide->getLevelCols(1);
    double yLvl1DivisorDbl = (double) slide->getLevelRows(0) / (double) slide->getLevelRows(1);
    mxLvl1Divisor = (int)(ceil(xLvl1DivisorDbl * 10) / 10);
    myLvl1Divisor = (int)(ceil(yLvl1DivisorDbl * 10) / 10);
    if (mxLvl1Divisor >= 2 && myLvl1Divisor >= 2 && mxLvl1Divisor <= 8 && myLvl1Divisor <= 8)
    {
      mUseableLvl1 = true;
    }
  }
  return (mValidObject == true) ? 0 : 3;
}


void SlideConvertor::closeRelated()
{
  if (mpImageL2)
  {
    safeBmpFree(mpImageL2);
    mpImageL2 = NULL;
  }
  if (mTif)
  {
    mTif->close();
    delete mTif;
    mTif = NULL;
  }
  if (mZip)
  {
    mZip->closeArchive();
    delete mZip;
    mZip = NULL;
  }
  if (logFile)
  {
    logFile->close();
    delete logFile;
    logFile = NULL;
  }
  if (slide)
  {
    slide->close();
    delete slide;
    slide = NULL;
  }
  if (mySubSections)
  {
    blendLevelsFree(mySubSections, mTotalYSections);
  }
  if (mySubSections)
  {
    delete[] mySubSections;
    mySubSections = NULL;
    mTotalYSections = 0;
  }
  mStep=0;
  mZSteps=0;
  mLastDirection=-1;
  mLastZLevel=-1;
  mValidObject=false;
  mBaseTotalCols=0;
  mBaseTotalRows=0;
}


int main(int argc, char** argv)
{
  SlideConvertor slideConv;
  int error=0;
  std::string infile, outfile;
  std::string hostname = "";
  size_t hostnameLen = 0;

  int optInOlympusIni = getBoolOpt(SLIDE_DEF_IN_OLYMPUS_INI);
  int optInAperioSvs = getBoolOpt(SLIDE_DEF_IN_APERIO_SVS);
  int optOutGoogle = getBoolOpt(SLIDE_DEF_OUT_GOOGLE);
  int optOutTif = getBoolOpt(SLIDE_DEF_OUT_TIF);
  int optBlend = getBoolOpt(SLIDE_DEF_BLEND);
  int optHighlight = getBoolOpt(SLIDE_DEF_HIGHLIGHT);
  int optLog = getBoolOpt(SLIDE_DEF_LOG); 
  int optOutJson = getBoolOpt(SLIDE_DEF_OUT_JSON);
  int optQuality = SLIDE_DEF_QUALITY;
  int optDebug = SLIDE_DEF_DEBUG;
  int optMaxJpegCache = SLIDE_DEF_MAX_JPEG_CACHE;
  int64_t optMaxMem = SLIDE_DEF_MAX_MEM; 
  double optGamma = 1.0f;
  int optRotate = 0;
  int optUseGamma = 0;
  int allOptions = 0;
  int optSuppressJson = 0;
  
  const char fullSyntax[] =
"Usage: vmslideconv [OPTION] <inputFileNameOrDirectory> <outputFile>\n"
"Input file or directory is auto-detected. If it is a directory it is opened\n"
"as an Olympus ini dataset. If the input file has a tiff header, it is \n"
"processed as an aperio tiff file, otherwise assumed to be an Olympus ini\n"
"dataset.\n"
"Main Output Flags:\n"
"  -g, --google               Output to google maps format zipped. Default " 
SLIDE_DEF_OUT_GOOGLE ".\n"
"  -t, --tif                  Output to tif file instead. Default " 
SLIDE_DEF_OUT_TIF ".\n\n"
"Other Optional Flags:\n"
"  -a, --gamma                Alter gamma. Gamma less than one is darker, \n"
"                             greater than one is brighter. 1.5 is 50% percent\n"
"                             brighter 0.5 is 50% darker. Default no extra\n"
"                             gamma processing is done.\n"
"  -b, --blend                Blend the top level with the middle level. \n"
"                             Only applicable with Olympus ini datasets.\n"
"                             Default " 
SLIDE_DEF_BLEND ".\n"
"  -d, --debug=x              Debug mode, output debugging info and files. The\n"
"                             higher the more debugging output. Sets logging\n"
"                             on as well if greater than 1. Default " 
xstringfy(SLIDE_DEF_DEBUG) ".\n"
"  -h, --host=                Set hostname or folder prefix name in JSON file\n"
"                             output file for Google Maps output.\n"
"  -j, --max-jpeg-cache=x     Specify max jpeg cache size in megabytes.Don't\n"
"                             specify this unless you run out of memory or\n"
"                             looking to decrease the time to output.\n"
"                             Default " 
xstringfy(SLIDE_DEF_MAX_JPEG_CACHE) "mb.\n"
"  -l, --log                  Log general information about the slide. \n"
"                             Default " 
SLIDE_DEF_LOG ".\n"
"  -m, --max-mem=x            Specify max memory size of pixel rescaler or\n"
"                             resizer in megabytes. Don't specify this unless\n"
"                             you run out of memory or want to increase\n"
"                             performance. Default " 
xstringfy(SLIDE_DEF_MAX_MEM) "mb.\n"
"  -q, --quality=x            Set minimal jpeg quality percentage. Default " 
xstringfy(SLIDE_DEF_QUALITY) "%.\n"
"  -r, --rotate=x             Set orientation of slide or rotate the entire\n"
"                             by x degrees. Currently only -90, 90, 180, or\n"
"                             270 degree rotation is supported.\n"
"  -s, --suppress-json        Suppress output of JSON description file.\n"
"                             By default an output file with a .json\n"
"                             extention is created with Google Maps Output.\n\n";

  if (argc < 3)
  {
    std::cerr << fullSyntax;
    return 1;
  }
  int opt;
  int optIndex = 0;
  bool invalidOpt = false;
  char emptyString[] = "";
  static struct option longOptions[] =
    {
      // Main output arguments
      {"google",            no_argument,        0,             'g'},
      {"tiff",              no_argument,        0,             't'},
      
      // Optional output arguments
      {"gamma",             required_argument,  0,             'a'},
      {"blend",             required_argument,  0,             'b'},
      {"debug",             required_argument,  0,             'd'},
      {"host",              required_argument,  0,             'h'},
      {"max-jpeg-cache",    required_argument,  0,             'j'},
      {"log",               no_argument,        0,             'l'},
      {"max-mem",           required_argument,  0,             'm'},   
      {"olympus",           no_argument,        0,             'o'},
      {"quality",           required_argument,  0,             'q'},
      {"rotate",            required_argument,  0,             'r'},
      {"suppress-json",     no_argument,        0,             's'},
      {0,                   0,                  0,              0 }
    };
  
  while((opt = getopt_long(argc, argv, "gta:b:d:h:j:lm:oq:r:s", longOptions, &optIndex)) != -1)
  {
    if (optarg == NULL) optarg = emptyString;
    switch (opt)
    {
      case 'a':
        optGamma = getDoubleOpt(optarg, invalidOpt);
        optUseGamma = 1;
        break;
      case 'b':
        optBlend = getBoolOpt(optarg, invalidOpt);
        break;
      case 'd':
        optDebug = getIntOpt(optarg, invalidOpt);
        break;
      case 'g':
        optOutGoogle = getBoolOpt(optarg, invalidOpt);
        if (optOutGoogle) optOutTif=0;
        break;
      case 'h':
        hostname = getStrOpt(optarg, invalidOpt);        
        hostnameLen = hostname.length();
        if (hostnameLen > 0 && hostname[hostnameLen-1] != '/')
        {
          hostname.append(1, '/');
        }
        break;
      case 'j':
        optMaxJpegCache = getIntOpt(optarg, invalidOpt);
        break;
      case 'l':
        optLog = getBoolOpt(optarg, invalidOpt);
        break;
      case 'm':
        optMaxMem = getIntOpt(optarg, invalidOpt);
        break;
      case 'o':
        optInOlympusIni = getBoolOpt(optarg, invalidOpt);
        break;
      case 'q':
        optQuality = getIntOpt(optarg, invalidOpt);
        break;
      case 'r':
        optRotate = getIntOpt(optarg, invalidOpt);
        break;
      case 's':
        optSuppressJson = getBoolOpt(optarg, invalidOpt);
        optOutJson = optSuppressJson ? 0 : 1; 
        break;
      case 't':
        optOutTif = getBoolOpt(optarg, invalidOpt);
        if (optOutTif) optOutGoogle=0;
        break;
      case '?':
        if (infile.length() == 0)
        {
          infile=optarg;
        }
        else if (outfile.length() == 0)
        {
          outfile=optarg;
        }
        else
        {
          invalidOpt = true;
        }
        break;
    }
    if (invalidOpt)
    {
      std::cerr << fullSyntax;
      return 1;
    } 
  }
  for (; optind < argc; optind++)
  {
    if (infile.length() == 0)
    {
      infile=argv[optind];
    }
    else if (outfile.length() == 0)
    {
      outfile=argv[optind];
    }
  }
  if (optDebug > 1) optLog = 1;
  
  if (infile.length() == 0 || outfile.length() == 0)
  {
    std::cerr << fullSyntax;
    return 1;
  }
  allOptions = 
            (optBlend * CONV_BLEND) |
            (optHighlight * CONV_HIGHLIGHT) |
            (optLog * CONV_LOG) |
            (optUseGamma * CONV_CUSTOM_GAMMA) |
            (optOutJson * CONV_OUT_JSON) | 
            (optOutGoogle * CONV_OUT_GOOGLE) |
            (optOutTif * CONV_OUT_TIF) |
            (optInOlympusIni * CONV_IN_OLYMPUS_INI) |
            (optInAperioSvs * CONV_IN_APERIO_SVS);

  if ((allOptions & (CONV_OUT_TIF | CONV_OUT_GOOGLE))==0)
  {
    std::cerr << "No output format specified. Please use -g or --google for google maps output or -t or --tiff for tif output." << std::endl;
    return 1;
  }
  if ((allOptions & CONV_OUT_TIF) && (allOptions & CONV_OUT_GOOGLE))
  {
    std::cerr << "Please specify either -g or --google for google maps output or -t or --tiff for tif output." << std::endl;
    return 1;
  }
  if (optMaxMem < 32)
  {
    std::cerr << "Max resize memory must be a size greater than or equal to 32 megabytes." << std::endl;
    return 1;
  }
  if (optMaxJpegCache == 0)
  {
    optMaxJpegCache = 1;
  }

  if (allOptions & CONV_OUT_GOOGLE)
  {
    std::cout << "Output format: Google Maps Zipped" << std::endl;
  }
  if (allOptions & CONV_OUT_TIF)
  {
    std::cout << "Output format: TIFF/SVS" << std::endl;
  }
  if (optRotate != 0)
  {
    std::cout << "Slide Orientation: " << optRotate << " degrees" << std::endl;
  }  
  if (allOptions & CONV_CUSTOM_GAMMA)
  {
    std::cout << "Custom gamma set: " << optGamma << std::endl;
  }

  if (optDebug > 0)
  {
    if (optInOlympusIni)
    {
      std::cout << "Set input is Olympus Ini Directory" << std::endl;
    }
    if (optRotate == 0)
    {
      std::cout << "Slide Orientation: Normal" << std::endl;
    }
    std::cout << "Set logging: " << boolInt2Txt(allOptions & CONV_LOG) << std::endl;
    std::cout << "Set debug level: " << optDebug << std::endl;
    std::cout << "Set minimum quality: " << optQuality << std::endl;
    std::cout << "Set border highlight: " << boolInt2Txt(allOptions & CONV_HIGHLIGHT) << std::endl;
    std::cout << "Set blend levels: " << boolInt2Txt(allOptions & CONV_BLEND) << std::endl;
    std::cout << "Set maximum resize/scale memory: " << optMaxMem << "mb " << std::endl;
    std::cout << "Set maximum jpeg cache memory: " << optMaxJpegCache << "mb " << std::endl;
  }

  jpgCache.setMaxOpen(optMaxJpegCache);

  safeBmpEnvSetup(optDebug);
 
  slideConv.setDebugLevel(optDebug);
  error=slideConv.open(infile.c_str(), outfile.c_str(), hostname, allOptions, optRotate);
  slideConv.setGamma(allOptions & CONV_CUSTOM_GAMMA, optGamma);
  slideConv.setQuality(optQuality);
  slideConv.setMaxMem(optMaxMem * 1024 * 1024);

  if (error==0)
  {
    if (allOptions & CONV_OUT_TIF)
    {
      error = slideConv.convert2Tif();
    }
    else if (allOptions & CONV_OUT_GOOGLE)
    {
      error = slideConv.convert2Gmap();
    }
  }
  else if (error>0) 
  {
    if (error==1)
    {
      std::cerr << "Failed to open " << infile << std::endl;
    }
    else if (error==2)
    {
      // failed to create tiff file, error already outputted
    }
    else if (error==3)
    {
      std::cerr << "No valid levels found." << std::endl;
    }
    error++;
  }
  slideConv.closeRelated();
  
  safeBmpEnvCleanup();
  return error;
}


std::string stdStrToUpper(std::string str)
{
  std::string newStr = str;

  size_t endStr = str.length();
  for (size_t i = 0; i < endStr; i++)
  {
    newStr[i] = (char) toupper(str[i]);
  }
  return newStr;
}
