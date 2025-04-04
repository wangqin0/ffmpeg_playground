cmake_minimum_required(VERSION 3.14)

project(ffmpeg_playground
        LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find FFmpeg packages via vcpkg
find_package(FFMPEG REQUIRED COMPONENTS AVCODEC AVFORMAT AVUTIL SWRESAMPLE SWSCALE AVDEVICE)

add_subdirectory(src)
