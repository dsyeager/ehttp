#include "ntoa.h"

#include <limits.h>

#include <iostream>
#include <limits>

using namespace std;

template<typename NUMB> void show()
{
        cout << "\tdigits: " << numeric_limits<NUMB>::digits << endl;
        cout << "\tmax: " << numeric_limits<NUMB>::max() << endl;
        uint32_t chars = ((sizeof(NUMB)*CHAR_BIT - 1)*28/93 + 3);
        cout << "\tchars: " << chars << endl;

        string buff;
        ntoa(numeric_limits<NUMB>::max(), buff);
        cout << "\tprinted size: " << buff.length() << endl;
}

int main(int argc, char** argv)
{
        cout << "int32_t" << endl;
        show<int32_t>();

        cout << "int64_t" << endl;
        show<int64_t>();

        cout << "uint32_t" << endl;
        show<uint32_t>();

        cout << "uint64_t" << endl;
        show<uint64_t>();
}
