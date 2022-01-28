/**************************************************************************
Initial author: Paul F. Richards (paulrichards321@gmail.com) 2005-2017
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
#ifdef _MSC_VER
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <new>
#include <cstdio>
#include <memory>
#include <cmath>
#include <cerrno>
#include <cstring>
#include <iostream>
#include "jpgsupport.h"

int reset_error_mjr(j_common_ptr cinfo)
{
  return 0;
}

/*
 * ERROR HANDLING:
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */

/*
 * Here's the routine that will replace the standard error_exit method:
 */

void my_jpeg_error_exit(j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  //struct jpeg_error_mgr* pjerr = cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  /* (*cinfo->err->output_message) (cinfo); */

  /* Return control to the setjmp point */
  throw cinfo->err; 
  //longjmp(myerr->setjmp_buffer, 1);
}


void Jpg::jpgClearAttribs()
{
  safeBmpClear(&mFullSrc);
}


void Jpg::jpgCleanup()
{
  safeBmpFree(&mFullSrc);
}


bool Jpg::testHeader(BYTE* header, int)
{
  if (header[0] == 0xFF && header[1] == 0xD8)
    return true;
  else
    return false;
}


bool Jpg::open(const std::string& newFileName, bool setGrayScale)
{
  FILE *infile = NULL;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW* pjSampleRows = 0; 
  
  if (mValidObject)
  {
    jpgCleanup();
    baseCleanup();
    jpgClearAttribs();
    baseClearAttribs();
  }
  memset(&cinfo, 0, sizeof(cinfo));
  memset(&jerr, 0, sizeof(jerr));

  mFileName = newFileName;

  infile = fopen(mFileName.c_str(), "rb");
  if (infile == 0) 
  {
    mErrMsg << "Error opening '" << mFileName << "': " << std::strerror(errno);
    return false;
  }

  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = my_jpeg_error_exit;

  try 
  {
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    if (cinfo.num_components==3) 
    {
      mSamplesPerPixel = 3;
    }
    else
    {
      mErrMsg << "Warning: '" << mFileName << "' has unsupported samples per pixel: " << cinfo.num_components << std::endl;
      return false;
    }
    jpeg_start_decompress(&cinfo);
    mActualWidth = cinfo.output_width;
    mActualHeight = cinfo.output_height;
    mBitCount = mSamplesPerPixel * 8;
    safeBmpAlloc2(&mFullSrc, mActualWidth, mActualHeight);
    mValidObject=true;    
    safeBmpByteSet(&mFullSrc, mBkgColor);
    int64_t destLineWidth = mActualWidth * mSamplesPerPixel;
    int64_t totalScanlines=0;
    pjSampleRows = new JSAMPROW[mActualHeight];
    for (int64_t y = 0; y < mActualHeight; y++)
    {
      pjSampleRows[y] = mFullSrc.data + (y * destLineWidth);
    }
    while (cinfo.output_scanline < cinfo.output_height) 
    {
      totalScanlines += jpeg_read_scanlines(&cinfo, &pjSampleRows[totalScanlines], mActualHeight-totalScanlines);
    }
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    delete[] pjSampleRows;
    pjSampleRows = 0;
    fclose(infile);
    infile = 0;
  }
  catch (jpeg_error_mgr* pJerr) 
  {
    mErrMsg << "Error decompressing '" << mFileName << "': ";
    mErrMsg << pJerr->jpeg_message_table[pJerr->msg_code];
    jpeg_destroy_decompress(&cinfo);
    if (infile) fclose(infile);
    if (pjSampleRows)
    {
      delete[] pjSampleRows;
    }
    return false;
  } 
  catch (std::bad_alloc &e) 
  {
    jpeg_destroy_decompress(&cinfo);
    if (infile) fclose(infile);
    if (pjSampleRows)
    {
      delete[] pjSampleRows;
    }
    mErrMsg << "Insufficient memory to decompress '" << mFileName;
    mErrMsg << "' into memory";
    throw std::runtime_error(mErrMsg.str());
  }
  return true;
}


bool Jpg::read(safeBmp *pBmpDest, int64_t x, int64_t y, int64_t width, int64_t height)
{
  if (mValidObject==false) return false;
  mReadWidth=0;
  mReadHeight=0;
  mErrMsg.str("");
  if (x<0 || y<0 || x>mActualWidth || y>mActualHeight || height<=0 || width<=0) 
  {
    std::cerr << "In jpeg::read parameters out of bounds." << std::endl;
    return false;
  }
  if (x+width > mActualWidth)
  {
    std::cerr << "In jpeg::read, width truncated. Actual width=" << mActualWidth;
    std::cerr << " x=" << x << " width=" << width << std::endl;
    width = mActualWidth - x;
  }
  if (y+height > mActualHeight)
  {
    std::cerr << "In jpeg::read, height truncated. Actual height=" << mActualHeight;
    std::cerr << " y=" << y << " height=" << height << std::endl;
    height = mActualHeight - y;
  }
  safeBmpCpy(pBmpDest, 0, 0, &mFullSrc, x, y, width, height);
  mReadWidth = width;
  mReadHeight = height;
  return true;
}


bool my_jpeg_write(std::string& newFileName, BYTE *pFullBitmap, int64_t width, int64_t height, int quality, std::string* perrMsg)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile = NULL;
  JSAMPROW * pjSampleRows = 0;
  
  outfile = fopen(newFileName.c_str(), "wb");
  if (outfile == NULL)
  {
    if (perrMsg) *perrMsg = std::strerror(errno);
    return false;
  }
  /* this is a pointer to one row of image data */
  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = my_jpeg_error_exit;

  try
  {
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    /* Setting the parameters of the output file here */
    cinfo.image_width = width;  
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    /* default compression parameters, we shouldn't be worried about these */
    jpeg_set_defaults(&cinfo);
    /* set the quality */
    jpeg_set_quality(&cinfo, quality, TRUE);
    /* Now do the compression .. */
    jpeg_start_compress(&cinfo, TRUE);
    /* like reading a file, this time write one row at a time */
    pjSampleRows = new JSAMPROW[height];
    for (int64_t y = 0; y < height; y++)
    {
      pjSampleRows[y] = &pFullBitmap[width * y * 3];
    }
    int64_t totalScanlines=0;
    while ((int64_t)cinfo.next_scanline < height) 
    {
      totalScanlines += jpeg_write_scanlines(&cinfo, &pjSampleRows[totalScanlines], height-totalScanlines);
    }
    /* similar to read file, clean up after we're done compressing */
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(outfile);
    delete[] pjSampleRows;
    pjSampleRows = 0;
  }
  catch (jpeg_error_mgr* pJerr) 
  {
    if (perrMsg)
    {
      perrMsg->assign("Error compressing '");
      perrMsg->append(newFileName);
      perrMsg->append("': ");
      perrMsg->append(pJerr->jpeg_message_table[pJerr->msg_code]);
    }
    jpeg_destroy_compress(&cinfo);
    if (outfile) fclose(outfile);
    if (pjSampleRows)
    {
      delete[] pjSampleRows;
      pjSampleRows = 0;
    }
    return false;
  } 
  catch (std::bad_alloc &e) 
  {
    std::cerr << "Fatal Error: Not enough memory to decompress '";
    std::cerr << newFileName;
    std::cerr << "' into memory!";
    exit(1);
  }
  return true;
}


bool my_jpeg_compress(BYTE** ptpCompressedBitmap, BYTE *pFullBitmap, int64_t width, int64_t height, int quality, std::string* perrMsg, unsigned long *pOutSize)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW * pjSampleRows = 0;
  
  if (pOutSize == NULL || ptpCompressedBitmap == NULL) return false;

  *pOutSize = 0;
  *ptpCompressedBitmap = NULL;

  /* this is a pointer to one row of image data */
  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = my_jpeg_error_exit;

  try
  {
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, ptpCompressedBitmap, pOutSize);

    /* Setting the parameters of the output file here */
    cinfo.image_width = width;  
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    /* default compression parameters, we shouldn't be worried about these */
    jpeg_set_defaults(&cinfo);
    /* set the quality */
    jpeg_set_quality(&cinfo, quality, TRUE);
    /* Now do the compression .. */
    jpeg_start_compress(&cinfo, TRUE);
    /* like reading a file, this time write one row at a time */
    pjSampleRows = new JSAMPROW[height];
    for (int64_t y = 0; y < height; y++)
    {
      pjSampleRows[y] = &pFullBitmap[width * y * 3];
    }
    int64_t totalScanlines=0;
    while (cinfo.next_scanline < height) 
    {
      totalScanlines += jpeg_write_scanlines(&cinfo, &pjSampleRows[totalScanlines], height-totalScanlines);
    }
    /* similar to read file, clean up after we're done compressing */
    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    delete[] pjSampleRows;
    pjSampleRows = 0;
  }
  catch (jpeg_error_mgr* pJerr) 
  {
    if (perrMsg)
    {
      perrMsg->assign("Error compressing buffer ");
      perrMsg->append(pJerr->jpeg_message_table[pJerr->msg_code]);
    }
    jpeg_destroy_compress(&cinfo);
    if (pjSampleRows)
    {
      delete[] pjSampleRows;
      pjSampleRows = 0;
    }
    if (*ptpCompressedBitmap)
    {
      free(*ptpCompressedBitmap);
      *ptpCompressedBitmap = NULL;
    }
    return false;
  } 
  catch (std::bad_alloc &e) 
  {
    std::cerr << "Fatal Error: Not enough memory to compress bitmap to jpeg image!" << std::endl;
    exit(1);
  }
  return true;
}


void my_jpeg_free(BYTE** ptpCompressedBitmap)
{
  if (ptpCompressedBitmap != NULL && *ptpCompressedBitmap != NULL)
  {
    free(*ptpCompressedBitmap);
    *ptpCompressedBitmap = NULL;
  }
}
