#include "ntoa.h"
#include "aton.h"

#include <limits.h>

#include <iostream>
#include <limits>
#include <string>

using namespace std;

template<typename NUMB> void test()
{
        cout << "\tmin-max: " << numeric_limits<NUMB>::min() << '-' << numeric_limits<NUMB>::max() << endl;
        uint32_t chars = ((sizeof(NUMB)*CHAR_BIT - 1)*28/93 + 3);
        cout << "\tchars: " << chars << endl;

        string buff;
        ntoa(numeric_limits<NUMB>::max() / 2, buff);
        string_view rem;

        NUMB val = aton<NUMB>(buff, &rem);

        cout << "buff: " << buff << ", val: " << val << endl;


}

int main(int argc, char** argv)
{
        test<int32_t>();
}
