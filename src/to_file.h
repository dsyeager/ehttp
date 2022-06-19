#pragma once

#include "ntoa.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

void to_file(std::string_view val, const char *fpath)
{
        std::string tmp(fpath);
        tmp += ".tmp";
        ntoa(getpid(), tmp);
        std::ofstream out(tmp);
        out << val;
        out.close();
        rename(tmp.c_str(), fpath);
}
