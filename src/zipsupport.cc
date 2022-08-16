#include "zipsupport.h"
#include <iostream>
#include <algorithm>
#include <cerrno>
#include <ctime>
#include <cstring>
#include <cerrno>
#include <cstdio>
#include <sys/stat.h>

#if defined(WIN32) || defined(WIN64)
#include <sys/types.h>
#else
#include <unistd.h>
#endif

static uint32_t unix2dostime(time_t *time);

std::string getErrnoStrErr()
{
  std::string errStr;
  char errBuf[512];
  char * errPtr = NULL;

  errBuf[0] = 0;
  (void) errPtr;

  #ifdef HAVE_GLIBC_STRERROR_R
  errPtr = strerror_r(errno, errBuf, sizeof(errBuf));
  if (errPtr)
  {
    errStr = errPtr;
  }
  #elif HAVE_POSIX_STRERROR_R
  if (strerror_r(errno, errBuf, sizeof(errBuf)) == 0)
  {
    errStr = errBuf;
  }
  #elif HAVE_STRERROR_S
  (void) strerror_s(errBuf, sizeof(errBuf), errno);
  errStr = errBuf;
  #elif HAVE_STRERROR
  errPtr = strerror(errno);
  if (errPtr)
  {
    errStr = errPtr;
  }
  #else
  errStr = "";
  #endif

  return errStr;
}


int ZipFile::openArchive(std::string filename, int append)
{
  int status = 0;
  bool isExistingFile = false;

  mOutputFile = filename;
  mDirNames.clear();

  #if defined(WIN32) || defined(WIN64)
  struct _stat sb;
  status = _stat(filename.c_str(), &sb);
  if (status != -1)
  {
    isExistingFile = (sb.st_mode & _S_IFREG) != 0 ? true : false;
    if (isExistingFile == false)
    {
      std::cout << "Error: Path '" << filename << "' exists and is not a regular file." << std::endl;
      return -1;
    }
  }
  #else
  struct stat sb;
  status = stat(filename.c_str(), &sb);
  if (status != -1)
  {
    isExistingFile = (sb.st_mode & S_IFREG) != 0 ? true : false;
    if (isExistingFile == false)
    {
      std::cout << "Error: Path '" << filename << "' exists and is not a regular file." << std::endl;
      return -1;
    }
  }
  #endif
  if (isExistingFile)
  {
    remove(filename.c_str());
  }

  mZipArchive = zipOpen64(mOutputFile.c_str(), append);
  if (mZipArchive == NULL)
  {
    mErrMsg = getErrnoStrErr();
    status = -1;
  }
  else
  {
    status = 0;
  }
  return status;
}


void ZipFile::setCompression(int method, int flags)
{
  mCompressMethod = method;
  mCompressFlags = flags;
}


int ZipFile::flushArchive()
{
  if (mZipArchive == NULL) return ZIP_OK;
  //int status = zipFlush(mZipArchive, NULL);
  return ZIP_OK;
}


int ZipFile::closeArchive()
{
  if (mZipArchive == NULL) return 0;
  
  #ifdef HAVE_ZIPCLOSE_64
  int status = zipClose_64(mZipArchive, NULL);
  #elif defined(HAVE_ZIPCLOSE64)
  int status = zipClose64(mZipArchive, NULL);
  #else
  int status = zipClose(mZipArchive, NULL);
  #endif

  if (status != ZIP_OK)
  {
    mErrMsg = getErrnoStrErr();
  }    
  mZipArchive = NULL;
  mDirNames.clear();
  return status;
}


int ZipFile::addFile(std::string filename, BYTE* buff, int64_t size)
{
  zip_fileinfo zinfo;
  time_t currentTime;

  if (mZipArchive == NULL || buff == NULL) return 0;

  memset(&zinfo, 0, sizeof(zinfo));
  time(&currentTime);
  
  zinfo.mz_dos_date = unix2dostime(&currentTime);
  zinfo.internal_fa = 0644;
  zinfo.external_fa = 0644 << 16L;

  #ifdef HAVE_ZIPOPENNEWFILEINZIP_64
  int status = zipOpenNewFileInZip_64(mZipArchive, filename.c_str(), &zinfo, 
    NULL, 0, NULL, 0, NULL, mCompressMethod, mCompressFlags, 1); 
  #elif HAVE_ZIPOPENNEWFILEINZIP64
  int status = zipOpenNewFileInZip64(mZipArchive, filename.c_str(), &zinfo, 
    NULL, 0, NULL, 0, NULL, mCompressMethod, mCompressFlags, 1); 
  #elif defined(HAVE_ZIPOPENNEWFILEINZIP4_64)
  int status = zipOpenNewFileInZip4_64(mZipArchive, filename.c_str(), &zinfo, 
    NULL, 0, NULL, 0, NULL, mCompressMethod, mCompressFlags, 
    0, 0, 0, 0, NULL, 0, OLY_ZIP_VERSION_MADE_BY, 0, 1); 
  #else
  int status = zipOpenNewFileInZip(mZipArchive, filename.c_str(), &zinfo, 
    NULL, 0, NULL, 0, mCompressMethod, mCompressFlags);
  #endif

  if (status == ZIP_OK)
  {
    status = zipWriteInFileInZip(mZipArchive, size == 0 ? (BYTE*) "" : buff, (unsigned) size);
    if (status == ZIP_OK)
    {
      #ifdef HAVE_ZIPCLOSEFILEINZIP64
      status = zipCloseFileInZip64(mZipArchive);
      #else
      status = zipCloseFileInZip(mZipArchive);
      #endif
      if (status != ZIP_OK)
      {
        mErrMsg = getErrnoStrErr();
      }
    }
    else
    {
      mErrMsg = getErrnoStrErr();
      #ifdef HAVE_ZIPCLOSEFILEINZIP64
      status = zipCloseFileInZip64(mZipArchive);
      #else
      status = zipCloseFileInZip(mZipArchive);
      #endif
    }
  }
  else
  {
    mErrMsg = getErrnoStrErr();
  }
  return status;
}


int ZipFile::addDir(std::string name)
{
  zip_fileinfo zinfo;
  time_t currentTime;
  
  if (mZipArchive == NULL) return 0;
  
  memset(&zinfo, 0, sizeof(zinfo));
  time(&currentTime);
  zinfo.mz_dos_date = unix2dostime(&currentTime);
  zinfo.internal_fa = 0755;
  zinfo.external_fa = 040755 << 16L;

  std::string nameWithSlash=name;
  std::size_t lastSlashIndex = name.find_last_of(mZipPathSeparator);
  int status = 0;
  if (lastSlashIndex == std::string::npos || lastSlashIndex < name.length())
  {
    nameWithSlash.append(1, mZipPathSeparator);
  }
  if (std::find(mDirNames.begin(), mDirNames.end(), nameWithSlash) != mDirNames.end())
  {
    return ZIP_OK;
  }

  #ifdef HAVE_ZIPOPENNEWFILEINZIP_64
  status = zipOpenNewFileInZip_64(mZipArchive, nameWithSlash.c_str(), 
    &zinfo, NULL, 0, NULL, 0, NULL, OLY_DEF_COMPRESS_METHOD, 0, 1);
  #elif HAVE_ZIPOPENNEWFILEINZIP64
  status = zipOpenNewFileInZip64(mZipArchive, nameWithSlash.c_str(), 
    &zinfo, NULL, 0, NULL, 0, NULL, OLY_DEF_COMPRESS_METHOD, 0, 1);
  #elif defined(HAVE_ZIPOPENNEWFILEINZIP4_64)
  status = zipOpenNewFileInZip4_64(mZipArchive, nameWithSlash.c_str(), 
    &zinfo, NULL, 0, NULL, 0, NULL, OLY_DEF_COMPRESS_METHOD, 0, 
    0, 0, 0, 0, NULL, 0, OLY_ZIP_VERSION_MADE_BY, 0, 1); 
  #else
  status = zipOpenNewFileInZip(mZipArchive, nameWithSlash.c_str(), 
    &zinfo, NULL, 0, NULL, 0, OLY_DEF_COMPRESS_METHOD, 0);
  #endif

  if (status == ZIP_OK)
  {
    zipWriteInFileInZip(mZipArchive, "", 0);
    mDirNames.push_back(nameWithSlash);
    #ifdef HAVE_ZIPCLOSEFILEINZIP64
    status = zipCloseFileInZip64(mZipArchive);
    #else
    status = zipCloseFileInZip(mZipArchive);
    #endif
  }
  if (status != ZIP_OK)
  {
    mErrMsg = getErrnoStrErr();
  }
  return status;
}


uint32_t unix2dostime(time_t *time)
{
  struct tm ltime;
  struct tm* pltime = &ltime;
  if (time==NULL) return 0;
  
  #ifdef HAVE_LOCALTIME_S
  localtime_s(&ltime, time);
  #elif HAVE_LOCALTIME_R
  localtime_r(time, &ltime);
  #else
  pltime = localtime(time);
  #endif

  int year = pltime->tm_year - 80;
  if (year < 0)
    year = 0;

  return (year << 25
	  | (pltime->tm_mon + 1) << 21
	  | pltime->tm_mday << 16
	  | pltime->tm_hour << 11
	  | pltime->tm_min << 5
	  | pltime->tm_sec >> 1);
}


const char* ZipFile::getErrorMsg()
{
  return mErrMsg.c_str();
}

