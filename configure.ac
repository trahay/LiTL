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
AC_CHECK_LIB([rt], [clock_gettime])
AC_CHECK_LIB([m], [ceil])

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

AC_SUBST(libdir)
AC_SUBST(exec_prefix)
AC_SUBST(prefix)
AC_SUBST(includedir)

AC_OUTPUT([Makefile evnt.pc \
            src/Makefile \
            tests/Makefile ])
