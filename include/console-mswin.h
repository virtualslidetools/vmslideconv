#ifndef __CONSOLE_MSWIN_FILE_H
#define __CONSOLE_MSWIN_FILE_H

#include <string>
#include <cstring>
#include <direct.h>
void retractCursor();

bool platform_mkdir(std::string name, std::string* perror);

#define SEPARATOR '\\'

#define
