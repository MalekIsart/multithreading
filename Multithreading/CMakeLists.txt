cmake_minimum_required(VERSION 3.0)

set(CMAKE_SUPPRESS_REGENERATION true)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

project(Multithreading)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#add_subdirectory(AsyncLoader)
add_subdirectory(ThreadPool)