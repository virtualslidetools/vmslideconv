#include <iostream>
#include <cstring>
#include <vector>
#include "imagesupport.h"
#include "jpgcachesupport.h"
#include "composite.h"
#include "safebmp.h"

extern JpgCache jpgCache;

bool CompositeSlide::read(safeBmp* pBmpDest, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t cols, int64_t rows, int64_t *pReadCols, int64_t *pReadRows)
{
  *pReadCols = 0;
  *pReadRows = 0;
  if (checkZLevel(level, direction, zLevel)==false || checkLevel(level)==false)
  {
    return false;
  }
  int64_t actualCols = mEtc[level]->mTotalCols;
  int64_t actualRows = mEtc[level]->mTotalRows;
  if (x>actualCols || y>actualRows)
  {
    std::cerr << "Warning: in CompositeSlide::read: x or y out of bounds: x=" << x << " y=" << y << std::endl;
    return true;
  }
  if (cols <= 0 || rows <= 0)
  {
    std::cerr << "Warning: in CompositeSlide::read: cols or rows out of bounds: cols=" << cols << " rows=" << rows << std::endl;
    return true;
  } 
  if (x+cols < 1 || y+cols < 1)
  {
    return true;
  }
  int64_t maxCols=cols;
  int64_t maxRows=rows;
  if (x+cols>actualCols)
  {
    maxCols=actualCols-x;
  }
  if (y+rows>actualRows)
  {
    maxRows=actualRows-y;
  }
 
  JpgIniConf* pConf=mEtc[level];
  int64_t fileCols, fileRows;
/*
  if (mOrientation == 90 || mOrientation == -90 || mOrientation == 270)
  {
    fileCols=pConf->mPixelRows;
    fileRows=pConf->mPixelCols;
  }
  else
  {
*/
    fileCols=pConf->mPixelCols;
    fileRows=pConf->mPixelRows;
//  }
  int64_t colsGrab=0, rowsGrab=0;
  int64_t totalTilesRead=0;
  for (int64_t tileNum=0; tileNum<pConf->mTotalTiles; tileNum++)
  {
    if (zLevel > 0 && direction > 0 && pConf->mxyArr[tileNum].mzStack[direction-1][zLevel] == false) continue;
    int64_t xFilePos=pConf->mxyArr[tileNum].mxPixel;
    int64_t yFilePos=pConf->mxyArr[tileNum].myPixel;
    if (((x<xFilePos && x+maxCols>xFilePos) || (x>=xFilePos && x<xFilePos+fileCols)) &&
        ((y<yFilePos && y+maxRows>yFilePos) || (y>=yFilePos && y<yFilePos+fileRows)))
    {
      Jpg *pJpg;
      int64_t xRead=0;
      int64_t xWrite=xFilePos-x;
      colsGrab=(x+maxCols)-xFilePos;
      if (xWrite<0)
      {
        xWrite=0;
        xRead=x-xFilePos;
        colsGrab=fileCols-xRead;
        if (colsGrab>maxCols)
        {
          colsGrab=maxCols;
        }
      }
      int64_t yRead=0;
      int64_t yWrite=yFilePos-y;
      rowsGrab=(y+maxRows)-yFilePos;
      if (yWrite<0)
      {
        yWrite=0;
        yRead=y-yFilePos;
        rowsGrab=fileRows-yRead;
        if (rowsGrab>maxRows)
        {
          rowsGrab=maxRows;
        }
      }
      if (yRead+rowsGrab>fileRows)
      {
        rowsGrab=fileRows-yRead;
      }
      if (xRead+colsGrab>fileCols)
      {
        colsGrab=fileCols-xRead;
      }
      std::string& fileName=(direction > 0 ? pConf->mxyArr[tileNum].mFileName[direction-1][zLevel] : pConf->mxyArr[tileNum].mBaseFileName);
      safeBmp bmpSrc;
      safeBmpAlloc2(&bmpSrc, colsGrab, rowsGrab);
      pJpg=jpgCache.open(fileName, false);
      if (pJpg && pJpg->isValidObject() && pJpg->read(&bmpSrc, xRead, yRead, colsGrab, rowsGrab))
      {
        safeBmpCpy(pBmpDest, xWrite, yWrite, &bmpSrc, 0, 0, bmpSrc.cols, bmpSrc.rows);
        totalTilesRead++;
        if (level==2 && mOptBorder)
        {
          const int samplesPerPixel = 3;
          drawBorder(pBmpDest->data, samplesPerPixel, x, y, maxCols, maxRows, level); 
        }
      }
      else
      {
        std::string errMsg;
        pJpg->getErrMsg(errMsg);
        std::cerr << "Warning: failed to read " << fileName << ": " << errMsg << std::endl;
      }
      safeBmpFree(&bmpSrc);
    }
  }
  *pReadCols=maxCols;
  *pReadRows=maxRows;
  return true;
}
