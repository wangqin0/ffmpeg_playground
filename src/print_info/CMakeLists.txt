# Add executable
add_executable(print_info print_info.cpp)

target_include_directories(print_info PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(print_info PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(print_info PRIVATE ${FFMPEG_LIBRARIES})
