cmake_minimum_required(VERSION 3.12)
project(JPEG_to_AVI_master)

set(CMAKE_CXX_STANDARD 14)

include_directories(include)

add_executable(JPEG_to_AVI_master
        include/jpeg.h
        include/mp3.h
        include/read_file.h
        include/riff.h
        src/jpeg.cpp
        src/mjpeg.cpp
        src/mp3.cpp
        src/read_file.cpp
        src/riff.cpp
        include/utils.h
        src/utils.c)
