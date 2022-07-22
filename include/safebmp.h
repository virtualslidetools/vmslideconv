#ifndef __SAFEBMP_FILE_H
#define __SAFEBMP_FILE_H

#ifdef __cplusplus
#include <cstdint>
#include <cstring>
#else
#include <string.h>
#endif
#include <string>

#ifndef BYTE
typedef uint8_t BYTE;
#endif 

#define SAFEBMP_BEST_ENLARGE    0x0001
#define SAFEBMP_BEST_SHRINK     0x0002

typedef struct 
{
  int64_t cols;
  int64_t rows;
  int64_t strideCols;
  BYTE *data;
  bool freeStruct;
  bool freeData;
} safeBmp;

typedef struct
{
#ifdef USE_MAGICK
  Magick::MagickWand *magickWand;
  Magick::MagickPixelWand *magickPixelWand;
#else
  int unused;
#endif
} safeBmpZoomRes;

void safeBmpEnvSetup(int optDebug);
void safeBmpEnvCleanup();
safeBmp* safeBmpAlloc(int64_t cols, int64_t rows);
uint8_t* safeBmpAlloc2(safeBmp*, int64_t cols, int64_t rows);
uint8_t* safeBmpReAlloc2(safeBmp*, int64_t cols, int64_t rows);
void safeBmpInit(safeBmp *bmp, BYTE *data, int64_t cols, int64_t rows);
void safeBmpUint32Set(safeBmp *bmp, uint32_t value);
inline void safeBmpByteSet(safeBmp *bmp, int value) { memset(bmp->data, value, bmp->strideCols * bmp->rows); }
void safeBmpFree(safeBmp *bmp);
safeBmp* safeBmpSrc(BYTE* data, int64_t cols, int64_t rows);
void safeBmpCpy(safeBmp *bmpDest, int64_t xDest, int64_t yDest, safeBmp *bmp_src, int64_t xSrc, int64_t ySrc, int64_t cols, int64_t rows);
inline void safeBmpClear(safeBmp *bmp) { bmp->cols = 0; bmp->rows = 0; bmp->strideCols = 0; bmp->data = NULL; bmp->freeStruct = false; bmp->freeData = false; }
void safeBmpBGRtoRGBCpy(safeBmp *bmpDest, safeBmp *bmpSrc);
void safeBmpRotate(safeBmp *bmpDest, safeBmp *bmpSrc, int orientation);

safeBmpZoomRes* safeBmpZoomResInit();
void safeBmpZoomResFree(safeBmpZoomRes *pZoomRes);
safeBmp* safeBmpZoom(safeBmp *bmpSrc, int64_t newCols, int64_t newRows, double xScaleResize, double yScaleResize, int scaleMethod, safeBmpZoomRes* pZoomRes); 
void safeBmpZoom2(safeBmp *bmpDest, safeBmp *bmpSrc, int64_t newCols, int64_t newRows, double xScaleResize, double yScaleResize, int scaleMethod, safeBmpZoomRes* pZoomRes);

int safeJpgWrite(safeBmp *pBmpSrc, const std::string& newFileName, int quality, std::string* pErrMsg);
int safeJpgRead(safeBmp *pBmpDest, const std::string& fileName, int bkgColor, std::string* pErrMsg);
int safeJpgCompress(safeBmp* pBmpSrc, BYTE** ptpCompressedBitmap, int quality, std::string* pErrMsg, unsigned long *pOutSize);
void safeJpgCompressFree(BYTE** ptpCompressedBitmap);

#endif
