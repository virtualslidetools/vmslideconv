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
  return false; 
}


bool AperioSlide::isPreviewSlide(int level) 
{ 
  return false; 
}


bool AperioSlide::setOrientation(int orientation, std::fstream& logFile) 
{ 
  return true; 
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
  return 70; 
}


int64_t AperioSlide::getPixelWidth(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->tileWidth : 0;
}


int64_t AperioSlide::getPixelHeight(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->tileLength : 0; 
}


int64_t AperioSlide::getActualWidth(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->width : 0; 
}


int64_t AperioSlide::getActualHeight(int level) 
{ 
  return (mValidObject && level < mTif.directorySize() && level >= 0) ? mTif.at(level)->length : 0; 
}


int64_t AperioSlide::getLevelWidth(int level) 
{ 
  int64_t fullWidth = 0;
  if (mValidObject && level < mTif.directorySize() && level >= 0)
  {
    fullWidth = (mXYSwitched ? mTif.at(level)->length : mTif.at(level)->width); 
  }
  return fullWidth;
}


int64_t AperioSlide::getLevelHeight(int level) 
{ 
  int64_t fullHeight = 0;
  if (mValidObject && level < mTif.directorySize() && level >= 0)
  {
    fullHeight = (mXYSwitched ? mTif.at(level)->width : mTif.at(level)->length); 
  }
  return fullHeight;
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


bool AperioSlide::findXYOffset(int lowerLevel, int higherLevel, int64_t* bestXOffset0, int64_t* bestYOffset0, int64_t* bestXOffset1, int64_t* bestYOffset1, int optUseCustomOffset, int debugLevel, std::fstream& logFile)
{
  *bestXOffset0 = 0;
  *bestYOffset0 = 0;
  *bestXOffset1 = 0;
  *bestYOffset1 = 0;
  return true;
}


bool AperioSlide::open(const std::string& srcFileName, int options, int orientation, int optDebug, int64_t bestXOffset, int64_t bestYOffset, safeBmp **ptpImageL2)
{
  if (mValidObject)
  {
    mTif.close();
    baseClearAttribs();
  }
  bool success = mTif.open(srcFileName);
  if (success)
  {
    mTif.setDebugMode((optDebug ? true : false));
    mBaseWidth = mTif.getActualWidth();
    mBaseHeight = mTif.getActualHeight();
    parseMagFromDesc(mTif.at(0)->description); 
    mValidObject = true;
  }
  return success;
}  


bool AperioSlide::read(safeBmp *pDestBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t width, int64_t height, int64_t* pReadWidth, int64_t* pReadHeight)
{
  // mTif.read initializes pReadWidth and pReadHeight right away with zero
  return mTif.read(pDestBmp, level, x, y, width, height, pReadWidth, pReadHeight);
}


#ifndef USE_MAGICK
bool AperioSlide::loadFullImage(int level, safeBmp** ptpFullImage, cv::Mat** ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile)
{
  if (checkLevel(level)==false) return false;
  
  TiffDirAttribs* pConf = mTif.at(level);

  if (ptpFullImage == NULL && ptpMatImage == NULL)
  {
    return false;
  }

  if (ptpFullImage)
  {
    *ptpFullImage = NULL;
  }
  if (ptpMatImage)
  {
    *ptpMatImage = NULL;
  }

  int64_t orgDetailedWidth = pConf->width;
  int64_t orgDetailedHeight = pConf->length;
  int64_t simulatedWidth = orgDetailedWidth;
  int64_t simulatedHeight = orgDetailedHeight;
  if (useZoom)
  {
    simulatedWidth = (int64_t)lround((double)orgDetailedWidth / xZoomOut);
    simulatedHeight = (int64_t)lround((double)orgDetailedHeight / yZoomOut);
  }
  if (orientation == 90 || orientation == -90 || orientation == 270)
  {
    int64_t simulatedHeightOld = simulatedHeight;
    simulatedHeight = simulatedWidth;
    simulatedWidth = simulatedHeightOld;
  }

  cv::Mat* pImgComplete = new cv::Mat((int)simulatedHeight, (int)simulatedWidth, CV_8UC3, cv::Scalar(255, 255, 255));
  {
    safeBmp safeTemp;
    cv::Mat* pImgPart1 = new cv::Mat((int)orgDetailedWidth, (int)orgDetailedHeight, CV_8UC3, cv::Scalar(255, 255, 255));
    BYTE* pTempBmp = (BYTE*) pImgPart1->ptr();
    // bool AperioSlide::read(BYTE * pBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t width, int64_t height, bool setGrayScale, int64_t * pReadWidth, int64_t * pReadHeight)
    int64_t orgCols = 0;
    int64_t orgRows = 0;
    safeBmpInit(&safeTemp, pTempBmp, pImgPart1->cols, pImgPart1->rows);
    read(&safeTemp, level, 0, 0, 0, 0, orgDetailedWidth, orgDetailedHeight, &orgCols, &orgRows);
    cv::Mat* pImgPart = pImgPart1;
    cv::Mat imgPart2, imgPart3;
    cv::Mat* pImgScaled = NULL;
    switch (orientation)
    {
    case 0:
      break;
    case 90:
      cv::transpose(*pImgPart1, imgPart2);
      pImgPart1->release();
      cv::flip(imgPart2, imgPart3, 1);
      imgPart2.release();
      pImgPart = &imgPart3;
      break;
    case -90:
    case 270:
      cv::transpose(*pImgPart1, imgPart2);
      pImgPart1->release();
      cv::flip(imgPart2, imgPart3, 0);
      imgPart2.release();
      pImgPart = &imgPart3;
      break;
    case 180:
      cv::flip(*pImgPart1, imgPart2, -1);
      pImgPart1->release();
      pImgPart = &imgPart2;
      break;
    }
    if (useZoom)
    {
      cv::Size scaledSize((int64_t)lround(pImgPart->cols / xZoomOut), (int64_t)lround(pImgPart->rows / yZoomOut));
      pImgScaled = new cv::Mat((int64_t)scaledSize.width, (int64_t)scaledSize.height, CV_8UC3, cv::Scalar(255, 255, 255));
      cv::resize(*pImgPart, *pImgScaled, scaledSize);
      pImgPart->release();
      pImgPart = pImgScaled;
    }
    double xPixelDbl = ((double)pConf->width / (double)pConf->xAdj);
    double yPixelDbl = ((double)pConf->length / (double)pConf->yAdj);
    int64_t xPixel = (int64_t)round(xPixelDbl);
    int64_t yPixel = (int64_t)round(yPixelDbl);
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
      xPixelNew = (orgDetailedHeight - yPixel) - orgRows;
      yPixelNew = xPixel;
    }
    else if (orientation == -90 || orientation == 270)
    {
      xPixelNew = yPixel;
      yPixelNew = (orgDetailedWidth - xPixel) - orgCols;
    }
    else if (orientation == 180)
    {
      xPixelNew = (orgDetailedWidth - xPixel) - orgCols;
      yPixelNew = (orgDetailedHeight - yPixel) - orgRows;
    }
    if (useZoom)
    {
      xPixelNew = (int64_t)lround(xPixelNew / xZoomOut);
      yPixelNew = (int64_t)lround(yPixelNew / yZoomOut);
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
      cv::Rect roi(0, 0, (int)cols, (int)rows);
      cv::Mat srcRoi(*pImgPart, roi);
      cv::Rect roi2((int)xPixelNew, (int)yPixelNew, (int)cols, (int)rows);
      cv::Mat destRoi(*pImgComplete, roi2);
      srcRoi.copyTo(destRoi);
      srcRoi.release();
      destRoi.release();
    }
    else
    {
      std::cerr << "Warning: ROI outside of image boundaries: xPixelNew=" << xPixelNew << " cols=" << cols << " total width of image=" << pImgComplete->cols;
      std::cerr << " yPixelNew=" << yPixelNew << " rows=" << rows << " total height of image=" << pImgComplete->rows << std::endl;
    }
    pImgPart->release();
    if (pImgScaled)
    {
      delete pImgScaled;
      pImgScaled = NULL;
    }
    delete pImgPart1;
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
    cv::imwrite(levelFName.c_str(), *pImgComplete);
  }
  if (ptpFullImage && pImgComplete && pImgComplete->data)
  {
    safeBmp* pImageL2 = safeBmpAlloc(pConf->width, pConf->length);
    *ptpFullImage = pImageL2;
    safeBmp safeImgComplete2Ref;
    safeBmpInit(&safeImgComplete2Ref, pImgComplete->data, pConf->width, pConf->length);
    safeBmpBGRtoRGBCpy(pImageL2, &safeImgComplete2Ref);
    safeBmpFree(&safeImgComplete2Ref);
  }
  if (ptpMatImage == NULL && pImgComplete)
  {
    pImgComplete->release();
    delete pImgComplete;
    pImgComplete = NULL;
  }
  else
  {
    *ptpMatImage = pImgComplete;
  }
  return true;
}

#else

// TODO: Zoom is not enabled on this function yet

bool CompositeSlide::loadFullImage(int level, safeBmp** ptpImageL2, void** ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile)
{
  JpgIniConf* pConf = mEtc[level];
  if (pConf->mFound == false)
  {
    return false;
  }
  if (ptpImageL2 == NULL)
  {
    return false;
  }

  Magick::MagickWand* magickWand = Magick::NewMagickWand();
  Magick::PixelWand* pixelWand = Magick::NewPixelWand();
  Magick::PixelSetColor(pixelWand, "#ffffff");
  Magick::MagickSetImageType(magickWand, Magick::TrueColorType);
  Magick::MagickSetImageDepth(magickWand, 8);
  Magick::MagickSetImageAlphaChannel(magickWand, Magick::OffAlphaChannel);
  Magick::MagickSetCompression(magickWand, Magick::NoCompression);
  Magick::MagickNewImage(magickWand, pConf->width, pConf->length, pixelWand);
  Magick::MagickWand* magickWand2 = Magick::NewMagickWand();

  int64_t orgDetailedWidth = pConf->mOrgDetailedWidth;
  int64_t orgDetailedHeight = pConf->mOrgDetailedHeight;

  if (optDebug > 1)
  {
    logFile << "Reading level " << level << "." << std::endl;
  }
  for (int64_t i = 0; i < pConf->mTotalTiles; i++)
  {
    Magick::MagickSetImageType(magickWand2, Magick::TrueColorType);
    Magick::MagickSetImageDepth(magickWand2, 8);
    Magick::MagickSetImageAlphaChannel(magickWand2, Magick::OffAlphaChannel);
    //if (Magick::MagickReadImage(magickWand2, pConf->mxyArr[i].mBaseFileName.c_str()) == Magick::MagickFalse)
    {
    //  Magick::ExceptionType exType;
    //  std::cerr << "Failed to open '" << pConf->mxyArr[i].mBaseFileName << "'. Reason: " << Magick::MagickGetException(magickWand2, &exType) << std::endl;
      continue;
    }
    int64_t orgCols = (int64_t)Magick::MagickGetImageWidth(magickWand2);
    int64_t orgRows = (int64_t)Magick::MagickGetImageHeight(magickWand2);

    switch (orientation)
    {
    case 0:
      break;
    case 90:
    case -90:
    case 270:
    case 180:
      Magick::MagickRotateImage(magickWand2, pixelWand, (double)orientation);
      break;
    }
    double xPixelDbl = (double)(pConf->width / (double)pConf->xAdj);
    double yPixelDbl = (double)(pConf->length / (double)pConf->yAdj);
    int64_t xPixel = (int64_t)round(xPixelDbl);
    int64_t yPixel = (int64_t)round(yPixelDbl);
    int64_t xPixelNew = xPixel;
    int64_t yPixelNew = yPixel;
    if (orientation == 0)
    {
      // place holder, do nothing
    }
    else if (orientation == 90)
    {
      xPixelNew = (orgDetailedHeight - yPixel) - orgRows;
      yPixelNew = xPixel;
    }
    else if (orientation == -90 || orientation == 270)
    {
      xPixelNew = yPixel;
      yPixelNew = (orgDetailedWidth - xPixel) - orgCols;
    }
    else if (orientation == 180)
    {
      xPixelNew = (orgDetailedWidth - xPixel) - orgCols;
      yPixelNew = (orgDetailedHeight - yPixel) - orgRows;
    }
    Magick::MagickCompositeImage(magickWand, magickWand2, Magick::OverCompositeOp, Magick::MagickTrue, xPixelNew, yPixelNew);
    Magick::ClearMagickWand(magickWand2);
  }
  if (magickWand2) Magick::DestroyMagickWand(magickWand2);
  if (optDebug > 1)
  {
    Magick::Image* pImgComplete2 = Magick::GetImageFromMagickWand(magickWand);
    magickWand2 = Magick::NewMagickWandFromImage(pImgComplete2);
    Magick::MagickSetImageType(magickWand2, Magick::TrueColorType);
    Magick::MagickSetImageDepth(magickWand2, 8);
    Magick::MagickSetImageAlphaChannel(magickWand2, Magick::OffAlphaChannel);
    MagickSetImageCompressionQuality(magickWand2, 90);
    std::stringstream ss;
    ss << "imgComplete" << level << ".jpg";
    std::string levelFName = ss.str();
    Magick::MagickWriteImage(magickWand2, levelFName.c_str());
    Magick::DestroyMagickWand(magickWand2);
  }
  safeBmp* pImageL2 = safeBmpAlloc(pConf->width, pConf->length);
  *ptpImageL2 = pImageL2;
  Magick::MagickExportImagePixels(magickWand, 0, 0, pConf->width, pConf->length, "RGB", Magick::CharPixel, pImageL2->data);
  Magick::DestroyPixelWand(pixelWand);
  Magick::DestroyMagickWand(magickWand);
  return true;
}
#endif


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
