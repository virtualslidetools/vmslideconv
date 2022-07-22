/*************************************************************************
Initial Author: Paul F. Richards (paulrichards321@gmail.com) 2016-2017
https://github.com/paulrichards321/jpg2svs

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
**************************************************************************/
#include <new>
#include <vector>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <sys/stat.h>
#include <sys/types.h>
#include "vmslideconv.h"
#include "composite.h"

const char* CompositeSlide::mMiniNames[4][4] = 
{ 
  {
    "FinalScan.ini", "FinalCond.ini", 
    "SlideScan.ini", "SlideCond.ini" 
  },
  {
    "finalscan.ini", "finalcond.ini", 
    "slidescan.ini", "slidecond.ini" 
  },
  {
    "finalScan.ini", "finalCond.ini", 
    "slideScan.ini", "slideCond.ini" 
  },
  {
    "Finalscan.ini", "Finalcond.ini", 
    "Slidescan.ini", "Slidecond.ini" 
  }
};


void CompositeSlide::compositeClearAttribs()
{
  mxStart=0;
  myStart=0;
  mxMax=0;
  mxMin=0;
  myMax=0;
  myMin=0;
}


void CompositeSlide::compositeCleanup()
{
  if (mEtc.size()>0)
  {
    for (size_t i = 0; i < mEtc.size(); i++)
    {
      if (mEtc[i])
      {
        delete mEtc[i];
        mEtc[i] = NULL;
      }
    }
    mEtc.clear();
  }
}


IniConf::IniConf()
{
  mName="";
  mFound=false;
  mxAdj=0.0;
  myAdj=0.0;
  mTotalCols=0;
  mTotalRows=0;
  mIsPreviewSlide=false;
}


JpgIniConf::JpgIniConf()
{
  mPixelCols=0;
  mPixelRows=0;
  mDetailedCols=0;
  mDetailedRows=0;
  mOrgDetailedCols=0;
  mOrgDetailedRows=0;
  mTotalTiles=0;
  mxMin=0;
  mxMax=0;
  myMin=0;
  myMax=0;
  mxDiffMin=0;
  myDiffMin=0;
  mxStepSize=0;
  myStepSize=0;
  mxAxis=0;
  myAxis=0;
  mxKnowStepSize=false;
  myKnowStepSize=false;
  mKnowStepSizes=false;
  for (int zSplit=0; zSplit < 2; zSplit++)
  {
    for (int zLevel=0; zLevel < 4; zLevel++)
    {
      mzStackExists[zSplit][zLevel] = false;
    }
  }
  mQuality=70;
} 


bool CompositeSlide::isPreviewSlide(int level)
{
  return (mValidObject && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound) ? mEtc[level]->mIsPreviewSlide : false; 
}


std::vector<JpgFileXY>* CompositeSlide::getTileXYArray(int level)
{ 
  return (mValidObject && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound) ? &mEtc[level]->mxyArr : NULL; 
}


bool CompositeSlide::checkLevel(int level)
{
  return (mValidObject && level >= 0 && level < (int) mEtc.size() && mEtc[level]->mFound && mEtc[level]->mKnowStepSizes) ? true : false;
}


bool CompositeSlide::checkZLevel(int level, int direction, int zLevel)
{
  return (mValidObject && level >= 0 && level < (int) mEtc.size() && direction >= 0 && direction < 3 && mEtc[level]->mFound && (direction==0 || mEtc[level]->mzStackExists[direction-1][zLevel])) ? true : false; 
}


int CompositeSlide::getTotalLevels()
{
  return (int) mEtc.size();
}


int CompositeSlide::getTotalZLevels() 
{ 
  return mValidObject == true ? mTotalZLevels : 0; 
}


int CompositeSlide::getTotalBottomZLevels() 
{ 
  return mValidObject == true ? mTotalBottomZLevels : 0; 
}


int CompositeSlide::getTotalTopZLevels() 
{ 
  return mValidObject == true ? mTotalTopZLevels : 0; 
}


int CompositeSlide::getQuality(int level) 
{ 
  return (mValidObject == true && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound) ? mEtc[level]->mQuality : 0; 
}


int64_t CompositeSlide::getPixelCols(int level) 
{ 
  return (mValidObject == true && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound ? mEtc[level]->mPixelCols : 0); 
}


int64_t CompositeSlide::getPixelRows(int level) 
{ 
  return (mValidObject == true && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound ? mEtc[level]->mPixelRows : 0); 
}


int64_t CompositeSlide::getActualCols(int level) 
{ 
  return (mValidObject == true && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound ? mEtc[level]->mTotalCols : 0); 
}


int64_t CompositeSlide::getActualRows(int level) 
{ 
  return (mValidObject == true && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound ? mEtc[level]->mTotalRows : 0); 
}


int64_t CompositeSlide::getLevelCols(int level) 
{ 
  int64_t fullCols = 0;
  if (mValidObject == true && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound)
  {
    fullCols = (mXYSwitched ? mEtc[level]->mTotalRows : mEtc[level]->mTotalCols); 
  }
  return fullCols;
}


int64_t CompositeSlide::getLevelRows(int level) 
{ 
  int64_t fullRows = 0;
  if (mValidObject == true && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound)
  {
    fullRows = (mXYSwitched ? mEtc[level]->mTotalCols : mEtc[level]->mTotalRows); 
  }
  return fullRows;
}


double CompositeSlide::getXAdj(int level) 
{ 
  return (mValidObject && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound) ? mEtc[level]->mxAdj : 1; 
}


double CompositeSlide::getYAdj(int level) 
{ 
  return (mValidObject && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound) ? mEtc[level]->myAdj : 1; 
}


int64_t CompositeSlide::getTotalTiles(int level) 
{ 
  return (mValidObject && level < (int) mEtc.size() && level >= 0 && mEtc[level]->mFound) ? mEtc[level]->mTotalTiles : 0; 
}


bool CompositeSlide::open(const std::string& srcFileName, int options, int optDebug)
{
  JpgFileXY jpgxy;
  JpgFileXY jpgxyzstack;                  
  bool nameFound=false;
  bool xFound=false, yFound=false;
  bool header=false;
  std::string iImageCols = "IIMAGECOLS";
  std::string iImageRows = "IIMAGEROWS";
  std::string lXStageRef = "LXSTAGEREF";
  std::string lYStageRef = "LYSTAGEREF";
  std::string lXStepSize = "LXSTEPSIZE";
  std::string lYStepSize = "LYSTEPSIZE";
  std::string lXOffset = "LXOFFSET";
  std::string lYOffset = "LYOFFSET";
  std::string headerStr = "HEADER";
  std::string dMagnification = "DMAGNIFICATION";
  std::string ImageQuality = "IMAGEQUALITY";
  std::string copyrightStr = "COPYRIGHT";
  std::string inputDir = srcFileName;
  mGrayScale = false;

  if (mValidObject)
  {
    compositeCleanup();
    baseCleanup();
    compositeClearAttribs();
    baseClearAttribs();
  }
  for (int i=0; i<4; i++)
  {
    JpgIniConf *mConfLocal = new JpgIniConf;
    mConfLocal->mName = mMiniNames[0][i];
    mEtc.push_back(mConfLocal);
  }

  mOptDebug = optDebug;
  mOptBorder=options & CONV_HIGHLIGHT; 

  for (int i=0; i<4; i++)
  {
    for (int cases=0; cases < 4; cases++)
    {
      size_t namePos=inputDir.find(mMiniNames[cases][i]);
      if (namePos != std::string::npos)
      {
        inputDir = srcFileName.substr(0, namePos-1);
        break;
      }
    }
  }
  if (inputDir.length()>0)
  {
    char lastKey=inputDir[inputDir.length()];
    if (lastKey=='\\' || lastKey=='/')
    {
      inputDir=inputDir.substr(0, inputDir.length()-1);
    }
  }
  
  std::ofstream logFile;
  if (optDebug > 1)
  {
    logFile.open("SlideScan.openimage.log", std::ios::out);
  }
  for (int fileNum = 0; fileNum < 4; fileNum++)
  {
    JpgIniConf* pConf =  mEtc[fileNum];
    std::string inputName;
    std::ifstream iniFile;
    bool foundFile = false;

    for (int cases=0; cases < 4 && foundFile==false; cases++)
    {
      inputName = inputDir;
      inputName += separator();
      inputName += mMiniNames[cases][fileNum];
    
      iniFile.open(inputName.c_str());
      if (iniFile.good())
      {
        std::cout << "Found: '" << inputName << "'" << std::endl;
        foundFile = true;
      }
    }
    if (foundFile==false)
    {
      inputName = inputDir;
      inputName += separator();
      inputName += mMiniNames[0][fileNum];
      std::cout << "Warning: Failed to open: '" << inputName << "'!" << std::endl;
    }
    xFound = false;
    yFound = false;
    nameFound = false;
    header = false;

    int c = 0;
    while (iniFile.good() && iniFile.eof()==false)
    {
      std::string line="";
      std::string rawLine="";
      std::string normalLine="";
      do
      {
        c = iniFile.get();
        if (c == 0x0A || c == EOF) break;
        if (c != 0x0D) rawLine += (char) c;
        if (c != 0x0D && c != ' ' && c != '\t')
        {
          normalLine += (char) c;
          line += (char) toupper(c);
        }
      } while (iniFile.eof()==false && iniFile.good());

      if (line.length()>=3)
      {
        size_t rpos = line.find(']');
        if (line[0]=='[' && rpos != std::string::npos)
        {
          if (xFound && yFound && nameFound)
          {
            pConf->mxyArr.push_back(jpgxy);
          }
          std::string upperChunkName=line.substr(1, rpos-1);
          std::string chunkName=normalLine.substr(1, rpos-1);
          if (upperChunkName.compare(headerStr)==0)
          {
            jpgxy.mBaseFileName.clear();
            nameFound = false;
            header = true;
          }
          else
          {
            jpgxy.mBaseFileName = inputDir;
            jpgxy.mBaseFileName += separator();
            jpgxy.mBaseFileName += chunkName;
            char location[2][4] = { "_u", "_d" };
            struct stat fileStat;
            for (int zSplit=0; zSplit < 2; zSplit++)
            {
              for (int zLevel=0; zLevel<4; zLevel++)
              {
                std::ostringstream nameStream;
                nameStream << jpgxy.mBaseFileName << location[zSplit] << (zLevel+1) << ".jpg";
                jpgxy.mFileName[zSplit][zLevel] = nameStream.str();
                if (stat(jpgxy.mFileName[zSplit][zLevel].c_str(), &fileStat)==0)
                {
                  pConf->mzStackExists[zSplit][zLevel] = true;
                  jpgxy.mzStack[zSplit][zLevel] = true;
                }
                else
                {
                  jpgxy.mzStack[zSplit][zLevel] = false;
                }
              }
            }
            jpgxy.mBaseFileName += ".jpg";
            nameFound = true;
            header = false;
          }
          //g_object_unref(foldedHeaderName);
          xFound = false;
          yFound = false;
        }
        if (header)
        {
          size_t colsPos=line.find(iImageCols);
          if (colsPos != std::string::npos && colsPos+iImageCols.length()+1 < line.length())
          {
            std::string cols = line.substr(colsPos+iImageCols.length()+1);
            pConf->mPixelCols=atoi(cols.c_str());
          }
          size_t rowsPos=line.find(iImageRows);
          if (rowsPos != std::string::npos && rowsPos+iImageRows.length()+1 < line.length())
          {
            std::string rows = line.substr(rowsPos+iImageRows.length()+1);
            pConf->mPixelRows=atoi(rows.c_str());
          }
          size_t xStagePos=line.find(lXStageRef);
          if (xStagePos != std::string::npos && xStagePos+lXStageRef.length()+1<line.length())
          {
            std::string xStageSubStr = line.substr(xStagePos+lXStageRef.length()+1);
            pConf->mxAxis=atoi(xStageSubStr.c_str());
          }
          size_t yStagePos=line.find(lYStageRef);
          if (yStagePos != std::string::npos && yStagePos+lYStageRef.length()+1<line.length())
          {
            std::string yStageSubStr = line.substr(yStagePos+lYStageRef.length()+1);
            pConf->myAxis=atoi(yStageSubStr.c_str());
          }
          size_t yStepPos = line.find(lYStepSize);
          if (yStepPos != std::string::npos && yStepPos+lYStepSize.length()+1<line.length())
          {
            std::string yStepSubStr = line.substr(yStepPos+lYStepSize.length()+1);
            pConf->myStepSize = atoi(yStepSubStr.c_str());
            if (optDebug > 0)
            {
              std::cout << "Exact y step measurements found for level " << fileNum << std::endl;
            }
          }
          size_t xStepPos = line.find(lXStepSize);
          if (xStepPos != std::string::npos && xStepPos+lXStepSize.length()+1<line.length())
          {
            std::string xStepSubStr = line.substr(xStepPos+lXStepSize.length()+1);
            pConf->mxStepSize = atoi(xStepSubStr.c_str());
            if (optDebug > 0)
            {
              std::cout << "Exact x step measurements found for level " << fileNum << std::endl;
            }
          }
          size_t xOffsetPos = line.find(lXOffset);
          if (xOffsetPos != std::string::npos && xOffsetPos+lXOffset.length()+1<line.length())
          {
            //std::string xOffsetSubStr = line.substr(xOffsetPos+lXOffset.length()+1);
            //xOffset = atoi(xOffsetSubStr.c_str());
          }
          size_t yOffsetPos = line.find(lYOffset);
          if (yOffsetPos != std::string::npos && yOffsetPos+lYOffset.length()+1<line.length())
          {
            //std::string yOffsetSubStr = line.substr(yOffsetPos+lYOffset.length()+1);
            //yOffset = atoi(yOffsetSubStr.c_str());
          }
          size_t dMagPos = line.find(dMagnification);
          if (dMagPos != std::string::npos && dMagPos+dMagnification.length()+1<line.length())
          {
            std::string magLine = line.substr(dMagPos+dMagnification.length()+1);
            parseMagStr(magLine);
          }
          size_t qualityPos = line.find(ImageQuality);
          if (qualityPos != std::string::npos && qualityPos+ImageQuality.length()+1<line.length())
          {
            std::string qualitySubStr = line.substr(qualityPos+ImageQuality.length()+1);
            pConf->mQuality = atoi(qualitySubStr.c_str());
            if (optDebug > 0)
            {
              std::cout << "Jpeg quality read from ini file: " << pConf->mQuality << std::endl;
            }
            if (optDebug > 1)
            {
              logFile << "Jpeg quality read from ini file: " << pConf->mQuality << std::endl;
            }
          }
          size_t copyrightPos = line.find(copyrightStr);
          if (copyrightPos != std::string::npos && copyrightPos+copyrightStr.length()+1<line.length())
          {
            size_t equalsPos = rawLine.find("=");
            if (equalsPos != std::string::npos && equalsPos+1<rawLine.length())
            {
              equalsPos++;
              while (equalsPos < rawLine.length() && 
                     (rawLine[equalsPos] == ' ' || rawLine[equalsPos] == '\t')) equalsPos++;
              if (equalsPos < line.length()) mCopyrightTxt = rawLine.substr(equalsPos);
            }
          }
        }
        std::string line2=line.substr(0, 2);
        if (line2=="X=")
        {
          std::string somenum=line.substr(2);
          jpgxy.mx=atoi(somenum.c_str());
          if (header) 
          {
            pConf->mxAxis = jpgxy.mx;
            jpgxy.mx=0;
          }
          else
          {
            xFound=true;
          }
        }
        if (line2=="Y=")
        {
          std::string somenum=line.substr(2);
          jpgxy.my=atoi(somenum.c_str());
          if (header) 
          {
            pConf->myAxis = jpgxy.my;
            jpgxy.my=0;
          }
          else
          {
            yFound=true;
          }
        }
      }
    }
    if (xFound && yFound && nameFound)
    {
      pConf->mxyArr.push_back(jpgxy);
    }
    iniFile.close();
  }
  
  
  myMin=0, myMax=0, mxMin=0, mxMax=0;
  bool yMinSet=false, xMaxSet=false, xMinSet=false, yMaxSet=false;
  for (int fileNum=0; fileNum < 4; fileNum++)
  {
    JpgIniConf* pConf =  mEtc[fileNum];
    if (pConf->mxyArr.size()==0) continue;

    pConf->mTotalTiles = pConf->mxyArr.size();
    if (pConf->mPixelCols<=0 || pConf->mPixelRows<=0)
    {
      Jpg jpg;
      jpg.setUnfilledColor(mBkgColor);
      if (jpg.open(pConf->mxyArr[0].mBaseFileName))
      {
        pConf->mPixelCols=jpg.getActualCols();
        pConf->mPixelRows=jpg.getActualRows();
        jpg.close();
      }
      else
      {
        std::string errMsg;
        jpg.getErrMsg(errMsg);
        std::cerr << "Error: failed to open " << pConf->mxyArr[0].mBaseFileName << " do not have pixel cols and rows for source jpgs." << std::endl;
        std::cerr << "Returned error: " << errMsg << std::endl;
        if (optDebug > 1) 
        {
          logFile << "Error: failed to open " << pConf->mxyArr[0].mBaseFileName << " do not have pixel cols and rows for source jpgs." << std::endl;
          logFile << "Returned error: " << errMsg << std::endl;
          logFile.close();
        }
        return false;
      }
    }
    if (optDebug > 1) logFile << "fileName=" << pConf->mName << " jpgCols=" << pConf->mPixelCols << " jpgRows=" << pConf->mPixelRows << std::endl;
    pConf->mFound = true;
    
    //************************************************************************
    // Get the xmin and xmax values
    //************************************************************************
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end(), JpgFileXYSortForX());
    pConf->mxMin = pConf->mxyArr[0].mx;
    pConf->mxMax = pConf->mxyArr[pConf->mTotalTiles-1].mx;
    for (int64_t i=0; i+1 < pConf->mTotalTiles; i++)
    {
      if (pConf->mxyArr[i+1].mx==pConf->mxyArr[i].mx)
      {
        int64_t diff=pConf->mxyArr[i+1].my - pConf->mxyArr[i].my;
        if ((diff>0 && diff<pConf->myDiffMin) || (diff>0 && pConf->myDiffMin<1))
        {
          pConf->myDiffMin=diff;
        }
      }
    }

    //************************************************************************
    // Get the ymin and ymax values
    //************************************************************************
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end(), JpgFileXYSortForY());
    pConf->myMin=pConf->mxyArr[0].my;
    pConf->myMax=pConf->mxyArr[pConf->mTotalTiles-1].my; // + pConf->yDiffMin;

    for (int64_t i=0; i+1 < pConf->mTotalTiles; i++)
    {
      if (pConf->mxyArr[i+1].my==pConf->mxyArr[i].my)
      {
        int64_t diff=pConf->mxyArr[i+1].mx - pConf->mxyArr[i].mx;
        if ((diff>0 && diff<pConf->mxDiffMin) || (diff>0 && pConf->mxDiffMin<1)) 
        {
          pConf->mxDiffMin=diff;
        }
      }
    }
    if (pConf->mxStepSize>0)
    {
      if (optDebug > 1) logFile << "fileName=" << pConf->mName << " xAdj calculation exact=";
      pConf->mxKnowStepSize = true;
    }
    else
    {
      if (fileNum>0 && mEtc[fileNum-1]->mFound && mEtc[fileNum-1]->mxStepSize>0)
      {
        pConf->mxStepSize = mEtc[fileNum-1]->mxStepSize*4;
        pConf->mxKnowStepSize = true;
      }
      else
      {
        if (pConf->mxDiffMin > 0)
        {
          pConf->mxStepSize = pConf->mxDiffMin;
          pConf->mxKnowStepSize = true;
        }
        else
        {
          pConf->mxStepSize = abs(pConf->mxMax);
          pConf->mxKnowStepSize = false;
        }
      }
      if (optDebug > 1) logFile << "fileName=" << pConf->mName << " Guessing xAdj=";
    }
   	//pConf->mxMin -= pConf->mxStepSize;
    if (pConf->mPixelCols>0 && pConf->mxStepSize>0)
    {
      pConf->mxAdj = (double) pConf->mxStepSize / (double) pConf->mPixelCols;
      if (optDebug > 1) logFile << pConf->mxAdj << std::endl;
    }
   
    if (pConf->myStepSize>0)
    {
      if (optDebug > 1) logFile << "fileName=" << pConf->mName << " yAdj calculation exact=";
      pConf->myKnowStepSize = true;
    }
    else
    {
      if (fileNum>0 && mEtc[fileNum-1]->mFound && mEtc[fileNum-1]->myStepSize>0)
      {
        pConf->myStepSize = (int64_t) (mEtc[fileNum-1]->myStepSize*4);
        pConf->myKnowStepSize = true;
      }
      else
      {
        if (pConf->myDiffMin > 0)
        {
          pConf->myStepSize = pConf->myDiffMin;
          pConf->myKnowStepSize = true;
        }
        else
        {
          pConf->myStepSize = abs(pConf->myMax);
          pConf->myKnowStepSize = false;
        }
      }
      if (optDebug > 1) logFile << "fileName=" << pConf->mName << " Guessing yAdj=";
    }
    pConf->mKnowStepSizes = (pConf->mxKnowStepSize && pConf->myKnowStepSize) ? true : false;
    if (pConf->mPixelRows>0 && pConf->myStepSize>0)
    {
      pConf->myAdj = (double) pConf->myStepSize / (double) pConf->mPixelRows;
      if (optDebug > 1) logFile << pConf->myAdj << std::endl;
    }
  
    if (optDebug > 1) 
    {
      logFile << "fileName=" << pConf->mName << " xDiffMin=" << pConf->mxDiffMin << " xStepSize=" << pConf->mxStepSize << " xMin=" << pConf->mxMin << " xMax=" << pConf->mxMax << " xAxis=" << pConf->mxAxis << std::endl;
      logFile << "fileName=" << pConf->mName << " yDiffMin=" << pConf->myDiffMin << " yStepSize=" << pConf->myStepSize << " yMin=" << pConf->myMin << " yMax=" << pConf->myMax << " yAxis=" << pConf->myAxis << std::endl;
    }
    pConf->mOrgDetailedCols = (int64_t) floor((pConf->mxMax - (pConf->mxMin - pConf->mxStepSize)) / pConf->mxAdj);
    pConf->mDetailedCols = pConf->mOrgDetailedCols;

    pConf->mOrgDetailedRows = (int64_t) floor((pConf->myMax - (pConf->myMin - pConf->myStepSize)) / pConf->myAdj);
    pConf->mDetailedRows = pConf->mOrgDetailedRows;

    if ((yMinSet==false || pConf->myMin < myMin) && fileNum < 3)
    {
      myMin=pConf->myMin;
      yMinSet = true;
    }
    if ((yMaxSet==false || pConf->myMax > myMax) && fileNum < 3)
    {
      myMax=pConf->myMax;
      yMaxSet = true;
    }
    if ((xMinSet==false || pConf->mxMin < mxMin) && fileNum < 3)
    { 
      mxMin=pConf->mxMin;
      xMinSet = true;
    }
    if ((xMaxSet==false || pConf->mxMax > mxMax) && fileNum < 3)
    {
      mxMax=pConf->mxMax;
      xMaxSet = true;
    }
    if (fileNum==0)
    {
      for (int zLevel=0; zLevel<4; zLevel++)
      {
        if (pConf->mzStackExists[0][zLevel])
        {
          mTotalBottomZLevels++;
          mTotalZLevels++;
        }
      }
      for (int zLevel=0; zLevel<4; zLevel++)
      {
        if (pConf->mzStackExists[1][zLevel])
        {
          mTotalTopZLevels++;
          mTotalZLevels++;
        }
      }
    }
  }

  bool iniSortByAdj=true;
  for (int i=0; i<4; i++)
  {
    if (mEtc[i]->mFound==false || mEtc[i]->mxAdj==0 || mEtc[i]->myAdj==0)
    {
      iniSortByAdj=false;
    }
  }
  if (iniSortByAdj)
  {
    std::sort(mEtc.begin(), mEtc.end(), JpgFileXYSortForXAdj());
  }
  for (int fileNum=0; fileNum < 4; fileNum++)
  {
    if (mEtc[fileNum]->mxAxis==0 || mEtc[fileNum]->myAxis==0)
    {
      int targetFileNum;
      if (fileNum==0 && mEtc[2]->mxAxis > 0)
      {
        targetFileNum = 2;
      }
      else if (fileNum==0 && mEtc[1]->mxAxis > 0)
      {
        targetFileNum = 1;
      }
      else if (fileNum==2 && mEtc[0]->mxAxis > 0)
      {
        targetFileNum = 0;
      }
      else if (fileNum==2 && mEtc[1]->mxAxis > 0)
      {
        targetFileNum = 1;
      }
      else if (fileNum==1 || fileNum==3)
      {
        targetFileNum = fileNum - 1;
      }
      else 
      {
        targetFileNum = -1;
      }
      if (targetFileNum >= 0)
      {
        mEtc[fileNum]->mxAxis = mEtc[targetFileNum]->mxAxis;
        mEtc[fileNum]->myAxis = mEtc[targetFileNum]->myAxis;
      }
      else
      {
        mEtc[fileNum]->mxAxis=278000;
        mEtc[fileNum]->myAxis=142500;
      }
    }
    if (mEtc[fileNum]->mQuality == 0)
    {
      if (fileNum==1)
      {
        mEtc[fileNum]->mQuality = 90;
      }
      else if (fileNum>1)
      {
        mEtc[fileNum]->mQuality = 95;
      }
      else
      {
        mEtc[fileNum]->mQuality = 85;
      }
    }
  } 
  
  //*******************************************************************
  // Find the pyramid level lowest zoom and set that as current image
  //*******************************************************************
  int level=-1;
  for (int min=3; min>=0; min--)
  {
    if (mEtc[min]->mFound==true && mEtc[min]->mKnowStepSizes==true)
    {
      level=min;
      break;
    }
  }
  if (level==-1)
  {
    if (optDebug > 1) logFile << "File has no readable levels." << std::endl;
    mValidObject = false;
    if (optDebug > 1) logFile.close();
    return false;
  }
  
  //****************************************************************
  // Guess the total image cols and rows for each pyramid level
  //****************************************************************
  double multiX[3] = { 1.0, 1.0, 1.0 };
  double multiY[3] = { 1.0, 1.0, 1.0 };
  
  if (mEtc[3]->mFound && mEtc[2]->mFound)
  {
    multiX[2] = mEtc[2]->mxAdj / mEtc[3]->mxAdj;
    multiY[2] = mEtc[2]->myAdj / mEtc[3]->myAdj;
  }
  if (mEtc[2]->mFound && mEtc[1]->mFound)
  {
    multiX[1] = mEtc[2]->mxAdj / mEtc[1]->mxAdj;
    multiY[1] = mEtc[2]->myAdj / mEtc[1]->myAdj;
  }
  else if (mEtc[3]->mFound && mEtc[1]->mFound)
  {
    multiX[1] = mEtc[3]->mxAdj / mEtc[1]->mxAdj;
    multiY[1] = mEtc[3]->myAdj / mEtc[1]->myAdj;
  }
  if (mEtc[0]->mFound)
  {
    if (mEtc[2]->mFound)
    {
      multiX[0] = mEtc[2]->mxAdj / mEtc[0]->mxAdj;
      multiY[0] = mEtc[2]->myAdj / mEtc[0]->myAdj;
    }
    else if (mEtc[3]->mFound)
    {
      multiX[0] = mEtc[3]->mxAdj / mEtc[0]->mxAdj;
      multiY[0] = mEtc[3]->myAdj / mEtc[0]->myAdj;
    }
    else if (mEtc[1]->mFound)
    {
      multiX[0] = mEtc[1]->mxAdj / mEtc[0]->mxAdj;
      multiY[0] = mEtc[1]->myAdj / mEtc[0]->myAdj;
    }
  }
  
  if (mEtc[2]->mFound && mEtc[2]->mKnowStepSizes)
  {
    mEtc[2]->mTotalCols = (int64_t)floor((double)(mEtc[2]->mxMax - (mEtc[2]->mxMin - mEtc[2]->mxStepSize)) / (double) mEtc[2]->mxAdj);
    mEtc[2]->mTotalRows = (int64_t)floor((double)(mEtc[2]->myMax - (mEtc[2]->myMin - mEtc[2]->myStepSize)) / (double) mEtc[2]->myAdj);

    mEtc[3]->mTotalCols = (int64_t) floor(mEtc[2]->mTotalCols * multiX[2]);
    mEtc[3]->mTotalRows = (int64_t) floor(mEtc[2]->mTotalRows * multiY[2]);

    mEtc[1]->mTotalCols = (int64_t) floor(mEtc[2]->mTotalCols * multiX[1]);
    mEtc[1]->mTotalRows = (int64_t) floor(mEtc[2]->mTotalRows * multiY[1]);

    mEtc[0]->mTotalCols = (int64_t) floor(mEtc[2]->mTotalCols * multiX[0]);
    mEtc[0]->mTotalRows = (int64_t) floor(mEtc[2]->mTotalRows * multiY[0]);
  }
  else if (mEtc[3]->mFound && mEtc[3]->mKnowStepSizes)
  {
    mEtc[3]->mTotalCols = (int64_t) floor((double)(mEtc[3]->mxMax - (mEtc[3]->mxMin - mEtc[3]->mxStepSize)) / (double) mEtc[3]->mxAdj);
    mEtc[3]->mTotalRows = (int64_t) floor((double)(mEtc[3]->myMax - (mEtc[3]->myMin - mEtc[3]->myStepSize)) / (double) mEtc[3]->myAdj);

    mEtc[1]->mTotalCols = (int64_t) floor(mEtc[3]->mTotalCols * multiX[1]);
    mEtc[1]->mTotalRows = (int64_t) floor(mEtc[3]->mTotalRows * multiY[1]);
    mEtc[0]->mTotalCols = (int64_t) floor(mEtc[3]->mTotalCols * multiX[0]);
    mEtc[0]->mTotalRows = (int64_t) floor(mEtc[3]->mTotalRows * multiY[0]);
  }
  else if (mEtc[1]->mFound && mEtc[1]->mKnowStepSizes)
  {
    mEtc[1]->mTotalCols = (int64_t) floor((double)(mEtc[1]->mxMax - (mEtc[1]->mxMin - mEtc[1]->mxStepSize)) / (double) mEtc[1]->mxAdj);
    mEtc[1]->mTotalRows = (int64_t) floor((double)(mEtc[1]->myMax - (mEtc[1]->myMin - mEtc[1]->myStepSize)) / (double) mEtc[1]->myAdj);

    mEtc[0]->mTotalCols = (int64_t) floor(mEtc[1]->mTotalCols * multiX[0]);
    mEtc[0]->mTotalRows = (int64_t) floor(mEtc[1]->mTotalRows * multiY[0]);
  }
  else
  {
    for (int fileNum=0; fileNum < 4; fileNum++)
    {
      mEtc[fileNum]->mTotalCols = (int64_t) floor((double)(mEtc[fileNum]->mxMax - (mEtc[fileNum]->mxMin - mEtc[fileNum]->mxStepSize)) / (double) mEtc[fileNum]->mxAdj);
      mEtc[fileNum]->mTotalRows = (int64_t) floor((double)(mEtc[fileNum]->myMax - (mEtc[fileNum]->myMin - mEtc[fileNum]->myStepSize)) / (double) mEtc[fileNum]->myAdj);
    }
  }

  // log file cols and rows
  for (int fileNum=0; fileNum < 4; fileNum++)
  {
    if (optDebug > 1) logFile << "fileName=" << mEtc[fileNum]->mName << " totalCols in pixels=" << mEtc[fileNum]->mTotalCols << " totalRows in pixels=" << mEtc[fileNum]->mTotalRows << std::endl;
  }

  JpgIniConf* pHigherConf = NULL;
  JpgIniConf* pLowerConf = NULL;
  bool higherLevelFound = false, lowerLevelFound = false;
  if (mEtc[2]->mFound && mEtc[2]->mKnowStepSizes)
  {
    pHigherConf = mEtc[2];
    higherLevelFound = true;
  }
  else if (mEtc[3]->mFound && mEtc[3]->mKnowStepSizes)
  {
    pHigherConf = mEtc[3];
    higherLevelFound = true;
  }
  if (mEtc[0]->mFound)
  {
    pLowerConf = mEtc[0];
    lowerLevelFound = true;
  }
  if (mEtc[1]->mFound)
  {
    pLowerConf = mEtc[1];
    lowerLevelFound = true;
  }
  //*****************************************************************
  // Calculate the x and y coordinate of higherLevels 
  //*****************************************************************
  for (int fileNum=2; fileNum<4; fileNum++)
  {
    JpgIniConf* pConf= mEtc[fileNum];
    if (pConf->mFound==false) continue;
     
    for (int64_t i=0; i<pConf->mTotalTiles; i++)
    {
      // missing lines problem occurs in rounding here
      double xPixel=((double)(pConf->mxMax - pConf->mxyArr[i].mx)/(double)pConf->mxAdj);
      int64_t xPixelInt=(int64_t) round(xPixel);
      //if (xPixelInt>0) xPixelInt--;
      pConf->mxyArr[i].mxPixel=xPixelInt; // previous use lround here
      
      double yPixel=((double)(pConf->myMax - pConf->mxyArr[i].my)/(double)pConf->myAdj);
      int64_t yPixelInt=(int64_t) round(yPixel);
      //if (yPixelInt>0) yPixelInt--;
      pConf->mxyArr[i].myPixel=yPixelInt; // previous use lround here
      
      if (optDebug > 1) logFile << "filename=" << pConf->mxyArr[i].mBaseFileName << " x=" << xPixelInt << " y=" << yPixelInt << std::endl;

      for (int zSplit=0; zSplit < 2; zSplit++)
      {
        for (int zLevel=0; zLevel<4; zLevel++)
        {
          if (pConf->mxyArr[i].mzStack[zSplit][zLevel] && optDebug > 1)
          {
            logFile << "filename=" << pConf->mxyArr[i].mFileName[zSplit][zLevel] << " x=" << xPixelInt << " y=" << yPixelInt << std::endl;
          }
        }
      }
    }
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end());
  }
  mValidObject = true;

  int64_t bestXOffsetL0=0, bestXOffsetL1=0;
  int64_t bestYOffsetL0=0, bestYOffsetL1=0;
  if (lowerLevelFound && higherLevelFound)
  {
    double higherRatioX = (double) pHigherConf->mPixelCols / (double) pHigherConf->mxStepSize; 
    double higherRatioY = (double) pHigherConf->mPixelRows / (double) pHigherConf->myStepSize;

    double higherMinBaseX = (double) (pLowerConf->mxAxis - pHigherConf->mxMax);
    double higherMinBaseY = (double) (pLowerConf->myAxis - pHigherConf->myMax);

    if (mEtc[0]->mFound)
    {
      double ratioAL0X = (double) mEtc[0]->mPixelCols / (double) mEtc[0]->mxStepSize;
      double ratioAL0Y = (double) mEtc[0]->mPixelRows / (double) mEtc[0]->myStepSize;
      double ratioBL0X = (double) pHigherConf->mxStepSize / (double) mEtc[0]->mxStepSize;
      double ratioBL0Y = (double) pHigherConf->myStepSize / (double) mEtc[0]->myStepSize;

      double stageBaseL0X = (double) mEtc[0]->mxAxis + ((double) pHigherConf->mxStepSize / 2);
      stageBaseL0X -= (double) mEtc[0]->mxStepSize / 2;

      double stageBaseL0Y = (double) mEtc[0]->myAxis + ((double) pHigherConf->myStepSize / 2);
      stageBaseL0Y -= (double) mEtc[0]->myStepSize / 2;

      double lowerMinBaseL0X = stageBaseL0X - mEtc[0]->mxMax;
      double lowerMinBaseL0Y = stageBaseL0Y - mEtc[0]->myMax;
   
      double minusL0X = higherMinBaseX * higherRatioX * ratioBL0X;
      double minusL0Y = higherMinBaseY * higherRatioY * ratioBL0Y;

      bestXOffsetL0 = (int64_t) ceil(lowerMinBaseL0X * ratioAL0X - minusL0X);
      bestYOffsetL0 = (int64_t) ceil(lowerMinBaseL0Y * ratioAL0Y - minusL0Y);
    }
    if (mEtc[1]->mFound)
    {
      double ratioAL1X = (double) mEtc[1]->mPixelCols / (double) mEtc[1]->mxStepSize;
      double ratioAL1Y = (double) mEtc[1]->mPixelRows / (double) mEtc[1]->myStepSize;
      double ratioBL1X = (double) pHigherConf->mxStepSize / (double) mEtc[1]->mxStepSize;
      double ratioBL1Y = (double) pHigherConf->myStepSize / (double) mEtc[1]->myStepSize;

      double stageBaseL1X = (double) mEtc[1]->mxAxis + ((double) pHigherConf->mxStepSize / 2);
      stageBaseL1X -= (double) mEtc[1]->mxStepSize / 8;

      double stageBaseL1Y = (double) mEtc[1]->myAxis + ((double) pHigherConf->myStepSize / 2);
      stageBaseL1Y -= (double) mEtc[1]->myStepSize / 8;

      double lowerMinBaseL1X = stageBaseL1X - mEtc[1]->mxMax;
      double lowerMinBaseL1Y = stageBaseL1Y - mEtc[1]->myMax;
   
      double minusL1X = higherMinBaseX * higherRatioX * ratioBL1X;
      double minusL1Y = higherMinBaseY * higherRatioY * ratioBL1Y;

      bestXOffsetL1 = (int64_t) ceil(lowerMinBaseL1X * ratioAL1X - minusL1X);
      bestYOffsetL1 = (int64_t) ceil(lowerMinBaseL1Y * ratioAL1Y - minusL1Y);
    }
  }
  if (optDebug > 0)
  {
    std::cout << "Best X Offset Level0=" << bestXOffsetL0 << std::endl;
    std::cout << "Best Y Offset Level0=" << bestYOffsetL0 << std::endl;
    std::cout << "Best X Offset Level1=" << bestXOffsetL1 << std::endl;
    std::cout << "Best Y Offset Level1=" << bestYOffsetL1 << std::endl;
  }
  //*****************************************************************
  // Calculate the x and y coordinate of each file starting pixels
  //*****************************************************************
  for (int fileNum=0; fileNum<2; fileNum++)
  {
    JpgIniConf* pConf= mEtc[fileNum];
    if (pConf->mFound==false) continue;
    pConf->mxSortedArr.resize(pConf->mxyArr.size());

    for (int64_t i=0; i<pConf->mTotalTiles; i++)
    {
      double xPixel;
      xPixel=((double)(pConf->mxMax - pConf->mxyArr[i].mx)/(double)pConf->mxAdj);
      double yPixel;
      yPixel=((double)(pConf->myMax - pConf->mxyArr[i].my)/(double)pConf->myAdj);
      if (higherLevelFound && fileNum==0)
      {
        xPixel += bestXOffsetL0;
        yPixel += bestYOffsetL0;
      }  
      else if (higherLevelFound && fileNum==1)
      {
        xPixel += bestXOffsetL1;
        yPixel += bestYOffsetL1;
      }
      pConf->mxyArr[i].mxPixel=(int64_t)round(xPixel);
      pConf->mxyArr[i].myPixel=(int64_t)round(yPixel);
      pConf->mxSortedArr[i].mxPixel=(int64_t)round(xPixel);
      pConf->mxSortedArr[i].myPixel=(int64_t)round(yPixel);
      
      if (optDebug > 1) logFile << "filename=" << pConf->mxyArr[i].mBaseFileName << " x=" << xPixel << " y=" << yPixel << std::endl;
    }
    std::sort(pConf->mxyArr.begin(), pConf->mxyArr.end());
    std::sort(pConf->mxSortedArr.begin(), pConf->mxSortedArr.end(), JpgXYSortForX());
    for (int64_t tileNum=0; tileNum< (int64_t) pConf->mxyArr.size(); tileNum++)
    {
      for (int64_t tileNum2=0; tileNum2< (int64_t) pConf->mxyArr.size(); tileNum2++)
      {
        if (pConf->mxSortedArr[tileNum].mxPixel==pConf->mxyArr[tileNum2].mxPixel && pConf->mxyArr[tileNum2].myPixel==pConf->mxSortedArr[tileNum].myPixel)
        {
          pConf->mxyArr[tileNum2].mxSortedIndex = tileNum;
          break;
        }
      }
    }
  }
  mBaseCols = mEtc[0]->mTotalCols;
  mBaseRows = mEtc[0]->mTotalRows;
  std::string previewFileName = inputDir;
  previewFileName += separator();
  previewFileName += "PreviewSlide.jpg";
  Jpg previewJpg;
  if (previewJpg.open(previewFileName))
  {
    JpgIniConf *previewConf = new JpgIniConf;
    previewConf->mPixelCols = previewConf->mTotalCols = previewJpg.getActualCols();
    previewConf->mPixelRows = previewConf->mTotalRows = previewJpg.getActualRows();
    previewConf->mTotalTiles = 1;
    if (optDebug > 1) logFile << " PreviewSlide.jpg found. Cols=" << previewConf->mPixelCols << " Rows=" << previewConf->mPixelRows << std::endl;
    jpgxy.mBaseFileName = previewFileName;
    jpgxy.mxPixel=0;
    jpgxy.myPixel=0;
    jpgxy.mx=0;
    jpgxy.my=0;
    previewConf->mFound = true;
    previewConf->mxyArr.push_back(jpgxy);
    previewConf->mIsPreviewSlide = true;
    mEtc.push_back(previewConf);
    previewJpg.close();
  }
  else
  {
    std::cerr << "Warning: PreviewSlide.jpg not found." << std::endl;
  }
  if (optDebug > 1) logFile.close();
  return true;
}


bool CompositeSlide::parseMagStr(std::string magLine)
{
  mMagFound = false;
  mMag = 0;
  mMagStr = "";
  
  size_t endStr = magLine.length();
  size_t endNum = std::string::npos;
  size_t startNum = std::string::npos;
  for (size_t pos = 0; pos < endStr; pos++)
  {
    char k = magLine[pos];
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
    if (endNum == std::string::npos) endNum = endStr;
    mMagStr = magLine.substr(startNum, endNum-startNum);
    mMag = atof(mMagStr.c_str());
    mMagFound = true;
  }
  return mMagFound;
}


safeBmp* CompositeSlide::loadFullImage(int level, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::ofstream& logFile)
{
  JpgIniConf *pConf =  mEtc[level];
  
  if (pConf->mFound == false)
  {
    return NULL;
  }

  safeBmpZoomRes* pZoomRes = safeBmpZoomResInit();
  int64_t orgDetailedCols = pConf->mOrgDetailedCols;
  int64_t orgDetailedRows = pConf->mOrgDetailedRows;
  int64_t simulatedCols = orgDetailedCols; 
  int64_t simulatedRows = orgDetailedRows;
  if (useZoom)
  {
    simulatedCols = (int64_t) lround((double) orgDetailedCols / xZoomOut);
    simulatedRows = (int64_t) lround((double) orgDetailedRows / yZoomOut);
  }
  if (orientation == 90 || orientation == -90 || orientation == 270)
  {
    int64_t simulatedRowsOld = simulatedRows;
    simulatedRows = simulatedCols;
    simulatedCols = simulatedRowsOld;
  }
 
  safeBmp *pImgComplete = safeBmpAlloc(simulatedCols, simulatedRows);
  safeBmpByteSet(pImgComplete, 255);

  for (int64_t i=0; i < pConf->mTotalTiles; i++)
  {
    safeBmp imgPart1, imgPart2, imgPart3;
    std::string errStr;
    
    safeBmpClear(&imgPart1);
    safeBmpClear(&imgPart2);
    safeBmpClear(&imgPart3);

    if (safeJpgRead(&imgPart1, pConf->mxyArr[i].mBaseFileName, 255, &errStr) != 0)
    {
      logFile << "Error opening '" << pConf->mxyArr[i].mBaseFileName << "': " << errStr << std::endl;
      std::cerr << "Error opening '" << pConf->mxyArr[i].mBaseFileName << "': " << errStr << std::endl;
      continue;
    }
    safeBmp *pImgPart = &imgPart1;
    int64_t orgCols = imgPart1.cols;
    int64_t orgRows = imgPart1.rows;

    if (orientation == 90 || orientation == -90 || orientation == 270)
    {
      safeBmpAlloc2(&imgPart2, imgPart1.rows, imgPart1.cols);
      safeBmpRotate(&imgPart2, pImgPart, orientation);
      pImgPart = &imgPart2;
    }
    else if (orientation == 180)
    {
      safeBmpAlloc2(&imgPart2, imgPart1.cols, imgPart1.rows);
      safeBmpRotate(&imgPart2, pImgPart, orientation);
      pImgPart = &imgPart2;
    }
    if (useZoom)
    {
      int scaleMethod = SAFEBMP_BEST_ENLARGE;
      if (xZoomOut < 1.0 || yZoomOut < 1.0)
      {
        scaleMethod = SAFEBMP_BEST_SHRINK;
      }
      safeBmpZoom2(&imgPart3, pImgPart, lround(pImgPart->cols / xZoomOut), lround(pImgPart->rows / yZoomOut), xZoomOut, yZoomOut, scaleMethod, pZoomRes);
      pImgPart = &imgPart3;
    }
    double xPixelDbl=0;
    double yPixelDbl=0;
    xPixelDbl=((double)(pConf->mxMax - pConf->mxyArr[i].mx)/(double)pConf->mxAdj);
    yPixelDbl=((double)(pConf->myMax - pConf->mxyArr[i].my)/(double)pConf->myAdj);
    int64_t xPixel = (int64_t) round(xPixelDbl);
    int64_t yPixel = (int64_t) round(yPixelDbl);
    int64_t cols = pImgPart->cols;    
    int64_t rows = pImgPart->rows;
    int64_t xPixelNew = xPixel;
    int64_t yPixelNew = yPixel;
    if (orientation == 0)
    {
      // place holder, do nothing
    }  
    else if (orientation == 90)
    {
      xPixelNew = (orgDetailedRows - yPixel) - orgRows;
      yPixelNew = xPixel;
    }
    else if (orientation == -90 || orientation == 270)
    {
      xPixelNew = yPixel;
      yPixelNew = (orgDetailedCols - xPixel) - orgCols;
    }
    else if (orientation == 180)
    {
      xPixelNew = (orgDetailedCols - xPixel) - orgCols;
      yPixelNew = (orgDetailedRows - yPixel) - orgRows;
    }
    if (useZoom)
    {
      xPixelNew = (int64_t) lround(xPixelNew / xZoomOut);
      yPixelNew = (int64_t) lround(yPixelNew / yZoomOut);
    }
    if (xPixelNew < 0)
    {
      cols += xPixelNew;
      xPixelNew = 0;
    }
    if (yPixelNew < 0)
    {
      rows += yPixelNew;
      yPixelNew = 0;
    }
    if (xPixelNew + cols > pImgComplete->cols)
    {
      cols -= (xPixelNew + cols) - pImgComplete->cols;
    }
    if (yPixelNew + rows > pImgComplete->rows)
    {
      rows -= (yPixelNew + rows) - pImgComplete->rows;
    }
    if (cols > 0 && rows > 0 && xPixelNew < pImgComplete->cols && yPixelNew < pImgComplete->rows)
    {
      safeBmpCpy(pImgComplete, xPixelNew, yPixelNew, pImgPart, 0, 0, cols, rows);
    }
    else
    {
      std::cerr << "Warning: ROI outside of image boundaries: xPixelNew=" << xPixelNew << " cols=" << cols << " total cols of image=" << pImgComplete->cols;
      std::cerr << " yPixelNew=" << yPixelNew << " rows=" << rows << " total rows of image=" << pImgComplete->rows << std::endl;

      logFile << "Warning: ROI outside of image boundaries: xPixelNew=" << xPixelNew << " cols=" << cols << " total cols of image=" << pImgComplete->cols;
      logFile << " yPixelNew=" << yPixelNew << " rows=" << rows << " total rows of image=" << pImgComplete->rows << std::endl;
    }
    safeBmpFree(&imgPart1);
    safeBmpFree(&imgPart2);
    safeBmpFree(&imgPart3);
  }
  if (optDebug > 1 && pImgComplete && pImgComplete->data)
  {
    std::stringstream ss;
    ss << "imgComplete" << level;
    if (orientation != 0)
    {
      ss << "_" << orientation;    
    }
    ss << ".jpg";

    std::string levelFName = ss.str();
    std::string errMsg;
    safeJpgWrite(pImgComplete, levelFName, 90, &errMsg);
  }
  safeBmpZoomResFree(pZoomRes);
  return pImgComplete;
}


bool CompositeSlide::testHeader(BYTE* fileHeader, int64_t length)
{
  std::string headerStr = (const char*) fileHeader;
  std::string header = "header";
  if (length >= 8 && headerStr.length() >= 8)
  {
    std::string chunk = headerStr.substr(1, 6);
//    std::cout << chunk << std::endl;
//    gchar* foldedChunkName = g_utf8_casefold((gchar*)chunk.c_str(), chunk.size());
//    gchar* foldedHeaderName = g_utf8_casefold((gchar*)header.c_str(), header.length());
    if (strcasecmp(chunk.c_str(), header.c_str())==0)
    {
      return true;
    }
  }
  return false;
}


bool JpgXYSortForX::operator() (const JpgXY& jpgXY1, const JpgXY& jpgXY2) 
{
  if (jpgXY1.mxPixel==jpgXY2.mxPixel)
  {
    return jpgXY1.myPixel<jpgXY2.myPixel;
  }
  else
  {
    return jpgXY1.mxPixel<jpgXY2.mxPixel;
  }
}

bool JpgFileXY::operator < (const JpgFileXY& jpgFile) const
{
  if (jpgFile.myPixel==myPixel)
  {
    return mxPixel<jpgFile.mxPixel;
  }
  else
  {
    return myPixel<jpgFile.myPixel;
  }
}

bool JpgFileXYSortForX::operator() (const JpgFileXY& jpgFile1, const JpgFileXY& jpgFile2) 
{
  if (jpgFile1.mx==jpgFile2.mx)
  {
    return jpgFile1.my<jpgFile2.my;
  }
  else
  {
    return jpgFile1.mx<jpgFile2.mx;
  }
}

bool JpgFileXYSortForY::operator() (const JpgFileXY& jpgFile1, const JpgFileXY& jpgFile2) 
{
  if (jpgFile2.my==jpgFile1.my)
  {
    return jpgFile1.mx<jpgFile2.mx;
  }
  else
  {
    return jpgFile1.my<jpgFile2.my;
  }
}

bool JpgFileXYSortForXAdj::operator() (const IniConf *iniConf1, const IniConf *iniConf2)
{
  return (iniConf1->mxAdj < iniConf2->mxAdj);
}
