#include "aton.h"
#include "describe.h"
#include "get_nanoseconds.h"
#include "string_view.h"

#include <string.h>

using dsy::string_view;
using namespace std::literals;
using std::cout;
using std::endl;

// comparison tests of the various string to number functions

class timeit
{
public:
        timeit(std::string_view desc, uint32_t cnt)
                :m_desc(desc), m_cnt(cnt)
        {
                m_start = get_nanoseconds();
        }

        ~timeit()
        {
                m_end = get_nanoseconds();
                uint64_t total = m_end - m_start;
                std::cout << m_desc << ", cnt: " << m_cnt << ", total ns: " << total << ", each: " << (total / m_cnt) << std::endl; 
        }

        std::string m_desc;
        uint32_t m_cnt;
        uint64_t m_start = 0;
        uint64_t m_end = 0;
};

template<typename NUMB>
void test_aton(uint32_t cnt, const string_view& src, NUMB expected)
{
        timeit ti("aton"sv, cnt);
        for (uint32_t i = 0; i < cnt; i++)
        {
                NUMB val = aton<NUMB>(src);
                if (val != expected)
                {
                        cout << "aton<" << describe<NUMB>() << "> for " << src << " failed" << endl;
                        break;
                } 
        }
}

void test_strtoul(uint32_t cnt, const string_view& src, uint64_t expected)
{
        timeit ti("strtoul"sv, cnt);
        for (uint32_t i = 0; i < cnt; i++)
        {
                char buff[50];
                strncpy(buff, src.data(), src.length());
                uint64_t val = strtoul(buff, nullptr, 10);
                if (val != expected)
                {
                        cout << "strtoul" << " for " << src << " failed" << endl;
                        break;
                } 
        }
}

void test_uint64(uint32_t cnt)
{
        // aton<uint64_t>, strtoul,  
        test_aton(cnt, "1234567890"sv, 1234567890UL);
        test_strtoul(cnt, "1234567890"sv, 1234567890UL);

}

int main (int argc, char** argv)
{
        uint64_t cnt = 1000;

        for (uint32_t i = 1; i < argc; i++)
        {
                string_view arg(argv[i]);
                string_view key = arg.split('=');

                if (key == "--cnt"sv)
                        arg.aton(cnt);

        } 

        test_uint64(cnt);
}
