#pragma once

#include "ntoa.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>

bool from_file(std::string &val, const char *fpath)
{
        std::ifstream ifs(fpath);
        if (!ifs.is_open())
        {
                std::cerr << "Failed to open " << fpath << std::endl;
                return false;
        }

        val.assign((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
        return true;
}
