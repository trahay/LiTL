cmake_minimum_required (VERSION 3.1)

project(LiTL
  VERSION 0.2.0
  LANGUAGES C
  DESCRIPTION "LiTL is a tracing library"
  HOMEPAGE_URL https://github.com/trahay/LiTL
)

# include CMake modules
include(CheckLibraryExists)


option(ENABLE_GETTID
	"Use syscall(SYS_gettid) to get the thread ID instead of pthread_self(). This however costs a system call for each trace entry"
	OFF)

option(FORCE_32BITS
	"Build LiTL in 32-bit mode"
	OFF)

CHECK_LIBRARY_EXISTS(rt clock_gettime "" librt_exist)
if (NOT librt_exist)
    message(FATAL_ERROR "librt was not found.")
endif()

CHECK_LIBRARY_EXISTS(m ceil "" libm_exist)
if (NOT libm_exist)
    message(FATAL_ERROR "libm was not found.")
endif()

set (INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig")

# Subdirectory
add_subdirectory (src)
add_subdirectory (utils)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/litl.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/litl.pc)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/litl.pc
	       DESTINATION "${INSTALL_PKGCONFIG_DIR}")
