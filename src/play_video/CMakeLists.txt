# Add executable
add_executable(play_video play_video.cpp)

target_include_directories(play_video PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_directories(play_video PRIVATE ${FFMPEG_LIBRARY_DIRS})
target_link_libraries(play_video PRIVATE ${FFMPEG_LIBRARIES})
