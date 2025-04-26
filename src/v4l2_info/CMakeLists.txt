set(TARGET_NAME v4l2_info)

aux_source_directory(. SRCS)

add_executable(${TARGET_NAME} ${SRCS})
target_link_libraries(${TARGET_NAME} ${LINK_LIB})
