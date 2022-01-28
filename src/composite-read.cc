#include <iostream>
#include <cstring>
#include <vector>
#include "imagesupport.h"
#include "jpgcachesupport.h"
#include "composite.h"
#include "safebmp.h"

extern JpgCache jpgCache;

bool CompositeSlide::read(safeBmp* pBmpDest, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t width, int64_t height, int64_t *pReadWidth, int64_t *pReadHeight)
{
  *pReadWidth = 0;
  *pReadHeight = 0;
  if (checkZLevel(level, direction, zLevel)==false || checkLevel(level)==false)
  {
    return false;
  }
  int64_t actualWidth = mEtc[level]->mTotalWidth;
  int64_t actualHeight = mEtc[level]->mTotalHeight;
  if (x>actualWidth || y>actualHeight)
  {
    std::cerr << "Warning: in CompositeSlide::read: x or y out of bounds: x=" << x << " y=" << y << std::endl;
    return true;
  }
  if (width <= 0 || height <= 0)
  {
    std::cerr << "Warning: in CompositeSlide::read: width or height out of bounds: width=" << width << " height=" << height << std::endl;
    return true;
  } 
  if (x+width < 1 || y+width < 1)
  {
    return true;
  }
  int64_t maxWidth=width;
  int64_t maxHeight=height;
  if (x+width>actualWidth)
  {
    maxWidth=actualWidth-x;
  }
  if (y+height>actualHeight)
  {
    maxHeight=actualHeight-y;
  }
 
  JpgIniConf* pConf=mEtc[level];
  int64_t fileWidth, fileHeight;
/*
  if (mOrientation == 90 || mOrientation == -90 || mOrientation == 270)
  {
    fileWidth=pConf->mPixelHeight;
    fileHeight=pConf->mPixelWidth;
  }
  else
  {
*/
    fileWidth=pConf->mPixelWidth;
    fileHeight=pConf->mPixelHeight;
//  }
  int64_t widthGrab=0, heightGrab=0;
  int64_t totalTilesRead=0;
  for (int64_t tileNum=0; tileNum<pConf->mTotalTiles; tileNum++)
  {
    if (zLevel > 0 && direction > 0 && pConf->mxyArr[tileNum].mzStack[direction-1][zLevel] == false) continue;
    int64_t xFilePos=pConf->mxyArr[tileNum].mxPixel;
    int64_t yFilePos=pConf->mxyArr[tileNum].myPixel;
    if (((x<xFilePos && x+maxWidth>xFilePos) || (x>=xFilePos && x<xFilePos+fileWidth)) &&
        ((y<yFilePos && y+maxHeight>yFilePos) || (y>=yFilePos && y<yFilePos+fileHeight)))
    {
      Jpg *pJpg;
      int64_t xRead=0;
      int64_t xWrite=xFilePos-x;
      widthGrab=(x+maxWidth)-xFilePos;
      if (xWrite<0)
      {
        xWrite=0;
        xRead=x-xFilePos;
        widthGrab=fileWidth-xRead;
        if (widthGrab>maxWidth)
        {
          widthGrab=maxWidth;
        }
      }
      int64_t yRead=0;
      int64_t yWrite=yFilePos-y;
      heightGrab=(y+maxHeight)-yFilePos;
      if (yWrite<0)
      {
        yWrite=0;
        yRead=y-yFilePos;
        heightGrab=fileHeight-yRead;
        if (heightGrab>maxHeight)
        {
          heightGrab=maxHeight;
        }
      }
      if (yRead+heightGrab>fileHeight)
      {
        heightGrab=fileHeight-yRead;
      }
      if (xRead+widthGrab>fileWidth)
      {
        widthGrab=fileWidth-xRead;
      }
      std::string& fileName=(direction > 0 ? pConf->mxyArr[tileNum].mFileName[direction-1][zLevel] : pConf->mxyArr[tileNum].mBaseFileName);
      safeBmp bmpSrc;
      safeBmpAlloc2(&bmpSrc, widthGrab, heightGrab);
      pJpg=jpgCache.open(fileName, false);
      if (pJpg && pJpg->isValidObject() && pJpg->read(&bmpSrc, xRead, yRead, widthGrab, heightGrab))
      {
        safeBmpCpy(pBmpDest, xWrite, yWrite, &bmpSrc, 0, 0, bmpSrc.width, bmpSrc.height);
        totalTilesRead++;
        if (level==2 && mOptBorder)
        {
          const int samplesPerPixel = 3;
          drawBorder(pBmpDest->data, samplesPerPixel, x, y, maxWidth, maxHeight, level); 
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
  *pReadWidth=maxWidth;
  *pReadHeight=maxHeight;
  return true;
}
