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
  bool open(const std::string& inputDir, int options, int debugLevel = 0); 
  bool read(safeBmp *pDestBmp, int level, int direction, int zLevel, int64_t x, int64_t y, int64_t cols, int64_t rows, int64_t *readCols, int64_t *readRows);

  safeBmp* loadFullImage(int level, int orientation, double xZoomOut, double yZoomOut, bool useZoom, int optDebug, std::ofstream& logFile);
  bool checkLevel(int level);
  bool checkZLevel(int level, int direction, int zLevel);
  bool isPreviewSlide(int level);
  bool setOrientation(int orientation, std::fstream& logFile);
  int getTotalZLevels();
  int getTotalBottomZLevels();
  int getTotalTopZLevels();
  int getQuality(int level);
  int64_t getPixelCols(int level);
  int64_t getPixelRows(int level);
  int64_t getActualCols(int level);
  int64_t getActualRows(int level);
  int64_t getLevelCols(int level);
  int64_t getLevelRows(int level);
  double getXAdj(int level);
  double getYAdj(int level);
  int64_t getTotalTiles(int level);
  bool parseMagFromDesc(std::string desc);
};  

#endif
