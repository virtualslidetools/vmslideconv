#include "aperio.h"

std::string stdStrToUpper(std::string);


int AperioSlide::getTotalLevels()
{
  return (mValidObject) ? mTif.directorySize() : 0;
}


bool AperioSlide::checkLevel(int level)
{
  return (mValidObject && level < mTif.directorySize() && level >= 0); 
}


bool AperioSlide::checkZLevel(int level, int direction, int zLevel) 
{ 
  (void) level;
  (void) direction;
  (void) zLevel;
  return false; 
}


bool AperioSlide::isPreviewSlide(int level) 
{ 
  (void) level;
  return false; 
}


int AperioSlide::getTotalZLevels() 
{ 
  return 0; 
}


int AperioSlide::getTotalBottomZLevels() 
{ 
  return 0; 
}


int AperioSlide::getTotalTopZLevels() 
{ 
  return 0; 
}


int AperioSlide::getQuality(int level) 
{
  (void) level;
  return 70; 
}


int64_t AperioSlide::getPixelCols(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->tileCols : 0;
}


int64_t AperioSlide::getPixelRows(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->tileLength : 0; 
}


int64_t AperioSlide::getActualCols(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->cols : 0; 
}


int64_t AperioSlide::getActualRows(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->length : 0; 
}


int64_t AperioSlide::getLevelCols(int level) 
{ 
  int64_t fullCols = 0;
  if (mValidObject && level < mTif.directorySize() && level >= 0)
  {
    fullCols = (mXYSwitched ? mTif.at(level)->length : mTif.at(level)->cols); 
  }
  return fullCols;
}


int64_t AperioSlide::getLevelRows(int level) 
{ 
  int64_t fullRows = 0;
  if (mValidObject && level < mTif.directorySize() && level >= 0)
  {
    fullRows = (mXYSwitched ? mTif.at(level)->cols : mTif.at(level)->length); 
  }
  return fullRows;
}


double AperioSlide::getXAdj(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->xAdj : 1; 
}


double AperioSlide::getYAdj(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->yAdj : 1; 
}


int64_t AperioSlide::getTotalTiles(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->totalTiles : 0; 
}


bool AperioSlide::open(const std::string& srcFileName, int options, int optDebug)
{
  (void) options;

  if (mValidObject)
  {
    mTif.close();
    baseClearAttribs();
  }
  bool success = mTif.open(srcFileName);
  if (success)
  {
    mTif.setDebugMode((optDebug ? true : false));
    mBaseCols = mTif.getActualCols();
    mBaseRows = mTif.getActualRows();
    parseMagFromDesc(mTif.at(0)->description); 
    mValidObject = true;
  }
  return success;
}  


bool AperioSlide::read(safeBmp *pDestBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t cols, int64_t rows, int64_t* pReadCols, int64_t* pReadRows)
{
  (void) direction;
  (void) zLevel;
  // mTif.read initializes pReadCols and pReadRows right away with zero
  return mTif.read(pDestBmp, level, x, y, cols, rows, pReadCols, pReadRows);
}


safeBmp* AperioSlide::loadFullImage(int level, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::ofstream& logFile)
{
  safeBmp *pOrgBmp = NULL;
  safeBmp *pRotatedBmp = NULL;
  safeBmp *pZoomedBmp = NULL;
  safeBmp *pFinalBmp = NULL;

  (void) optDebug;
  (void) logFile;

  if (checkLevel(level)==false) return NULL;
  
  TiffDirAttribs* pTifConf = mTif.at(level);

  int64_t orgCols = pTifConf->cols;
  int64_t orgRows = pTifConf->length;
  int64_t readCols = 0;
  int64_t readRows = 0;
  int64_t newCols = 0;
  int64_t newRows = 0;
  
  switch (orientation)
  {
  case 0:
  case 180:
    newCols = pTifConf->cols;
    newRows = pTifConf->length;
    break;
  case -90:
  case 90:
  case 270:
    newCols = pTifConf->length;
    newRows = pTifConf->cols;
    break;
  default:
    return NULL;
  }
  pOrgBmp = safeBmpAlloc(orgCols, orgRows);

  read(pOrgBmp, level, 0, 0, 0, 0, orgCols, orgRows, &readCols, &readRows);

  if (orientation == 180 || orientation == -90 || orientation == 90 || orientation == 270)
  {
    pRotatedBmp = safeBmpAlloc(newCols, newRows);
    safeBmpRotate(pRotatedBmp, pOrgBmp, orientation);
    safeBmpFree(pOrgBmp);
    pOrgBmp = NULL;
    pFinalBmp = pRotatedBmp;
  }
  else
  {
    pFinalBmp = pOrgBmp;
  }
  if (useZoom && (xZoomOut != 1.0 || yZoomOut != 1.0))
  {
    safeBmpZoomRes* pZoomRes = safeBmpZoomResInit();
    int64_t zoomedCols = (int64_t) ceil(newCols * xZoomOut);
    int64_t zoomedRows = (int64_t) ceil(newRows * yZoomOut);
    int scaleMethod = SAFEBMP_BEST_ENLARGE;
    if (xZoomOut < 1.0 || yZoomOut < 1.0)
    {
      scaleMethod = SAFEBMP_BEST_SHRINK;
    }
    pZoomedBmp = safeBmpZoom(pFinalBmp, zoomedCols, zoomedRows, xZoomOut, yZoomOut, scaleMethod, pZoomRes);
    safeBmpFree(pFinalBmp);
    safeBmpZoomResFree(pZoomRes);
    pFinalBmp = pZoomedBmp;
  }
  return pFinalBmp;
}


bool AperioSlide::parseMagFromDesc(std::string desc)
{
  mMagFound = false;
  mMag = 0;
  mMagStr = "";
  std::string upperDesc = stdStrToUpper(desc);
  size_t pos = 0;
  size_t endStr = upperDesc.length();
  while (mMagFound == false && pos < endStr)
  {
    pos = upperDesc.find("APPMAG", pos);
    if (pos == std::string::npos) break;
    pos += 6;
    size_t endSection = upperDesc.find("|", pos);
    if (endSection == std::string::npos) 
    {
      endSection = endStr;
    }
    size_t endNum = std::string::npos;
    size_t startNum = std::string::npos;
    /* |AppMag = 40|Stuff = 2|Etc| 10-12 */
    for (; pos < endSection; pos++)
    {
      char k = upperDesc[pos];
      if (isdigit(k) || (k == '.' && startNum != std::string::npos))
      {
        if (startNum == std::string::npos) startNum = pos;
      }
      else if (startNum != std::string::npos && endNum == std::string::npos)
      {
        endNum = pos;
      }
    } 
    if (startNum != std::string::npos)
    {
      if (endNum == std::string::npos) endNum = endSection;
      mMagStr = upperDesc.substr(startNum, endNum-startNum);
      mMag = atof(mMagStr.c_str());
      mMagFound = true;
    }
  }
  return mMagFound;
}
