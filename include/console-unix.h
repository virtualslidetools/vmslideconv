#ifndef __CONSOLE_UNIX_FILE_H
#define __CONSOLE_UNIX_FILE_H

#include <iostream>
#include <cerrno>
#include <cstring>
#include <sys/stat.h>

void retractCursor();

bool platform_mkdir(std::string name, std::string* perror);

#endif
