#include <iostream>
#include <cstring>
#include <vector>
#include "composite.h"

bool CompositeSlide::drawBorder(BYTE *pBmp, int samplesPerPixel, int64_t x, int64_t y, int64_t cols, int64_t rows, int level)
{
  if (checkLevel(level)==false) return false;
  JpgIniConf *pHigherConf=mEtc[level];
  const int default_thickness=8;
  //std::ofstream logFile("drawBorder.log", std::ios::out | std::ofstream::app);
  
  //logFile << " drawBorder x: " << x << " y: " << y << " cols: " << cols << " rows: " << rows << " level: " << level << " samplesPerPixel: " << samplesPerPixel << std::endl;

  int srcLevel=-1;
  for (int curLevel=level-1; curLevel>=0 && srcLevel==-1; curLevel--)
  {
    if (mEtc[curLevel]->mFound)
    {
      srcLevel=curLevel;
    }
  }
  if (srcLevel==-1)
  {
    return false;
  }
  JpgIniConf *pLowerConf=mEtc[srcLevel];

  double xZoomOut = pHigherConf->mxAdj / pLowerConf->mxAdj;
  double yZoomOut = pHigherConf->myAdj / pLowerConf->myAdj;
  int64_t fileCols=(int64_t) lround(pLowerConf->mPixelCols / xZoomOut);
  int64_t fileRows=(int64_t) lround(pLowerConf->mPixelRows / yZoomOut);
  //int64_t xTolerance=(int64_t)(((double)fileCols) / 4);
  //int64_t yTolerance=(int64_t)(((double)fileRows) / 4);
  //int64_t xLastPos=-1,yLastPos=-1;
  //int64_t xNextPos=-1,yNextPos=-1;
  for (int64_t tileNum=0; tileNum<pLowerConf->mTotalTiles; tileNum++)
  {
    int64_t xCurrentPos=(int64_t) lround(pLowerConf->mxyArr[tileNum].mxPixel / xZoomOut);
    int64_t yCurrentPos=(int64_t) lround(pLowerConf->mxyArr[tileNum].myPixel / yZoomOut);
    // first check if the x y coordinates are within the region of the bitmap
    
    if (((x<xCurrentPos-default_thickness && x+cols>xCurrentPos-default_thickness) || (x>=xCurrentPos-default_thickness && x<xCurrentPos+fileCols+default_thickness)) &&
       ((y<yCurrentPos-default_thickness && y+rows>yCurrentPos-default_thickness) || (y>=yCurrentPos-default_thickness && y<yCurrentPos+fileRows+default_thickness)))
    {
      int64_t y2=0, y3=fileRows;
      int64_t y4=0, y5=fileRows;
      int64_t x2=0, x3=fileCols;
      int64_t x4=0, x5=fileCols;

      for (int64_t tileNum2=0; tileNum2<pLowerConf->mTotalTiles; tileNum2++)
      {
        int64_t xCurrentPos2=(int64_t) lround(pLowerConf->mxyArr[tileNum2].mxPixel / xZoomOut);
        int64_t yCurrentPos2=(int64_t) lround(pLowerConf->mxyArr[tileNum2].myPixel / yZoomOut);
        if (xCurrentPos2==xCurrentPos && yCurrentPos2==yCurrentPos) continue;
        
        //********************************************************************
        // first check for left border
        //********************************************************************
        if (((xCurrentPos>xCurrentPos2 && xCurrentPos2+fileCols>=xCurrentPos) &&
            ((yCurrentPos<yCurrentPos2 && yCurrentPos+fileRows>yCurrentPos2) 
         ||  (yCurrentPos>=yCurrentPos2 && yCurrentPos<yCurrentPos2+fileRows))))
        {
          if (yCurrentPos2 < yCurrentPos)
          {
            int64_t y2b=(yCurrentPos2 + fileRows) - yCurrentPos;
            if (y2b > y2)
            {
              y2=y2b;
            }
          }
          else
          {
            int64_t y3b=yCurrentPos2 - yCurrentPos;
            if (y3b < y3)
            {
              y3=y3b;
            }
          }
        }
        //********************************************************************
        // check for right border
        //********************************************************************
        if (((xCurrentPos<xCurrentPos2 && xCurrentPos+fileCols>=xCurrentPos2) &&
            ((yCurrentPos<yCurrentPos2 && yCurrentPos+fileRows>yCurrentPos2) || (yCurrentPos>=yCurrentPos2 && yCurrentPos<yCurrentPos2+fileRows))))
        {
          if (yCurrentPos2 < yCurrentPos)
          {
            int64_t y4b=(yCurrentPos2 + fileRows) - yCurrentPos;
            if (y4b > y4)
            {
              y4=y4b;
            }
          }
          else
          {
            int64_t y5b=yCurrentPos2 - yCurrentPos;
            if (y5b < y5)
            {
              y5=y5b;
            }
          }
        }
        //**********************************************************************
        // check for top border
        //**********************************************************************
/*
        if (((xCurrentPos<=xCurrentPos2 && xCurrentPos+fileCols+3>=xCurrentPos2) ||
             (xCurrentPos>=xCurrentPos2 && xCurrentPos2+fileCols+3>=xCurrentPos)) &&
            (yCurrentPos>yCurrentPos2 && yCurrentPos<=yCurrentPos2+fileRows+3))*/
        if (((xCurrentPos<=xCurrentPos2 && xCurrentPos+fileCols>=xCurrentPos2) ||
             (xCurrentPos>=xCurrentPos2 && xCurrentPos2+fileCols>=xCurrentPos)) &&
            (yCurrentPos>yCurrentPos2 && yCurrentPos<=yCurrentPos2+fileRows))
        {
          if (xCurrentPos2 < xCurrentPos)
          {
            int64_t x2b=(xCurrentPos2 + fileCols) - xCurrentPos;
            if (x2b > x2)
            {
              x2=x2b;
            }
          }
          else
          {
            int64_t x3b=xCurrentPos2 - xCurrentPos;
            if (x3b < x3)
            {
              x3=x3b;
            }
          }
        }
        //**********************************************************************
        // check for bottom border
        //**********************************************************************
/*
        if (((xCurrentPos<=xCurrentPos2 && xCurrentPos+fileCols+3>=xCurrentPos2) ||
             (xCurrentPos>=xCurrentPos2 && xCurrentPos2+fileCols+3>=xCurrentPos)) &&
             (yCurrentPos2>yCurrentPos && yCurrentPos+fileRows+3>=yCurrentPos2))*/

        if (((xCurrentPos<=xCurrentPos2 && xCurrentPos+fileCols>=xCurrentPos2) ||
             (xCurrentPos>=xCurrentPos2 && xCurrentPos2+fileCols>=xCurrentPos)) &&
             (yCurrentPos2>yCurrentPos && yCurrentPos+fileRows>=yCurrentPos2))
        {
          if (xCurrentPos2 < xCurrentPos)
          {
            int64_t x4b=(xCurrentPos2 + fileCols) - xCurrentPos;
            if (x4b > x4)
            {
              x4=x4b;
            }
          }
          else
          {
            int64_t x5b=xCurrentPos2 - xCurrentPos;
            if (x5b < x5)
            {
              x5=x5b;
            }
          }
        }
      }
      if (y2 < y3)
      {
        int64_t yWrite1=(yCurrentPos+y2)-y;
        int64_t yWrite2=(yCurrentPos+y3)-y;
        //int64_t yWrite2=yWrite1+y3;
        if (yWrite1 < 0) 
        {
          //yWrite2=y3+yWrite1;
          yWrite1=0;
        }
        if (yWrite2 < 0)
        {
          yWrite2=0;
        }
        if (yWrite2 > rows)
        {
          yWrite2=rows;
        }
        int64_t xLineMark=xCurrentPos-x-default_thickness;
        int64_t thickness;
        if (xLineMark < 0)
        {
          thickness=xLineMark+default_thickness;
          xLineMark=0;
        }  
        else
        {
          thickness=default_thickness;
        }  
        if (xLineMark+thickness > cols)
        {
          thickness=cols - xLineMark;
        }
        drawYHighlight(pBmp, samplesPerPixel, xLineMark, yWrite1, yWrite2, cols, rows, (int) thickness, 1);
      }
      if (y4 < y5)
      {
        int64_t yWrite1=(yCurrentPos+y4)-y;
        int64_t yWrite2=(yCurrentPos+y5)-y;
        if (yWrite1 < 0) 
        {
          yWrite1=0;
        }
        if (yWrite2 < 0)
        {
          yWrite2=0;
        }
        if (yWrite2 > rows)
        {
          yWrite2=rows;
        }
        int64_t xLineMark=(xCurrentPos+fileCols)-x;
        int64_t thickness;
        if (xLineMark<0)
        {
          thickness=xLineMark+default_thickness;
          xLineMark=0;
        }
        else
        {
          thickness=default_thickness;
        }  
        if (xLineMark+thickness > cols)
        {
          thickness=cols - xLineMark;
        }
        drawYHighlight(pBmp, samplesPerPixel, xLineMark, yWrite1, yWrite2, cols, rows, (int) thickness, 1);
      }
      if (x2 < x3)
      {
        int64_t xWrite1=(xCurrentPos+x2)-x;
        int64_t xWrite2=(xCurrentPos+x3)-x;
        //int64_t xWrite2=xWrite1+x3;
        if (xWrite1 < 0) 
        {
          //xWrite2=x3+xWrite1;
          xWrite1=0;
        }
        if (xWrite2 < 0)
        {
          xWrite2=0;
        }
        if (xWrite2>cols)
        {
          xWrite2=cols;
        }
        int64_t thickness;
        int64_t yLineMark=yCurrentPos-y-default_thickness;
        if (yLineMark < 0)
        {
          thickness=yLineMark + default_thickness;
          yLineMark = 0;
        }
        else
        {
          thickness=default_thickness;
        }  
        if (yLineMark+thickness > rows)
        {
          thickness=rows - yLineMark;
        }
        //logFile << " XHighlight yLineMark: " << yLineMark << " xWrite1: " << xWrite1 << " xWrite2: " << xWrite2 << " cols: " << cols << " rows: " << rows << " thickness: " << thickness << std::endl;
        drawXHighlight(pBmp, samplesPerPixel, yLineMark, xWrite1, xWrite2, cols, rows, (int) thickness, 1);
      }
      if (x4 < x5)
      {
        int64_t xWrite1=(xCurrentPos+x4)-x;
        int64_t xWrite2=(xCurrentPos+x5)-x;
        if (xWrite1 < 0) 
        {
          xWrite1=0;
        }
        if (xWrite2 < 0)
        {
          xWrite2=0;
        }
        if (xWrite2>cols)
        {
          xWrite2=cols;
        }
        int64_t yLineMark=(yCurrentPos+fileRows)-y;
        int64_t thickness;
        if (yLineMark < 0)
        {
          thickness=yLineMark + default_thickness;
          yLineMark = 0;
        }
        else
        {
          thickness=default_thickness;
        }
        if (yLineMark+thickness > rows)
        {
          thickness=rows - yLineMark;
        }  
        //logFile << " XHighlight yLineMark: " << yLineMark << " xWrite1: " << xWrite1 << " xWrite2: " << xWrite2 << " cols: " << cols << " rows: " << rows << " thickness: " << thickness << std::endl;
        drawXHighlight(pBmp, samplesPerPixel, yLineMark, xWrite1, xWrite2, cols, rows, (int) thickness, 1);
      }
    }
  }
  return true;
}


bool drawXHighlight(BYTE *pBmp, int samplesPerPixel, int64_t y1, int64_t x1, int64_t x2, int64_t cols, int64_t rows, int thickness, int position)
{
  if (x1 < 0 || x2 < 0 || y1 < 0)
  {
    //std::cerr << "Warning: drawYHighlight: parameters out of bound!" << std::endl;
    //std::cerr << " y1=" << y1 << " x1=" << x1 << " x2=" << x2 << " cols=" << cols << " rows=" << rows << std::endl;
    return false;
  }
  if (x1 > x2)
  {
    int64_t x3=x1;
    x1=x2;
    x2=x3;
  }
  if (x2 > cols)
  {
    //std::cerr << "Warning: drawXHighlight: parameters out of bound!" << std::endl;
    //std::cerr << " y1=" << y1 << " x1=" << x1 << " x2=" << x2 << " cols=" << cols << " rows=" << rows << std::endl;
    return false;
  }
  int64_t bmpSize = cols * rows * samplesPerPixel;
  if (samplesPerPixel==3 || samplesPerPixel==4)
  {
    while (thickness > 0)
    {
      thickness--;
      int64_t offset = (cols * samplesPerPixel * (y1+(thickness*position))) + (x1 * samplesPerPixel);
      for (int64_t x3=x1; x3 < x2 && offset+2 < bmpSize; x3++, offset+=samplesPerPixel)
      {
        pBmp[offset] = 0;
        pBmp[offset+1] = 0;
        pBmp[offset+2] = 0;
      }
    }
  }
  else if (samplesPerPixel==1)
  {
    while (thickness > 0)
    {
      thickness--;
      int64_t offset = (cols * (y1+(thickness*position))) + x1;
      for (int64_t x3=x1; x3 < x2 && offset < bmpSize; x3++, offset++)
      {
        pBmp[offset] = 0;
      }
    }
  } 
  return true;
}


bool drawYHighlight(BYTE *pBmp, int samplesPerPixel, int64_t x1, int64_t y1, int64_t y2, int64_t cols, int64_t rows, int thickness, int position)
{
  if (y1 < 0 || y2 < 0 || x1 < 0)
  {
    std::cerr << "Warning: drawYHighlight: parameters out of bound!" << std::endl;
    std::cerr << " x1=" << x1 << " y1=" << y1 << " y2=" << y2 << " cols=" << cols << " rows=" << rows << std::endl;
    return false;
  }
  if (y1 > y2)
  {
    int64_t y3=y1;
    y1=y2;
    y2=y3;
  }
  if (y2 > rows)
  {
    std::cerr << "Warning: drawYHighlight: parameters out of bound!" << std::endl;
    std::cerr << " x1=" << x1 << " y1=" << y1 << " y2=" << y2 << " cols=" << cols << " rows=" << rows << std::endl;
    return false;
  }
  int64_t bmpSize=cols * rows * samplesPerPixel;
  if (samplesPerPixel==3 || samplesPerPixel==4)
  {
    while (thickness>0)
    {
      thickness--;
      for (int64_t y3=y1; y3 < y2; y3++)
      {
        int64_t offset = (cols * samplesPerPixel * y3) + ((x1+(thickness*position)) * samplesPerPixel);
        if (offset+2 < bmpSize)
        {
          pBmp[offset] = 0;
          pBmp[offset+1] = 0;
          pBmp[offset+2] = 0;
        }
      }
    }
  }
  else if (samplesPerPixel==1)
  {
    while (thickness>0)
    {
      thickness--;
      for (int64_t y3=y1; y3 < y2; y3++)
      {
        int64_t offset = (cols * y3) + x1+(thickness*position);
        if (offset < bmpSize)
        {
          pBmp[offset] = 0;
        }
      }
    }
  } 
  return true;
}

