/*******************************************************************************
Copyright (c) 2005-2016, Paul F. Richards

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*******************************************************************************/
#ifndef __IMAGESUPPORT_FILE_H
#define __IMAGESUPPORT_FILE_H

#include <string>
#include <sstream>
#include <vector>
#include <cmath>
#include <cstdint>

#ifndef uint
typedef unsigned int uint;
#endif

#ifndef BYTE
typedef uint8_t BYTE;
#endif

// Visual C++ 2005 and below does not contain lroundf

#ifdef _MSC_VER
#ifndef lroundf
inline long int lroundf(float x) { return (long int) floorf(x+(float)0.5); }
#endif
#ifndef lround
inline long int lround(double x) { return (long int) floor(x+0.5); }
#endif
#endif

#ifndef iround
inline int iround(double x) { return (int) lround(x); }
#endif

int dprintf(const char* format, ...);

class Image {
protected:
  int64_t mActualWidth, mActualHeight;
  int mBitCount;
  int mSamplesPerPixel;
  int mLevel;
  int64_t mReadWidth, mReadHeight;
  bool mValidObject;
  std::string mFileName;
  std::vector<BYTE> mInfo;
  int64_t mBitmapSize;
  BYTE mBkgColor;
  bool mGrayScale;
public:
  std::ostringstream mErrMsg;
public:
  Image() { baseClearAttribs(); }
  virtual ~Image() { baseCleanup(); }
  void baseClearAttribs();
  void baseCleanup();
  void setFileName(const std::string& newFileName) { mFileName = newFileName; }
  std::string getFileName() { return mFileName; }
  virtual bool open(const std::string& newFileName, bool setGrayScale = false) = 0;
  int getSamplesPerPixel() { return mSamplesPerPixel; }
  int64_t getActualWidth()  { return (mValidObject) ? mActualWidth : 0; }
  int64_t getActualHeight() { return (mValidObject) ? mActualHeight : 0; }
  int64_t getReadWidth() { return (mValidObject) ? mReadWidth : 0; }
  int64_t getReadHeight() { return (mValidObject) ? mReadHeight : 0; }
  bool isValidObject() { return mValidObject; }
  void getErrMsg(std::string& errStr) { errStr = mErrMsg.str(); }
  void setUnfilledColor(BYTE newColor) { mBkgColor = newColor; }
  bool getGrayScale() { return mGrayScale; }
};

Image* loadImage(const char* fileName, std::string& errStr);

class DummyImage : public Image {
public:
  bool open(const std::string&) { return false; }
  void cleanup() { }
  DummyImage() { mValidObject = false; }
  ~DummyImage() { } 
};

#endif // __IMAGESUPPORT_FILE_H
