# Add executable
add_executable(extract_keyframes extract_keyframes.cpp)

# Include directories
target_include_directories(extract_keyframes PRIVATE
        ${AVCODEC_INCLUDE_DIRS}
        ${AVFORMAT_INCLUDE_DIRS}
        ${AVUTIL_INCLUDE_DIRS}
        ${SWRESAMPLE_INCLUDE_DIRS}
        ${SWSCALE_INCLUDE_DIRS}
        ${AVDEVICE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(extract_keyframes PRIVATE
        ${AVCODEC_LIBRARIES}
        ${AVFORMAT_LIBRARIES}
        ${AVUTIL_LIBRARIES}
        ${SWRESAMPLE_LIBRARIES}
        ${SWSCALE_LIBRARIES}
        ${AVDEVICE_LIBRARIES}
)