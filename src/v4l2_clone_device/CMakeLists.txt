set(TARGET_NAME v4l2_clone_device)

set(LINK_LIB)
set(LINK_LIB ${LINK_LIB} yuv)
set(LINK_LIB ${LINK_LIB} SDL2::SDL2)
set(LINK_LIB ${LINK_LIB} PkgConfig::libdrm)

set(COMMON_SRCS)
set(COMMON_SRCS ${COMMON_SRCS} "../common/v4l2_utils.cc")
set(COMMON_SRCS ${COMMON_SRCS} "../common/capture_device_mmap.cc")
set(COMMON_SRCS ${COMMON_SRCS} "../common/sdl2_video_renderer.cc")
set(COMMON_SRCS ${COMMON_SRCS} "../common/output_device_mmap.cc")
set(COMMON_SRCS ${COMMON_SRCS} "../common/drm_prime_dmabuf.cc")
set(COMMON_SRCS ${COMMON_SRCS} "../common/output_device_dmabuf.cc")
aux_source_directory(. SRCS)

add_executable(${TARGET_NAME} ${SRCS} ${COMMON_SRCS})
target_link_libraries(${TARGET_NAME} ${LINK_LIB})
