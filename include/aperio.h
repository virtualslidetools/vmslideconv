#ifndef __APERIO_FILE_H
#define __APERIO_FILE_H

#include "virtualslide.h"
#include "tiffsupport.h"

class AperioSlide : public VirtualSlide {
protected:
  Tiff mTif;
public:
  AperioSlide() { }
  virtual ~AperioSlide() { mTif.close(); }
  void close() { mTif.close(); baseClearAttribs(); }
  int getTotalLevels();
  bool open(const std::string& inputDir, int options, int orientation, int debugLevel = 0, int64_t bestXOffset = -1, int64_t bestYOffset = -1, safeBmp **pImageL2 = NULL); 
  bool read(safeBmp *pDestBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t width, int64_t height, int64_t *readWidth, int64_t *readHeight);

  #ifndef USE_MAGICK
  bool findXYOffset(int lowerLevel, int higherLevel, int64_t *bestXOffset0, int64_t *bestYOffset0, int64_t *bestXOffset1, int64_t *bestYOffset1, int optUseCustomOffset, int debugLevel, std::fstream& logFile);
  #endif

  #ifndef USE_MAGICK
  bool loadFullImage(int level, safeBmp **ptpFullImage, cv::Mat **ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile);
  #else
  bool loadFullImage(int level, safeBmp **ptpImageL2, void **ptpMatImage, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::fstream& logFile);
  #endif
 
  bool checkLevel(int level);
  bool checkZLevel(int level, int direction, int zLevel);
  bool isPreviewSlide(int level);
  bool setOrientation(int orientation, std::fstream& logFile);
  int getTotalZLevels();
  int getTotalBottomZLevels();
  int getTotalTopZLevels();
  int getQuality(int level);
  int64_t getPixelWidth(int level);
  int64_t getPixelHeight(int level);
  int64_t getActualWidth(int level);
  int64_t getActualHeight(int level);
  int64_t getLevelWidth(int level);
  int64_t getLevelHeight(int level);
  double getXAdj(int level);
  double getYAdj(int level);
  int64_t getTotalTiles(int level);
  bool parseMagFromDesc(std::string desc);
};  

#endif
