AC_INIT([libzbs], [0.1])
AC_CONFIG_AUX_DIR([ac-aux])
AC_CONFIG_MACRO_DIR([ac-m4])

AM_PROG_AR
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AM_SILENT_RULES([yes])

CXXFLAGS="-std=c++11 $CXXFLAGS"

AC_PROG_LIBTOOL
AC_PROG_CXX

AC_ARG_ENABLE([assert],
	AS_HELP_STRING([--disable-assert], [Disable zbs library assertion checks]),
	[],
	AC_DEFINE([ZBS_ENABLE_ASSERT], [1], [Enables assertion checks within the zbs library]))

AC_CONFIG_HEADERS([config.h public_config.h])
AC_CONFIG_FILES([Makefile src/Makefile test/Makefile])

AC_OUTPUT
