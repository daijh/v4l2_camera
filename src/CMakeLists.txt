include_directories("${CMAKE_CURRENT_SOURCE_DIR}/common")

add_subdirectory(sdl2_renderer)

add_subdirectory(v4l2_info)
add_subdirectory(v4l2_player)
add_subdirectory(v4l2_clone_device)
