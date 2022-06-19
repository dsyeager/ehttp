#pragma once

#include <stdint.h>

template <typename OBJ> const char* describe();
template<> const char* describe<uint8_t>() { return "uint8_t"; }
template<> const char* describe<uint16_t>() { return "uint16_t"; }
template<> const char* describe<uint32_t>() { return "uint32_t"; }
template<> const char* describe<uint64_t>() { return "uint64_t"; }

template<> const char* describe<int8_t>() { return "int8_t"; }
template<> const char* describe<int16_t>() { return "int16_t"; }
template<> const char* describe<int32_t>() { return "int32_t"; }
template<> const char* describe<int64_t>() { return "int64_t"; }

template<> const char* describe<float>() { return "float"; }
template<> const char* describe<double>() { return "double"; }
