cmake_minimum_required(VERSION 3.18)

configure_file (
  ${CMAKE_CURRENT_SOURCE_DIR}/litl_config.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/litl_config.h
  )

add_library(litl SHARED
  litl_types.h
  litl_tools.h
  litl_tools.c
  litl_timer.h
  litl_timer.c
  litl_write.h
  litl_write.c
  litl_read.h
  litl_read.c
  litl_merge.h
  litl_merge.c
  litl_split.h
  litl_split.c
  )


target_link_libraries(litl
  PRIVATE
    pthread
)

target_include_directories(litl
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}
)

SET(LITL_HEADERS
  ${CMAKE_CURRENT_BINARY_DIR}/litl_config.h
  litl_types.h
  litl_tools.h
  litl_timer.h
  litl_write.h
  litl_read.h
  litl_merge.h
  litl_split.h
  )

set_target_properties(litl PROPERTIES PUBLIC_HEADER "${LITL_HEADERS}")

include(GNUInstallDirs)

install(
    TARGETS litl
)
