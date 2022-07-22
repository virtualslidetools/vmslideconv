#include <iostream>
#include <cstring>
#include <vector>
#include <assert.h>
#include "composite.h"
#include "blendbkgd.h"

void CompositeSlide::blendLevelsRegionScan(BlendSection** yFreeMap, int64_t ySize, int orientation) 
{
  if (yFreeMap == NULL) return;
  
  int baseLevel = 0;
  while (baseLevel < 4)
  {
    if (checkLevel(baseLevel)) break;
    baseLevel++;
  }
  JpgIniConf *pBaseConf = mEtc[baseLevel];
  
  int64_t baseTotalCols = getLevelCols(baseLevel);
  int64_t totalCols = pBaseConf->mTotalCols;
  int64_t totalRows = pBaseConf->mTotalRows;
  int64_t pixelCols = pBaseConf->mPixelCols;
  int64_t pixelRows = pBaseConf->mPixelRows;
  int64_t stdFileCols = pBaseConf->mPixelCols;
  int64_t stdFileRows = pBaseConf->mPixelRows;
  if (orientation == 90 || orientation == 270 || orientation == -90) 
  {
    stdFileRows = pixelCols;
    stdFileCols = pixelRows;
  }
  for (int64_t y=0; y < ySize; y++)
  {
    BlendSection *xTail = new BlendSection(0);
    xTail->setFree(baseTotalCols);
    yFreeMap[y] = xTail;
  }
 
  int64_t fileCols=stdFileCols;
  for (int64_t tileNum=0; tileNum<pBaseConf->mTotalTiles; tileNum++)
  {
    int64_t fileRows = stdFileRows;
    int64_t xPixel = pBaseConf->mxyArr[tileNum].mxPixel;
    int64_t yPixel = pBaseConf->mxyArr[tileNum].myPixel;
    int64_t x2 = pBaseConf->mxyArr[tileNum].mxPixel;
    int64_t y2 = pBaseConf->mxyArr[tileNum].myPixel;
    if (orientation == 0)
    {
      // place holder
    }  
    else if (orientation == 90)
    {
      x2 = (totalRows - yPixel) - pixelRows;
      y2 = xPixel;
    }
    else if (orientation == -90 || orientation == 270)
    {
      x2 = yPixel;
      y2 = (totalCols - xPixel) - pixelCols;
    }
    else if (orientation == 180)
    {
      x2 = (totalCols - xPixel) - pixelCols;
      y2 = (totalRows - yPixel) - pixelRows;
    }
    int64_t x3 = x2 + fileCols;
    int64_t y3 = y2 + fileRows;
    int64_t y = y2;
    if (y < 0)
    {
      fileRows += y;
      if (fileRows <= 0) continue;
      y = 0;
    }
    else if (y >= ySize)
    {
      continue;
    }
    if (y3 > ySize)
    {
      y3 = y;
    }
    while (y < y3)
    {
      BlendSection *xTail = yFreeMap[y];
      BlendSection *xNext = NULL;
      BlendSection *xPrevious = NULL;
      if (xTail == NULL)
      {
        xTail = new BlendSection(0);
        xTail->setFree(baseTotalCols);
        yFreeMap[y] = xTail;
      }
      while (xTail)
      {
        int64_t tailStart = xTail->getStart();
        int64_t tailLen = xTail->getFree();
        int64_t tailEnd = tailStart + tailLen;
        xNext = xTail->getNext();
        if (x2 >= tailStart && x2 < tailEnd)
        {
          if (x2 == tailStart)
          {
            delete xTail;
            if (xPrevious == NULL)
            {
              yFreeMap[y] = xNext;
            }
            else
            {
              xPrevious->setNext(xNext);
            }
            xTail = xPrevious;
          }
          else
          {
            xTail->setFree(x2 - tailStart);
          }
          if (x3 < tailEnd)
          {
            BlendSection *xNew = new BlendSection(x3);
            xNew->setFree(tailEnd - x3);
            xNew->setNext(xNext);
            if (xTail == NULL)
            {
              yFreeMap[y] = xNew;
            }
            else
            {
              xTail->setNext(xNew);
            }
            xNext = xNew;
          }
        }
        else if (x2 <= tailStart && x3 > tailStart)
        {
          if (x3 >= tailEnd)
          {
            delete xTail;
            xTail = NULL;
            if (xPrevious)
            {
              xPrevious->setNext(xNext);
              xTail = xPrevious;
            }
            else
            {
              yFreeMap[y] = xNext;
            }
          }
          else if (x3 < tailEnd)
          {
            xTail->setStart(x3);
            xTail->setFree(tailEnd - x3);
          }
        }
        xPrevious = xTail;
        xTail = xNext;
      }
      y++;
    } 
  }
}


void blendLevelsFree(BlendSection** yFreeMap, int64_t ySize)
{
  BlendSection *tail=NULL;
  BlendSection *tailOld=NULL;
  for (int64_t i=0; i < ySize; i++)
  {
    tail = yFreeMap[i];
    while (tail != NULL)
    {
      tailOld = tail;
      tail = tail->getNext();
      delete tailOld;
    }
  }
}


void blendLevels(BlendArgs *args)
{
  double xSrc = args->xSrc;
  double ySrc = args->ySrc;
  double grabColsB = args->grabColsB;
  double grabRowsB = args->grabRowsB;
  
  if (grabColsB < 0.0 || grabRowsB < 0.0) return;

  double xFactor = args->xFactor;
  double yFactor = args->yFactor;
 
  int64_t xStartA = (int64_t) floor(xSrc);
  int64_t yStartA = (int64_t) floor(ySrc);
  int64_t xEndA = (int64_t) ceil(xSrc + grabColsB);
  int64_t yEndA = (int64_t) ceil(ySrc + grabRowsB);
  if (yEndA > args->ySize) yEndA = args->ySize;
  
  int64_t xStartC = (int64_t) floor(xSrc / xFactor);
  int64_t yStartC = (int64_t) floor(ySrc / yFactor);
  BlendSection** yFreeMap = args->yFreeMap;
  for (int64_t y = yStartA; y < yEndA; y++)
  {
    BlendSection *xTail = yFreeMap[y];
    int64_t yStartD = (int64_t) floor((double) y / yFactor);
    int64_t yDest = (yStartD - yStartC) + args->yMargin;
    bool yIncrement = false;
    if (yDest > 0) 
    {
      yDest--;
      yIncrement = true;
    }
    while (xTail != NULL) 
    {
      int64_t xStartB = xTail->getStart();
      int64_t xFreeB = xTail->getFree();
      int64_t xEndB = xStartB + xFreeB;
      if (xStartB > xEndA) break;
      if (((xStartB <= xStartA && xEndB >= xStartA) || 
           (xStartB >= xStartA)))
      {
        if (xStartB < xStartA)
        {
          xStartB = xStartA;
        }
        int64_t xDest = (int64_t) floor(xStartB / xFactor);
        int64_t xDestEnd = (int64_t) ceil(xEndB / xFactor);
        int64_t xCopy = xDestEnd - xDest;
        if (xCopy < 0)
        {
          xTail = xTail->getNext();
          continue;
        }
        xCopy++;
        int64_t yCopy = 2;
        if (yIncrement)
        {
          yCopy++;
        }
        xDest = (xDest - xStartC) + args->xMargin;
        if (xDest > 0)
        {
          xDest--;
          xCopy++;
        }
        safeBmpCpy(args->pSafeDest, xDest, yDest, args->pSafeSrcL2, xDest, yDest, xCopy, yCopy);
      }
      xTail = xTail->getNext();
    }
  }
}
