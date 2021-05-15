cmake_minimum_required(VERSION 3.0)

project(AsyncLoader)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Crée des variables avec les fichiers à compiler
set(SOURCES
    ResourceLoader.cpp
    AsyncLoader.cpp
    )
    
set(HEADERS
    ResourceLoader.h
    )

add_executable(AsyncLoader ${SOURCES} ${HEADERS})