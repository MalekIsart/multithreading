cmake_minimum_required(VERSION 3.0)

project(ThreadPool)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Crée des variables avec les fichiers à compiler
set(SOURCES
    ThreadPool.cpp
    main.cpp
    )
    
set(HEADERS
    ThreadPool.h
    )

add_executable(ThreadPool ${SOURCES} ${HEADERS})