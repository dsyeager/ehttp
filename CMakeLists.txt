cmake_minimum_required(VERSION 3.1...3.29)

project(
  ehttp
  VERSION 1.0
  LANGUAGES CXX)

# how to avoid duplicate definitions when calling add_subdirectory
# string_view and servers have misc as a submodule
# so when I add them as subdirs here the test names become duplicates

#add_subdirectory(extern/string_view)
#add_subdirectory(extern/misc)
#add_subdirectory(extern/servers)
add_subdirectory(src)
add_subdirectory(tests)
