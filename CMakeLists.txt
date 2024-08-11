cmake_minimum_required(VERSION 3.1...3.29)

project(
  EHttp
  VERSION 1.0
  LANGUAGES CXX)

add_subdirectory(extern/string_view)
add_subdirectory(extern/misc)
add_subdirectory(extern/servers)
add_subdirectory(src)
add_subdirectory(tests)
