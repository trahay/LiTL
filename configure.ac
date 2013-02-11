# Copyright © Télécom SudParis.
# See COPYING in top-level directory.

#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.68])
AC_INIT([evnt], [0.1], [todo-evnt@fusionforge.int-evry.fr])
AM_INIT_AUTOMAKE([1.10 tar-pax color-tests])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_LIBTOOL
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([m], [ceil])
AC_CHECK_LIB([rt], [clock_gettime])

# Check the OS type
os_type=`uname -s`
if [ test "x$os_type" = "xLinux" ]; then
# Linux
  AC_MSG_WARN([OS: Linux])
  CFLAGS="$CFLAGS -DHAVE_LINUX"
  DYNLIB_EXT=".so"
  LD_PRELOAD_NAME="LD_PRELOAD"
  LD_LIBRARY_PATH_NAME="LD_LIBRARY_PATH"
elif [ test "x$os_type" = "xDarwin" ]; then
# Mac OS
  AC_MSG_WARN([OS: Mac OS])
  CFLAGS="$CFLAGS -DHAVE_DARWIN"
  DYNLIB_EXT=".dylib"
  LD_PRELOAD_NAME="DYLD_INSERT_LIBRARIES"
  LD_LIBRARY_PATH_NAME="DYLD_LIBRARY_PATH"
else
# unknown OS, let's try Linux
  AC_MSG_WARN([unknown OS: falling back to Linux])
  CFLAGS="$CFLAGS -DHAVE_LINUX"
  DYNLIB_EXT=".so"
  LD_PRELOAD_NAME="LD_PRELOAD"
  LD_LIBRARY_PATH_NAME="LD_LIBRARY_PATH"
fi

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h stdio.h string.h math.h time.h sys/stat.h unistd.h sys/syscall.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UINT8_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([atexit])

AC_ARG_ENABLE([gettid],
    AC_HELP_STRING([--enable-gettid],
        [Use syscall(SYS_gettid) to get the thread ID instead of pthread_self().
          This however costs a system call for each trace entry]))
if test "x$enable_gettid" = xyes; then
  AC_DEFINE([USE_GETTID], [1], [Use gettid])
fi

# On many multicore CPUs, clock cycles are not synchronized. Therefore, we prefer to use clock_gettime() instead of 
# time stamp counter (clock cycles).
AC_MSG_CHECKING([for clock_gettime(CLOCK_MONOTONIC_RAW)])
AC_COMPILE_IFELSE(
  AC_LANG_PROGRAM([[
    #include <time.h>
    struct timespec tp;
    ]],
    [[ clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
    ]]),
  AC_DEFINE(CLOCK_GETTIME_AVAIL, [1], [Use clock_gettime])
  AC_MSG_RESULT([yes]),
  AC_MSG_RESULT([no])
)

AC_SUBST(libdir)
AC_SUBST(exec_prefix)
AC_SUBST(prefix)
AC_SUBST(includedir)

AC_OUTPUT([Makefile evnt.pc \
            src/Makefile \
            tests/Makefile ])
