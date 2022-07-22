#ifdef HAVE_GLIBC_STRERROR_R
#include <string.h>
#include <errno.h>

void check(char c) {}

int
main () {
  char buffer[1024];
  /* This will not compile if strerror_r does not return a char* */
  check(strerror_r(EACCES, buffer, sizeof(buffer))[0]);
  return 0;
}
#endif
#ifdef HAVE_POSIX_STRERROR_R
#include <string.h>
#include <errno.h>

/* float, because a pointer can't be implicitly cast to float */
void check(float f) {}

int
main () {
  char buffer[1024];
  /* This will not compile if strerror_r does not return an int */
  check(strerror_r(EACCES, buffer, sizeof(buffer)));
  return 0;
}
#endif
