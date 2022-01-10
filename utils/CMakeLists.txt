cmake_minimum_required(VERSION 3.1)

add_executable(litl_print litl_print.c  )
add_executable(litl_merge litl_merge.c  )
add_executable(litl_split litl_split.c  )

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR}/include
  ${CMAKE_SOURCE_DIR}/src
  )

target_link_libraries( litl_print  PRIVATE   litl  )
target_link_libraries( litl_merge  PRIVATE   litl  )
target_link_libraries( litl_split  PRIVATE   litl  )

install(
    TARGETS litl_print litl_merge litl_split
)
