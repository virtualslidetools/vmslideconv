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
#include <vector>
#include <algorithm>
#include "tilecachesupport.h"

TileCache tileCache;

void TileCache::release(Tile *ptile)
{
  for (std::vector<Tile*>::iterator it=mtiles.begin(); it<mtiles.end(); it++)
  {
    if (*it == ptile)
    {
      mtiles.erase(it);
      break;
    }
  }
  delete ptile;
}

void TileCache::releaseAll()
{
  for (std::vector<Tile*>::iterator it=mtiles.begin(); it <mtiles.end(); it++)
  {
    delete *it;
    *it=NULL;
  }
  mtiles.clear();
}

BYTE* TileCache::find(void* pObject, int level, int64_t tileno, int64_t* pBytesRead)
{
  Tile *tile = 0;
  std::vector<Tile*>::iterator it;
  if (mtiles.size() > 0)
  {
    for (it=mtiles.begin(); it < mtiles.end(); it++)
    {
      tile = *it;  
      if (tile->getObjPtr()==pObject && tile->getLevel()==level && tile->getTileNo()==tileno)
      {
        it=mtiles.erase(it);
        if (mtiles.size() > 0)
        {
          mtiles.insert(mtiles.begin(), tile);
        }
        else
        {
          mtiles.push_back(tile);
        }
        *pBytesRead = tile->getBytesRead();
        return tile->getBmpPtr();
      }
    }
  }
  return NULL;
}

void TileCache::add(void* pObject, int level, int64_t tileno, int64_t bytesRead, BYTE *pBmp)
{
  Tile* tile = new Tile(pObject, level, tileno, bytesRead, pBmp);
  if (mtiles.size()+1 > mMaxOpen)
  {
    Tile *lastTile=mtiles.back();
    delete lastTile;
    mtiles.pop_back();
  }
  if (mtiles.size() > 0)
  {
    mtiles.insert(mtiles.begin(), tile);
  }
  else
  {
    mtiles.push_back(tile);
  }
}
