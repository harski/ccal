AC_PREREQ([2.68])
AC_INIT([ccal], [0.1], [hartitu@gmail.com])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

AC_CONFIG_SRCDIR([src/cal.c])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC_C99

# Checks for libraries.
AC_CHECK_LIB([ncursesw], [get_wch], , AC_MSG_ERROR([Cannot find ncursesw]))

AX_WITH_CURSES
if test "x$ax_cv_header_ncursesw_curses_h" != xyes && "xx$ax_cv_header_ncursesw_h" != xyes; then
    AC_MSG_ERROR([Cannot find a header for ncursesw])
fi

# Checks for header files.
AC_CHECK_HEADERS([locale.h stddef.h stdlib.h string.h unistd.h wchar.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MKTIME
AC_FUNC_REALLOC
AC_CHECK_FUNCS([localtime_r memmove setlocale])

AC_CONFIG_FILES([Makefile
                 src/Makefile])
AC_OUTPUT
