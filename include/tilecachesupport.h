/**************************************************************************
Initial author: Paul F. Richards (paulrichards321@gmail.com) 2016-2017
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
*************************************************************************/
#ifndef __TILECACHESUPPORT_FILE_H
#define __TILECACHESUPPORT_FILE_H

#include <vector>
#include <algorithm>
#include <cstdint>

#ifndef BYTE
typedef uint8_t BYTE;
#endif

class Tile
{
protected:
  void *mObjPtr;
  int mLevel;
  int64_t mTileNo;
  int64_t mBytesRead;
  BYTE* mpBmp;
public:
  Tile(void *pObj, int level, int64_t tileno, int64_t bytesRead, BYTE* pBmp) { mObjPtr = pObj; mLevel = level; mTileNo = tileno; mBytesRead = bytesRead; mpBmp = pBmp; }
  ~Tile() { if (mpBmp) { BYTE* pBmp = mpBmp; mpBmp = 0; delete[] pBmp; } }
  void* getObjPtr() { return mObjPtr; }
  int getLevel() { return mLevel; }
  int64_t getTileNo() { return mTileNo; }
  int64_t getBytesRead() { return mBytesRead; }
  BYTE* getBmpPtr() { return mpBmp; }
};

class TileCache
{
protected:
  unsigned int mMaxOpen;
  std::vector<Tile*> mtiles;
public:
  TileCache(unsigned int maxOpen) { mMaxOpen = maxOpen; }
  TileCache() { mMaxOpen = 1000; }
  ~TileCache() { releaseAll(); }
  void release(Tile *ptile);
  void releaseAll();
  BYTE* find(void* pObject, int level, int64_t tileno, int64_t* pBytesRead);
  void add(void* pObject, int level, int64_t tileno, int64_t bytesRead, BYTE *pBmp);
};

#endif
