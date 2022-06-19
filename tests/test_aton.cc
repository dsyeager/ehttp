#include "aton.h"
#include "describe.h"
#include "ntoa.h"
#include "string_view.h"

#include <iostream>
#include <typeinfo>

using namespace std;
using namespace std::literals;

template<typename NUMB>
void test_max_plus_one(uint64_t &successes, uint64_t &failures)
{
        NUMB mval = std::numeric_limits<NUMB>::max();
        string buff;
        ntoa(mval, buff);

        int bk = buff.back() - '0';
        buff.pop_back();

        buff += '0';
        NUMB expected = aton<NUMB>(buff);
        buff.pop_back();

        if (bk == 9)
        {
                int bk2 = buff.back() - '0';
                buff.pop_back();
                bk2++;
                ntoa(bk2, buff);
        }
        ntoa(++bk, buff);

        std::string_view rem;
        NUMB val = aton<NUMB>(buff, &rem);

        if (val != expected)
        {
                cout << "test_max_plus_one<" << describe<NUMB>() << "> failed, val: " << val << ", expected: " << expected << endl;
                failures++;
        }
        else
        {
                cout << "test_max_plus_one<" << describe<NUMB>() << "> succeeded" << endl;
                successes++;
        }
}

template<typename NUMB>
void test_max(uint64_t &successes, uint64_t &failures)
{
        NUMB mval = std::numeric_limits<NUMB>::max();
        string buff;
        ntoa(mval, buff);
        NUMB val = aton<NUMB>(buff);
        if (val != mval)
        {
                cout << "test_max<" << describe<NUMB>() << "> failed" << endl;
                failures++;
        }
        else
        {
                cout << "test_max<" << describe<NUMB>() << "> succeeded" << endl;
                successes++;
        }
}

template<typename NUMB>
void test(uint64_t &successes, uint64_t &failures)
{
        string buff;
        size_t cnt = 0;
        uint64_t succ = 0;
        uint64_t fail = 0;
        NUMB i = 0;
        NUMB tests = std::numeric_limits<NUMB>::max();
        if (std::numeric_limits<NUMB>::max() > 10000000) tests = 10000000;

        for (i = 0; i < tests; i++)
        {
                buff.clear();
                ntoa(i, buff);
                NUMB val = aton<NUMB>(buff);
                NUMB val2 = 0;
                dsy::string_view sv(buff);
                sv.aton(val2);

                if (val != i || val2 != i)
                {
                        cout << i << " != " << val << endl;
                        fail++;
                }
                else
                {
                        succ++;
                }
                cnt++;
        }
        cout << "tested 0-" << cnt << " for " << describe<NUMB>() << ", successes: " << succ << ", failures: " << fail << endl; 


        test_max<NUMB>(successes, failures);
        test_max_plus_one<NUMB>(successes, failures);

        successes += succ;
        failures += fail;
}

template<typename NUMB>
void test_hex(uint64_t &successes, uint64_t &failures)
{
        string buff;
        size_t cnt = 0;
        uint64_t succ = 0;
        uint64_t fail = 0;
        NUMB i = 0;
        NUMB tests = std::numeric_limits<NUMB>::max();
        if (std::numeric_limits<NUMB>::max() > 10000000) tests = 10000000;

        for (i = 0; i < tests; i++)
        {
                buff.clear();

        }
        cout << "tested 0-" << cnt << " for " << describe<NUMB>() << ", successes: " << succ << ", failures: " << fail << endl; 


        test_max<NUMB>(successes, failures);
        test_max_plus_one<NUMB>(successes, failures);

        successes += succ;
        failures += fail;
}

int main (int argc, char** argv)
{
        uint64_t successes = 0;
        uint64_t failures = 0;
        test<uint8_t>(successes, failures);
        test<uint16_t>(successes, failures);
        test<uint32_t>(successes, failures);
        test<uint64_t>(successes, failures);



        cout << "Successes: " << successes << ", Failures: " << failures << endl;
}
