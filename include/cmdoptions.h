#ifndef __CMDOPTIONS_FILE_H
#define __CMDOPTIONS_FILE_H
#include <iostream>

class CmdOptions
{
protected:
  ostringstream col1, col2;
  std::string insertText;
public:
  CmdOptions & operator << (CmdOptions &out, const char* str)
  {
    insertText = str;
  }
}

#endif
