#pragma once

#include <limits.h>

#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

// base 10 conversion and only base 10 for now
// could weave base 16 (hex) into it by allowing user to pass base in 
// but will prolly be better optimized to add an aton_hex function that is specialized for hex
// is an assumption, a test would be good


template<typename NUMB, uint8_t base = 10> NUMB aton(const std::string_view &src, std::string_view *remainder = nullptr)
{
        if (src.empty())
                return 0;

        // https://stackoverflow.com/questions/43787672/the-max-number-of-digits-in-an-int-based-on-number-of-bits
        // is not exact, is 1-2 chars bigger than actual so -1 to account for nullptr

        // TODO: would checking the index be cheaper than checking max_val?
        //       if so we could start checking max_vals when index is >= max_chars - 1
        //static uint32_t max_chars = (sizeof(NUMB) * CHAR_BIT - 1) * 28 / 93 + 3 - 1;

        std::function<uint8_t(char)> get_val = [](char c)
        {
                if (c >= '0' && c <= '9')
                        return uint8_t(c - '0');
                if (c >= 'A' && c <= 'Z')
                        return uint8_t(10 + (c - 'A'));
                if (c >= 'a' && c <= 'z')
                        return uint8_t(10 + (c - 'a'));
                return uint8_t(std::numeric_limits<uint8_t>::max());
        };

        auto cur = src.begin();
        auto last = src.end();

        uint8_t cv = get_val(*cur);

        if (cv > base)
        {
                std::cerr << "non-numeric string: " << src << std::endl;
                if (remainder)
                        *remainder = std::string_view(cur, last - cur);
                return 0;
        }

        NUMB ret = *cur++ - '0';

        uint32_t chars = 1;

        static NUMB max_val = std::numeric_limits<NUMB>::max();
        static NUMB max_divided_by_base = max_val / base;

        while (cur != last)
        {
                char c = *cur;
                uint8_t cv = get_val(*cur);
                if (cv > base)
                {
                        std::cerr << "non-numeric string: " << src << std::endl;
                        if (remainder)
                                *remainder = std::string_view(cur, last - cur);
                        return ret;
                }
                
                if (ret > max_divided_by_base)
                {
                        //std::cerr << "overrun (" << base << "'s)" << std::endl;
                        if (remainder)
                                *remainder = std::string_view(cur, last - cur);
                        return ret;
                }
                ret *= base;

                if (cv > max_val - ret)
                {
                        std::cerr << "overrun (1's), src: " << src << std::endl;

                        if (remainder)
                                *remainder = std::string_view(cur, last - cur);
                        return ret;
                }
                ret += cv;
                cur++;
        }
        return ret;
}

