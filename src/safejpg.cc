#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>

extern "C" {
#include "jpeglib.h"
#include "jerror.h"
}
#include "safebmp.h"
#include <sstream>


void static safeJpgErrorExit(j_common_ptr cinfo)
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


int safeJpgWrite(safeBmp* pBmpSrc, const std::string& newFileName, int quality, std::string* perrMsg)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  FILE * outfile = NULL;
  JSAMPROW * pjSampleRows = 0;
  
  BYTE *pFullBitmap = pBmpSrc->data;
  int64_t cols = pBmpSrc->cols;
  int64_t rows = pBmpSrc->rows;

  #ifdef HAVE_FOPEN_S
  fopen_s(&outfile, newFileName.c_str(), "rb");
  #else
  outfile = fopen(newFileName.c_str(), "wb");
  #endif
  if (outfile == NULL)
  {
    #ifdef HAVE_STRERROR_S
    if (perrMsg) 
    {
      char tempStr[512];
      tempStr[0] = 0;
      strerror_s(tempStr, sizeof(tempStr), errno);
      *perrMsg = tempStr;
    }
    #else
    if (perrMsg) *perrMsg = std::strerror(errno);
    #endif
    return 1;
  }
  
  memset(&cinfo, 0, sizeof(cinfo));
  memset(&jerr, 0, sizeof(jerr));

  /* this is a pointer to one row of image data */
  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = safeJpgErrorExit;

  try
  {
    jpeg_create_compress(&cinfo);
    jpeg_stdio_dest(&cinfo, outfile);

    /* Setting the parameters of the output file here */
    cinfo.image_width = (JDIMENSION) cols;  
    cinfo.image_height = (JDIMENSION) rows;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    /* default compression parameters, we shouldn't be worried about these */
    jpeg_set_defaults(&cinfo);
    /* set the quality */
    jpeg_set_quality(&cinfo, quality, TRUE);
    /* Now do the compression .. */
    jpeg_start_compress(&cinfo, TRUE);
    /* like reading a file, this time write one row at a time */
    pjSampleRows = new JSAMPROW[rows];
    for (int64_t y = 0; y < rows; y++)
    {
      pjSampleRows[y] = &pFullBitmap[cols * y * 3];
    }
    int64_t totalScanlines=0;
    while ((int64_t)cinfo.next_scanline < rows && totalScanlines < rows) 
    {
      totalScanlines += jpeg_write_scanlines(&cinfo, &pjSampleRows[totalScanlines], (JDIMENSION) (rows-totalScanlines));
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
    return 2;
  } 
  catch (std::bad_alloc &e) 
  {
    (void) e;
    std::cerr << "Fatal Error: Not enough memory to decompress '";
    std::cerr << newFileName;
    std::cerr << "' into memory!";
    exit(1);
  }
  return 0;
}


int safeJpgCompress(safeBmp* pBmpSrc, BYTE** ptpCompressedBitmap, int quality, std::string* perrMsg, unsigned long *pOutSize)
{
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW * pjSampleRows = 0;
  BYTE *pFullBitmap;
  int64_t cols, rows;

  pFullBitmap = pBmpSrc->data;
  cols = pBmpSrc->cols;
  rows = pBmpSrc->rows;

  if (pOutSize == NULL || ptpCompressedBitmap == NULL) return false;

  *pOutSize = 0;
  *ptpCompressedBitmap = NULL;

  memset(&cinfo, 0, sizeof(cinfo));
  memset(&jerr, 0, sizeof(jerr));

  /* this is a pointer to one row of image data */
  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = safeJpgErrorExit;

  try
  {
    jpeg_create_compress(&cinfo);
    jpeg_mem_dest(&cinfo, ptpCompressedBitmap, pOutSize);

    /* Setting the parameters of the output file here */
    cinfo.image_width = (JDIMENSION) cols;  
    cinfo.image_height = (JDIMENSION) rows;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    /* default compression parameters, we shouldn't be worried about these */
    jpeg_set_defaults(&cinfo);
    /* set the quality */
    jpeg_set_quality(&cinfo, quality, TRUE);
    /* Now do the compression .. */
    jpeg_start_compress(&cinfo, TRUE);
    /* like reading a file, this time write one row at a time */
    pjSampleRows = new JSAMPROW[rows];
    for (int64_t y = 0; y < rows; y++)
    {
      pjSampleRows[y] = &pFullBitmap[cols * y * 3];
    }
    int64_t totalScanlines=0;
    while ((int64_t)cinfo.next_scanline < rows && totalScanlines < rows) 
    {
      totalScanlines += jpeg_write_scanlines(&cinfo, &pjSampleRows[totalScanlines], (JDIMENSION) (rows-totalScanlines));
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
    return 1;
  } 
  catch (std::bad_alloc &e) 
  {
    (void) e;
    std::cerr << "Fatal Error: Not enough memory to compress bitmap to jpeg image!" << std::endl;
    exit(1);
  }
  return 0;
}


void safeJpgCompressFree(BYTE** ptpCompressedBitmap)
{
  if (ptpCompressedBitmap != NULL && *ptpCompressedBitmap != NULL)
  {
    free(*ptpCompressedBitmap);
    *ptpCompressedBitmap = NULL;
  }
}


int safeJpgRead(safeBmp *pBmpDest, const std::string& fileName, int bkgColor, std::string* perrStr)
{
  FILE *infile = NULL;
  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW* pjSampleRows = 0; 
  int64_t cols = 0;
  int64_t rows = 0;
  std::stringstream errMsg;
  
  *perrStr = "";
  memset(&cinfo, 0, sizeof(cinfo));
  memset(&jerr, 0, sizeof(jerr));

  #ifdef HAVE_FOPEN_S
  fopen_s(&infile, fileName.c_str(), "rb");
  #else
  infile = fopen(fileName.c_str(), "rb");
  #endif
  if (infile == 0) 
  {
    #ifdef HAVE_STRERROR_S
    if (perrStr) 
    {
      char tempStr[512];
      tempStr[0] = 0;
      strerror_s(tempStr, sizeof(tempStr), errno);
      *perrStr = tempStr;
    }
    #else
    if (perrStr) *perrStr = std::strerror(errno);
    #endif
    return 1;
  }

  cinfo.err = jpeg_std_error(&jerr);
  jerr.error_exit = safeJpgErrorExit;

  try 
  {
    jpeg_create_decompress(&cinfo);
    jpeg_stdio_src(&cinfo, infile);
    jpeg_read_header(&cinfo, TRUE);
    if (cinfo.num_components != 3) 
    {
      errMsg << "Warning: '" << fileName << "' has unsupported samples per pixel: " << cinfo.num_components << std::endl;
      *perrStr = errMsg.str();
      return 2;
    }
    jpeg_start_decompress(&cinfo);
    cols = cinfo.output_width;
    rows = cinfo.output_height;
    safeBmpAlloc2(pBmpDest, cols, rows);
    safeBmpByteSet(pBmpDest, bkgColor);
    int64_t destLineCols = cols * 3;
    int64_t totalScanlines=0;
    pjSampleRows = new JSAMPROW[rows];
    for (int64_t y = 0; y < rows; y++)
    {
      pjSampleRows[y] = pBmpDest->data + (y * destLineCols);
    }
    while (cinfo.output_scanline < cinfo.output_height) 
    {
      totalScanlines += jpeg_read_scanlines(&cinfo, &pjSampleRows[totalScanlines], (JDIMENSION) (rows-totalScanlines));
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
    errMsg << "Error decompressing '" << fileName << "': ";
    errMsg << pJerr->jpeg_message_table[pJerr->msg_code];
    jpeg_destroy_decompress(&cinfo);
    if (infile) fclose(infile);
    if (pjSampleRows)
    {
      delete[] pjSampleRows;
    }
    *perrStr = errMsg.str();
    return 3;
  } 
  catch (std::bad_alloc &e) 
  {
    (void) e;
    jpeg_destroy_decompress(&cinfo);
    if (infile) fclose(infile);
    if (pjSampleRows)
    {
      delete[] pjSampleRows;
    }
    errMsg << "Insufficient memory to decompress '" << fileName;
    errMsg << "' into memory";
    *perrStr = errMsg.str();
    exit(1);
  }
  return 0;
}
