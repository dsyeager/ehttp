#pragma once

#include <algorithm>
#include <charconv>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>


/*
    A generic number -> string utility function.

    Alternatives tested are std::to_string
                            std::to_chars_result to_chars( char* first, char* last,
                                value, int base = 10 );


*/

static inline char get_char(uint8_t val)
{
        if (val < 10)
                return char('0' + val);
        if (val < 36) // up to base 36
                return 'a' + (val - 10);
        // could go higher just need to find which chars to use
        return std::numeric_limits<uint8_t>::max();
}

static inline std::string_view add_zero(std::string &dest)
{
        dest.push_back('0');
        return std::string_view(dest.c_str() + dest.length() - 1, 1);
}

template <typename NUMB> std::string_view ntoa(NUMB val, std::string &dest, uint8_t base = 10)
{
        char str[25];
        auto [ptr, ec] = std::to_chars(str, str + 25, val, base);
        std::string_view ret(str, ptr - str);
        dest += ret;
        return ret;
/*
        if (!val) return add_zero(dest);

        const size_t initial_length = dest.length();

        // was reserving but if string has to grow it will grow at most once per conversion

        while (val)
        {
                uint8_t ones = val % base;
                val /= base;
                dest.push_back(get_char(ones));
        }

        if (val < 0) dest.push_back('-');

        std::reverse(dest.begin() + initial_length, dest.end());
        return std::string_view(dest.c_str() + initial_length, dest.length() - initial_length);
*/
}

// TODO: need a specialization for double
