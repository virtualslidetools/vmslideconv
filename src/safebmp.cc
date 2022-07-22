#include <cassert>
#include <cstring>
#include <new>
#include <cstdlib>
#include <iostream>
#include "safebmp.h"

#ifdef USE_OPENCV
#include <opencv2/opencv.hpp>
#endif

#ifdef USE_MAGICK
namespace Magick
{
#include <MagickCore/MagickCore.h>
#include <MagickWand/MagickWand.h>
}
const char* getMagickCoreCoderPath();
const char* getMagickCoreFilterPath();
const char* getMagickCoreLibraryPath();
#endif 

safeBmp * safeBmpAlloc(int64_t cols, int64_t rows)
{
  safeBmp *bmp=NULL;
  
  int64_t strideCols = cols * 3;
  try 
  {
    bmp = new safeBmp;
    bmp->data = NULL;
    if (strideCols > 0 && rows > 0)
    {
      bmp->data = new BYTE[strideCols * rows];
      bmp->rows = rows;
      bmp->cols = cols;
      bmp->strideCols = strideCols;
    }
  }
  catch (std::bad_alloc &xa)
  {
    (void) xa;
    std::cerr << "Failed to allocate image of memory size=" << ((strideCols * rows) / 1024) << " kb." << std::endl;
    exit(1);
  }
  if (bmp && bmp->data)
  {
    bmp->freeStruct = true;
    bmp->freeData = true;
  }
  else
  {
    std::cerr << "Failed to allocate image of memory size=" << ((strideCols * rows) / 1024) << " kb." << std::endl;
    exit(1);
  }
  return bmp;
}


BYTE* safeBmpAlloc2(safeBmp *bmp, int64_t cols, int64_t rows)
{
  bmp->data = NULL;

  int64_t strideCols = cols * 3;
  try
  {
    if (strideCols > 0 && rows > 0)
    {
      bmp->data = new BYTE[strideCols * rows];
    }
  }
  catch (std::bad_alloc &xa)
  {
    (void) xa;
    bmp->data = NULL;
    std::cerr << "Failed to allocate image of memory size=" << ((strideCols * rows) / 1024) << " kb." << std::endl;
    exit(1);
  }
  if (bmp->data)
  {
    bmp->freeStruct = false;
    bmp->freeData = true;
    bmp->strideCols = strideCols;
    bmp->rows = rows;
    bmp->cols = cols;
  }
  else
  {
    std::cerr << "Failed to allocate image of memory size=" << ((strideCols * rows) / 1024) << " kb." << std::endl;
    exit(1);
  }
  return bmp->data;
}


BYTE* safeBmpReAlloc2(safeBmp *bmp, int64_t cols, int64_t rows)
{
  int64_t strideCols = cols * 3;
  
  if (bmp->cols != cols || bmp->rows != rows)
  {
    try
    {
      if (bmp->freeData && bmp->data)
      {
        delete[] bmp->data;
        bmp->data = NULL;
        bmp->freeData = false;
        bmp->strideCols = 0;
        bmp->cols = 0;
        bmp->rows = 0;
      }
      if (strideCols > 0 && rows > 0)
      {
        bmp->data = new BYTE[strideCols * rows];
      }
    }
    catch (std::bad_alloc &xa)
    {
      (void) xa;
      bmp->data = NULL;
      std::cerr << "Failed to allocate image of memory size=" << ((strideCols * rows) / 1024) << " kb." << std::endl;
      exit(1);
    }
    if (bmp->data)
    {
      bmp->freeData = true;
      bmp->strideCols = strideCols;
      bmp->rows = rows;
      bmp->cols = cols;
    }
    else
    {
      std::cerr << "Failed to allocate image of memory size=" << ((strideCols * rows) / 1024) << " kb." << std::endl;
      exit(1);
    }
  }
  return bmp->data;
}


safeBmp* safeBmpSrc(BYTE *data, int64_t cols, int64_t rows)
{
  safeBmp* bmp=new safeBmp;
  if (!bmp) return NULL;

  bmp->cols = cols;
  bmp->strideCols = cols * 3;
  bmp->rows = rows;
  bmp->data = data;
  bmp->freeStruct = true;
  bmp->freeData = false;

  return bmp;
}


void safeBmpInit(safeBmp *bmp, BYTE *bmpPtr, int64_t cols, int64_t rows)
{
  bmp->cols = cols;
  bmp->strideCols = cols * 3;
  bmp->rows = rows;
  bmp->data = bmpPtr;
  bmp->freeStruct = false;
  bmp->freeData = false;
}


void safeBmpFree(safeBmp *bmp)
{
  if (!bmp) return;
  if (bmp->freeData && bmp->data)
  {
    delete[] bmp->data;
    bmp->data = NULL;
    bmp->cols = 0;
    bmp->rows = 0;
    bmp->strideCols = 0;
    bmp->freeData = false;
  }
  if (bmp->freeStruct)
  {
    bmp->freeStruct = false;
    delete bmp;
  }
}


void safeBmpUint32Set(safeBmp *bmp, uint32_t value)
{
  uint64_t size=bmp->cols * bmp->rows;
  uint32_t *data = (uint32_t*) bmp->data;
  for (uint64_t i=0; i < size; i++) data[i] = value;
}


void safeBmpCpy(safeBmp *bmpDest, int64_t xDest, int64_t yDest, safeBmp *bmpSrc, int64_t xSrc, int64_t ySrc, int64_t cols, int64_t rows)
{
  int64_t xEnd = xSrc + cols;
  if (xSrc < 0)
  {
    cols += xSrc;
    xSrc = 0;
  }
  if (xEnd < 0 || xSrc > bmpSrc->cols || cols <= 0 ||
      xDest > bmpDest->cols) 
  {
    return;
  }
  if (xEnd > bmpSrc->cols)
  {
    cols = bmpSrc->cols - xSrc;
    if (cols < 0) return;
  }
  int64_t yEnd = ySrc + rows;
  if (ySrc < 0)
  {
    rows += ySrc;
    ySrc = 0;
  }
  if (yEnd < 0 || ySrc > bmpSrc->rows || rows <= 0 ||
      yDest > bmpDest->rows) 
  {
    return;
  }
  if (yEnd > bmpSrc->rows)
  {
    rows = bmpSrc->rows - ySrc;
  }
  if (xDest < 0)
  {
    cols += xDest;
    xDest = 0;
  }
  if (xDest + cols > bmpDest->cols)
  {
    cols = bmpDest->cols - xDest;
  }
  if (yDest < 0)
  {
    rows += yDest;
    yDest = 0;
  }
  if (yDest + rows > bmpDest->rows)
  {
    rows = bmpDest->rows - yDest;
  }
  if (rows < 0 || cols <= 0) return;
  int64_t srcRowCols = bmpSrc->strideCols;
  int64_t destRowCols = bmpDest->strideCols;
  int64_t xSrcOffset = xSrc * 3;
  int64_t xDestOffset = xDest * 3;
  int64_t colCopySize=cols*3;
  BYTE *destData = (BYTE*) bmpDest->data;
  BYTE *srcData = (BYTE*) bmpSrc->data;
  for (int64_t y = 0; y < rows; y++)
  {
    int64_t src = ((y+ySrc) * srcRowCols) + xSrcOffset;
    int64_t dest = ((y+yDest) * destRowCols) + xDestOffset;
    memcpy(&destData[dest], &srcData[src], colCopySize);
  }
}


void safeBmpBGRtoRGBCpy(safeBmp *bmpDest, safeBmp *bmpSrc)
{
  int64_t xSrc = 0;
  int64_t ySrc = 0;
  int64_t xDest = 0;
  int64_t yDest = 0;
  int64_t cols = bmpSrc->cols;
  int64_t rows = bmpSrc->rows;

  int64_t xEnd = xSrc + cols;
  if (xSrc < 0)
  {
    cols += xSrc;
    xSrc = 0;
  }
  if (xEnd < 0 || xSrc > bmpSrc->cols || cols <= 0 ||
      xDest > bmpDest->cols) 
  {
    return;
  }
  if (xEnd > bmpSrc->cols)
  {
    cols = bmpSrc->cols - xSrc;
    if (cols < 0) return;
  }
  int64_t yEnd = ySrc + rows;
  if (ySrc < 0)
  {
    rows += ySrc;
    ySrc = 0;
  }
  if (yEnd < 0 || ySrc > bmpSrc->rows || rows <= 0 ||
      yDest > bmpDest->rows) 
  {
    return;
  }
  if (yEnd > bmpSrc->rows)
  {
    rows = bmpSrc->rows - ySrc;
  }
  if (xDest < 0)
  {
    cols += xDest;
    xDest = 0;
  }
  if (xDest + cols > bmpDest->cols)
  {
    cols = bmpDest->cols - xDest;
  }
  if (yDest < 0)
  {
    rows += yDest;
    yDest = 0;
  }
  if (yDest + rows > bmpDest->rows)
  {
    rows = bmpDest->rows - yDest;
  }
  if (rows < 0 || cols <= 0) return;
  int64_t srcRowCols = bmpSrc->strideCols;
  int64_t destRowCols = bmpDest->strideCols;
  int64_t xSrcOffset = xSrc * 3;
  int64_t xDestOffset = xDest * 3;
  int64_t colCopySize=cols*3;
  BYTE *destData = (BYTE*) bmpDest->data;
  BYTE *srcData = (BYTE*) bmpSrc->data;
  for (int64_t y = 0; y < rows; y++)
  {
    int64_t src = ((y+ySrc) * srcRowCols) + xSrcOffset;
    int64_t dest = ((y+yDest) * destRowCols) + xDestOffset;
    //memcpy(&destData[dest], &srcData[src], colCopySize);
    int64_t end = src + colCopySize;
    while (src < end)
    {
      // BGR to RGB
      destData[dest+2] = srcData[src];
      destData[dest+1] = srcData[src+1];
      destData[dest] = srcData[src+2];
      dest += 3;
      src += 3;
    }
  }
}


void safeBmpRotate(safeBmp *bmpDest, safeBmp *bmpSrc, int orientation)
{
  int64_t srcCols = bmpSrc->cols;
  int64_t srcRows = bmpSrc->rows;
  int64_t srcLineCols = srcCols * 3;
  BYTE *in = bmpSrc->data;
  BYTE *out = bmpDest->data;

  if (orientation == 90)
  {
    if (srcCols > bmpDest->rows || srcRows > bmpDest->cols) return;
    int64_t destLineCols = bmpDest->cols * 3;
    for (int64_t xSrc=0; xSrc < srcCols; xSrc++)
    {
      int64_t xOffset = xSrc * 3;
      BYTE *outLine = out + (xSrc * destLineCols);
      for (int64_t ySrc=0; ySrc < srcRows; ySrc++)
      {
        BYTE* inLine = in + ((srcRows - ySrc - 1) * srcLineCols);
        inLine += xOffset;
        *outLine++ = *inLine++;
        *outLine++ = *inLine++;
        *outLine++ = *inLine++;
      }
    }
  }
  else if (orientation == 180)
  {
    if (srcCols > bmpDest->cols || srcRows > bmpDest->rows) return;
    BYTE *inLine = in;
    int64_t destLineCols = bmpDest->cols * 3;
    int64_t xDestAlign = (destLineCols - srcLineCols) + 3;
    for (int64_t ySrc=0; ySrc < srcRows; ySrc++)
    {
      BYTE *outLine = out + (((srcRows - ySrc)) * destLineCols) - xDestAlign;
      for (int64_t xSrc=0; xSrc < srcCols; xSrc++)
      {
        *outLine++ = *inLine++;
        *outLine++ = *inLine++;
        *outLine++ = *inLine++;
        outLine -= 6;
      }
    }
  }
  else if (orientation == 270 || orientation == -90)
  {
    if (srcCols > bmpDest->rows || srcRows > bmpDest->cols) return;
    int64_t destLineCols = srcRows * 3;
    for (int64_t xSrc=0; xSrc < srcCols; xSrc++)
    {
      BYTE* outLine = out + (srcCols - xSrc - 1) * destLineCols;
      int64_t xSrcOffset = xSrc * 3;
      for (int64_t ySrc=0; ySrc < srcRows; ySrc++)
      {
        BYTE* inLine = in + (ySrc * srcLineCols) + xSrcOffset;
        *outLine++ = *inLine++;
        *outLine++ = *inLine++;
        *outLine++ = *inLine++;
      }
    }
  }
}


safeBmp* safeBmpZoom(safeBmp* pBmpSrc, int64_t newCols, int64_t newRows, double xScaleResize, double yScaleResize, int scaleMethod, safeBmpZoomRes* pZoomRes)
{
  #ifdef USE_OPENCV
  (void) pZoomRes;
  cv::Mat* pImgScaled = new cv::Mat;
  // cv::Mat takes the number of rows first, than columns
  cv::Mat imgSrc((int) pBmpSrc->rows, (int) pBmpSrc->cols, CV_8UC3, pBmpSrc->data);
  cv::Size scaledSize((int) newCols, (int)newRows);
  cv::resize(imgSrc, *pImgScaled, scaledSize, xScaleResize, yScaleResize, scaleMethod == SAFEBMP_BEST_ENLARGE ? cv::INTER_CUBIC : cv::INTER_AREA);
  imgSrc.release();
  safeBmp *pBmpDest = safeBmpAlloc(pImgScaled->cols, pImgScaled->rows);
  memcpy(pBmpDest->data, pImgScaled->data, pImgScaled->cols * pImgScaled->rows * 3);
  pImgScaled->release();
  delete pImgScaled;

  #else

  Magick::MagickSetCompression(pZoomRes->magickWand, Magick::NoCompression);
  Magick::MagickSetImageType(pZoomRes->magickWand, Magick::TrueColorType);
  Magick::MagickSetImageDepth(pZoomRes->magickWand, 8);
  Magick::MagickSetImageAlphaChannel(pZoomRes->magickWand, Magick::OffAlphaChannel);
  Magick::MagickNewImage(pZoomRes->magickWand, pBmpSrc->cols, pBmpSrc->rows, pZoomRes->pixelWand);
  Magick::MagickImportImagePixels(pZoomRes->magickWand, 0, 0, pBmpSrc->cols, pBmpSrc->rows, "RGB", Magick::CharPixel, pBmpSrc->data);
  //Magick::MagickConstituteImage(l.magickWand, l.grabColsL2, l.grabRowsL2, "RGB", Magick::CharPixel, bitmapL2Mini.data);
  Magick::MagickResizeImage(pZoomRes->magickWand, newCols, newRows, scaleMethod == SAFEBMP_BEST_ENLARGE ? Magick::MitchellFilter : Magick::LanczosFilter);
  safeBmp *pBmpDest = safeBmpAlloc(newCols, newRows);
  Magick::MagickExportImagePixels(pZoomRes->magickWand, 0, 0, newCols, newRows, "RGB", Magick::CharPixel, pBmpDest->data);
  Magick::ClearMagickWand(pZoomRes->magickWand);
  #endif
  return pBmpDest;
}


void safeBmpZoom2(safeBmp* pBmpDest, safeBmp* pBmpSrc, int64_t newCols, int64_t newRows, double xScaleResize, double yScaleResize, int scaleMethod, safeBmpZoomRes* pZoomRes)
{
  #ifdef USE_OPENCV
  (void) pZoomRes;
  cv::Mat* pImgScaled = new cv::Mat;
  // cv::Mat takes the number of rows first, than columns
  cv::Mat imgSrc((int) pBmpSrc->rows, (int) pBmpSrc->cols, CV_8UC3, pBmpSrc->data);
  cv::Size scaledSize((int) newCols, (int)newRows);
  cv::resize(imgSrc, *pImgScaled, scaledSize, xScaleResize, yScaleResize, scaleMethod == SAFEBMP_BEST_ENLARGE ? cv::INTER_CUBIC : cv::INTER_AREA);
  imgSrc.release();
  safeBmpAlloc2(pBmpDest, pImgScaled->cols, pImgScaled->rows);
  memcpy(pBmpDest->data, pImgScaled->data, pImgScaled->cols * pImgScaled->rows * 3);
  pImgScaled->release();
  delete pImgScaled;

  #else
  
  Magick::MagickSetCompression(pZoomRes->magickWand, Magick::NoCompression);
  Magick::MagickSetImageType(pZoomRes->magickWand, Magick::TrueColorType);
  Magick::MagickSetImageDepth(pZoomRes->magickWand, 8);
  Magick::MagickSetImageAlphaChannel(pZoomRes->magickWand, Magick::OffAlphaChannel);
  Magick::MagickNewImage(pZoomRes->magickWand, pBmpSrc->cols, pBmpSrc->rows, pZoomRes->pixelWand);
  Magick::MagickImportImagePixels(pZoomRes->magickWand, 0, 0, pBmpSrc->cols, pBmpSrc->rows, "RGB", Magick::CharPixel, pBmpSrc->data);
  //Magick::MagickConstituteImage(l.magickWand, l.grabColsL2, l.grabRowsL2, "RGB", Magick::CharPixel, bitmapL2Mini.data);
  Magick::MagickResizeImage(pZoomRes->magickWand, newCols, newRows, scaleMethod == SAFEBMP_BEST_ENLARGE ? Magick::MitchellFilter : Magick::LanczosFilter);
  safeBmpAlloc2(pBmpDest, newCols, newRows);
  Magick::MagickExportImagePixels(pZoomRes->magickWand, 0, 0, newCols, newRows, "RGB", Magick::CharPixel, pBmpDest->data);
  Magick::ClearMagickWand(pZoomRes->magickWand);
  #endif
}


void quickEnv(const char* var, const char* value, int optDebug)
{
  char fullChar[512];
  std::string full=var;
  full.append("=");
  full.append(value);
  const char * fullConst = full.c_str();
  fullChar[0] = 0;

  #ifdef HAVE_STRNCPY_S
  strncpy_s(fullChar, sizeof(fullChar)-1, fullConst, full.size());
  #else
  strncpy(fullChar, fullConst, sizeof(fullChar)-1);
  #endif

  if (optDebug > 0)
  {
    std::cout << "ENV: " << fullChar << std::endl;
  }
  #ifdef HAVE__PUTENV
  _putenv(fullChar);
  #else
  putenv(fullChar);
  #endif
}


void safeBmpEnvSetup(int optDebug)
{
  (void) optDebug;
  #ifdef USE_MAGICK
  #if defined(__MINGW32__) || defined(__MINGW64__)
  quickEnv("MAGICK_CODER_MODULE_PATH", getMagickCoreCoderPath(), optDebug);
  quickEnv("MAGICK_CODER_FILTER_PATH", getMagickCoreFilterPath(), optDebug);
  #endif
  quickEnv("MAGICK_MAP_LIMIT", MAGICK_MAP_LIMIT, optDebug);
  quickEnv("MAGICK_MEMORY_LIMIT", MAGICK_MEMORY_LIMIT, optDebug);
  quickEnv("MAGICK_DISK_LIMIT", MAGICK_DISK_LIMIT, optDebug);
  quickEnv("MAGICK_AREA_LIMIT", MAGICK_AREA_LIMIT, optDebug);
  Magick::MagickWandGenesis();
  #endif
}


void safeBmpEnvCleanup()
{
  #ifdef USE_MAGICK
  Magick::MagickWandTerminus();
  #endif
}


safeBmpZoomRes* safeBmpZoomResInit()
{
  safeBmpZoomRes *pZoomRes = NULL;

  #ifdef USE_MAGICK
  pZoomRes = new safeBmpZoomRes;
  pZoomRes->magickWand = Magick::NewMagickWand();
  Magick::MagickSetCompression(pZoomRes->magickWand, Magick::NoCompression);
  Magick::MagickSetImageType(pZoomRes->magickWand, Magick::TrueColorType);
  Magick::MagickSetImageDepth(pZoomRes->magickWand, 8);
  Magick::MagickSetImageAlphaChannel(pZoomRes->magickWand, Magick::OffAlphaChannel);
  pZoomRes->pixelWand = Magick::NewPixelWand();
  Magick::PixelSetColor(pZoomRes->pixelWand, "#ffffff");
  #endif

  return pZoomRes;
}


void safeBmpZoomResFree(safeBmpZoomRes *pZoomRes)
{
  (void) pZoomRes;
  #ifdef USE_MAGICK
  if (pZoomRes)
  {
    if (pZoomRes->magickWand) 
    {
      Magick::DestroyMagickWand(pZoomRes->magickWand);
      pZoomRes->magickWand = NULL;
    }
    if (pZoomRes->pixelWand)
    {
      Magick::DestroyPixelWand(pZoomRes->pixelWand);
      pZoomRes->pixelWand = NULL;
    }
    delete pZoomRes;
  }
  #endif
}
