set(TARGET_NAME v4l2_player)

set(LINK_LIB)
set(LINK_LIB ${LINK_LIB} yuv)
set(LINK_LIB ${LINK_LIB} SDL2::SDL2)

set(COMMON_SRCS)
set(COMMON_SRCS ${COMMON_SRCS} "../common/v4l2_utils.cc")
set(COMMON_SRCS ${COMMON_SRCS} "../common/capture_device_mmap.cc")
set(COMMON_SRCS ${COMMON_SRCS} "../common/sdl2_video_renderer.cc")
aux_source_directory(. SRCS)

add_executable(${TARGET_NAME} ${SRCS} ${COMMON_SRCS})
target_link_libraries(${TARGET_NAME} ${LINK_LIB})
