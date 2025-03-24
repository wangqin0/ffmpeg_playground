cmake_minimum_required(VERSION 3.14)

project(ffmpeg_playground
        LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
pkg_check_modules(SWSCALE REQUIRED libswscale)
pkg_check_modules(AVDEVICE REQUIRED libavdevice)

# Include directories for FFmpeg headers
include_directories(
        ${AVCODEC_INCLUDE_DIRS}
        ${AVFORMAT_INCLUDE_DIRS}
        ${AVUTIL_INCLUDE_DIRS}
        ${SWRESAMPLE_INCLUDE_DIRS}
        ${SWSCALE_INCLUDE_DIRS}
        ${AVDEVICE_INCLUDE_DIRS}
)

# Add library directories to linker search paths
link_directories(
        ${AVCODEC_LIBRARY_DIRS}
        ${AVFORMAT_LIBRARY_DIRS}
        ${AVUTIL_LIBRARY_DIRS}
        ${SWRESAMPLE_LIBRARY_DIRS}
        ${SWSCALE_LIBRARY_DIRS}
        ${AVDEVICE_LIBRARY_DIRS}
)

# Add compile options if needed
add_compile_options(
        ${AVCODEC_CFLAGS_OTHER}
        ${AVFORMAT_CFLAGS_OTHER}
        ${AVUTIL_CFLAGS_OTHER}
        ${SWRESAMPLE_CFLAGS_OTHER}
        ${SWSCALE_CFLAGS_OTHER}
        ${AVDEVICE_CFLAGS_OTHER}
)

# Add subdirectories
add_subdirectory(src)