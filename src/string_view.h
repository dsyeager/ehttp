#pragma once

#include "aton.h"

#include <string_view>

namespace dsy
{

class string_view : public std::string_view
{
public:
        string_view() :std::string_view() {}

        string_view(const std::string_view& sv) :std::string_view(sv) {}

        string_view(const char* val, size_t len) :std::string_view(val, len) {}

        string_view(const char* val) :std::string_view(val) {}

        string_view(const std::string& val) :std::string_view(val) {}

        // wishy washy about split vs remove_until
        
        string_view split(char delim)
        {
                size_t pos = this->find(delim);
                string_view ret = this->substr(0, pos);
                if (pos >= this->length() - 1) // -1 to make zero based
                        *this = string_view();
                else
                        *this = this->substr(pos + 1);
                return ret;
        }

        template<typename NUMB> bool aton(NUMB &val, std::string_view *rem = nullptr)
        {
                std::string_view rem_local;
                if (!rem)
                        rem = &rem_local;
                val = ::aton<NUMB>(*this, rem);
                return rem->empty();
        }

};

};
