extern "C"
{
#ifdef HAVE_GLIBC_STRERROR_R
#include <cstring>
#include <cerrno>

void check(char c) {}

int main () {
  char buffer[1024];
  /* This will not compile if strerror_r does not return a char* */
  check(strerror_r(EACCES, buffer, sizeof(buffer))[0]);
  return 0;
}
#endif
#ifdef HAVE_POSIX_STRERROR_R
#include <cstring>
#include <cerrno>

/* float, because a pointer can't be implicitly cast to float */
void check(float f) {}

int main () {
  char buffer[1024];
  /* This will not compile if strerror_r does not return an int */
  check(strerror_r(EACCES, buffer, sizeof(buffer)));
  return 0;
}
#endif
#ifdef HAVE_ZIPOPENNEWFILEINZIP64
#include <cstring>

#ifdef OLY_USE_MZ_VER2
#include <mz.h>
#include <mz_compat.h>
#include <mz_os.h>
#else
#include "zip.h"
#endif

int main() {
  zipFile file;
  zip_fileinfo zinfo;
  memset(&file, 0, sizeof(file));
  memset(&zinfo, 0, sizeof(zinfo));
  return zipOpenNewFileInZip64(file, "test.txt", &zinfo, 
    NULL, 0, NULL, 0, NULL, 0, 0, 1); 
}
#endif
#ifdef HAVE_ZIPOPENNEWFILEINZIP_64
#include <cstring>

#ifdef OLY_USE_MZ_VER2
#include <mz.h>
#include <mz_compat.h>
#include <mz_os.h>
#else
#include "zip.h"
#endif

int main() {
  zipFile file;
  zip_fileinfo zinfo;
  memset(&file, 0, sizeof(file));
  memset(&zinfo, 0, sizeof(zinfo));
  return zipOpenNewFileInZip_64(file, "test.txt", &zinfo, 
    NULL, 0, NULL, 0, NULL, 0, 0, 1); 
}
#endif
#ifdef HAVE_ZIPOPENNEWFILEINZIP4_64
#include <cstring>

#ifdef OLY_USE_MZ_VER2
#include <mz.h>
#include <mz_compat.h>
#include <mz_os.h>
#else
#include "zip.h"
#endif

int main() {
  zipFile file;
  zip_fileinfo zinfo;
  memset(&file, 0, sizeof(file));
  memset(&zinfo, 0, sizeof(zinfo));
  return zipOpenNewFileInZip4_64(file, "test.txt", &zinfo, 
    NULL, 0, NULL, 0, NULL, 0, 0, 
    0, 0, 0, 0, NULL, 0, 0, 0, 1); 
}
#endif
#ifdef HAVE_ZIPCLOSEFILEINZIP64
#include "zip.h"
#include <cstring>

int main() {
  zipFile file;
  memset(&file, 0, sizeof(file));
  return zipCloseFileInZip64(file); 
}
#endif
#ifdef HAVE_ZIPCLOSE_64
#include <cstring>

#ifdef OLY_USE_MZ_VER2
#include <mz.h>
#include <mz_compat.h>
#include <mz_os.h>
#else
#include "zip.h"
#endif

int main() {
  zipFile file;
  memset(&file, 0, sizeof(file));
  return zipClose_64(zipfile, NULL);
}
#endif
#ifdef HAVE_ZIPCLOSE64
#include <cstring>

#ifdef OLY_USE_MZ_VER2
#include <mz.h>
#include <mz_compat.h>
#include <mz_os.h>
#else
#include "zip.h"
#endif

int main() {
  zipFile file;
  memset(&file, 0, sizeof(file));
  return zipClose64(zipfile, NULL);
}
#endif
}
