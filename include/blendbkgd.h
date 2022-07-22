#ifndef __BLENDBKGD_FILE_H
#define __BLENDBKGD_FILE_H

class BlendSection
{
protected:
  int64_t mStart, mFree;
  BlendSection *mNext;
public:
  BlendSection()
  {
    mStart = 0;
    mFree = 0;
    mNext = NULL;
  }
  BlendSection(int64_t value)
  {
    mStart = value;
    mFree = 0;
    mNext = NULL;
  }  
  inline void setStart(int64_t value) { mStart = value; }
  inline void clearFree() { mFree = 0; }
  inline void incrementFree() { mFree++; }
  inline void setNext(BlendSection *value) { mNext = value; }
  inline int64_t getStart() { return mStart; }
  inline int64_t getFree() { return mFree; }
  inline BlendSection* getNext() { return mNext; }
  inline void setFree(int64_t free) { mFree = free; }
};

typedef struct 
{
  safeBmp* pSafeDest;
  safeBmp* pSafeSrcL2;
  double xSrc, ySrc;
  double grabColsB, grabRowsB;
  double xFactor, yFactor;
  int64_t xMargin, yMargin;
  BlendSection **yFreeMap;
  int64_t ySize;
} BlendArgs;

void blendLevelsFree(BlendSection** yFreeMap, int64_t ySize);
void blendLevels(BlendArgs *args);

#endif
