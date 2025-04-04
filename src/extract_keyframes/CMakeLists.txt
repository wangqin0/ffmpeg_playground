# Add executable
add_executable(extract_keyframes extract_keyframes.cpp)

target_include_directories(extract_keyframes PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(extract_keyframes PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(extract_keyframes PRIVATE ${FFMPEG_LIBRARIES})
