#include "describe.h"
#include "get_nanoseconds.h"
#include "ntoa.h"

#include <array>
#include <charconv>
#include <iostream>
#include <limits>
#include <string>
#include <string_view>

using namespace std;
using namespace std::literals;

static uint32_t s_failures = 0;
static uint32_t s_successes = 0;

template<typename NUMB>
void test(NUMB val, string_view expected, uint8_t base = 10)
{
        string buff;
        ntoa(val, buff, base);
        if (buff == expected)
        {
                //cout << "success: " << expected << " == " << buff << endl;
                s_successes++;
        }
        else
        {
                cout << "failure: " << expected << " == '" << buff << "'" << endl;
                s_failures++;
                exit(1);
        }
}

static char s_buff[100];

template<typename NUMB> void test_10(NUMB val)
{
        sprintf(s_buff, "%d", val);
        test(val, s_buff, 10);
}

template<typename NUMB> void test_l10(NUMB val)
{
        sprintf(s_buff, "%ld", val);
        test(val, s_buff, 10);
}

template<typename NUMB> void test_16(NUMB val)
{
        sprintf(s_buff, "%x", val);
        test(val, s_buff, 16);
}

template<typename NUMB> void test_l16(NUMB val)
{
        sprintf(s_buff, "%lx", val);
        test(val, s_buff, 16);
}

int main (int argc, char** argv)
{
        test<uint8_t>(0, "0"sv);
        test<uint8_t>(1, "1"sv);
        test<uint8_t>(9, "9"sv);
        test<uint8_t>(25, "25"sv);

        uint64_t n1 = get_nanoseconds();

        for (uint8_t v = 0; v < 200; v++) 
        {
                test_10(v);
                test_16(v);
        }

        uint64_t n2 = get_nanoseconds();
        for (uint16_t v = 0; v < 10000; v++) 
        {
                test_10(v);
                test_16(v);
        }

        uint32_t cnt = 1000000;
        uint64_t n3 = get_nanoseconds();

        for (uint32_t v = 0; v < cnt; v++) 
        {
                test_10(v);
                test_16(v);
        }
        uint64_t n4 = get_nanoseconds();

        for (uint64_t v = 0; v < cnt; v++) 
        {
                test_l10(v);
                test_l16(v);
        }
        uint64_t n5 = get_nanoseconds();

        string buff;

        for (uint64_t v = 0; v < cnt; v++) 
        {
                buff.clear();
                ntoa(v, buff, 10);
        }

        uint64_t n6 = get_nanoseconds();

        for (uint64_t v = 0; v < cnt; v++) 
        {
                buff = std::to_string(v);
        }

        uint64_t n7 = get_nanoseconds();
        std::array<char, 25> str;

        for (uint64_t v = 0; v < cnt; v++) 
        {
                std::to_chars(str.data(), str.data() + str.size(), v, 10);
        }

        uint64_t n8 = get_nanoseconds();


        cout << "successes: " << s_successes << ", failures: " << s_failures << endl;
        cout << "ntoa<uint64_t> each: " << ((n6-n5)/cnt) << endl;
        cout << "std::to_string each: " << ((n7-n6)/cnt) << endl;
        cout << "std::to_char each: " << ((n8-n7)/cnt) << endl;
}
