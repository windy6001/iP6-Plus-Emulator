#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT(iP6,4.9-Beta-4, [BUG-REPORT-ADDRESS])
AC_CONFIG_SRCDIR([src/config.h.in])
AC_CONFIG_HEADERS([src/config.h])
AM_INIT_AUTOMAKE(1.15   foreign)
 

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB

# checks for build enviroment
AC_CANONICAL_BUILD 

# Checks for libraries.



# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([dirent.h OS.h fcntl.h limits.h locale.h malloc.h memory.h stddef.h stdlib.h string.h strings.h sys/ioctl.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([opendir atexit floor getcwd gettimeofday memset pow setlocale strchr strstr strtol])

#AC_CHECK_HEADERS([X11/extensions/scrnsaver.h], [], [],
#          [[#include <X11/Xlib.h>
#          ]])




AC_ARG_ENABLE(mitshm,
[  --enable-mitshm          turn on MITSHM [[default=no]]],
[\
case "${enableval}" in
 yes) enable_mitshm=yes ;;
 no)  enable_mitshm=no ;;
 *)   AC_MSG_ERROR(bad value for --enable-mitshm) ;;
esac],
enable_mitshm=no)
if test x"${enable_mitshm}" = x"yes"; then
#  AC_DEFINE(MITSHM, 1, [Define to 1 if you want to MITSHM])
   CFLAGS="$CFLAGS -DMITSHM"
fi

case ${build_vendor} in
	apple)
	  LIBS="-framework OpenAL $LIBS"
	;;
	*)
	  LIBS="-lopenal $LIBS"
	;;
esac


AC_SUBST( CFLAGS)
CFLAGS="$CFLAGS -O0 -DX11 -DUNIX -I/usr/X11/include -I/usr/X11R6/include -I/opt/X11/include -I/usr/local/include"

AC_SUBST( CXXFLAGS)
CXXFLAGS="$CFLAGS"

AC_SUBST( LIBS)
LIBS="-L/usr/X11/lib -L/opt/X11/lib -L/usr/X11R6/lib -L/usr/local/lib -lpthread \
  -lXaw -lXt -lXmu -lXext -lX11 -lm `libpng-config --static --libs`\
  $LIBS"


AC_ARG_ENABLE(sound, 
[   --enable-sound   Enable sound       [default=yes]],
     ,enable_sound=yes)
if test x$enable_sound = xyes; then
   CFLAGS="-DSOUND $CFLAGS"
fi

AC_ARG_ENABLE(debug,
[   --enable-debug  Enable debugging mode [default=yes]],
     ,enable_debug=yes)
if test x$enable_debug = xyes; then
   CFLAGS="-DDEBUG $CFLAGS"
fi


AC_CONFIG_FILES([ \
                 Makefile \
                 doc/Makefile \
                 src/Makefile \
		 src/compatible_rom/Makefile \
                 src/fmgen/Makefile \
                 src/sysdep/Makefile \
                 src/unix/Makefile \
                 src/win/Makefile \
                 Win_Project/Makefile \
                 ])
AC_OUTPUT

