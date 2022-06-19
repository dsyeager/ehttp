#pragma once

#include <stdint.h>
#include <time.h>

uint64_t get_nanoseconds()
{       
        timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        uint64_t ret = ts.tv_nsec;
        ret += ts.tv_sec * 1000000000;
        return ret;
}

