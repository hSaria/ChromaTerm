## This program is protected under the GNU GPL (See COPYING)

AM_INIT_AUTOMAKE
AC_INIT
AC_CONFIG_SRCDIR([defs.h])
AC_CONFIG_HEADERS([config.h])

AC_SUBST(mybindir)
if [ test $bindir != "\${exec_prefix}/bin" ]
then
	mybindir="$bindir"
else
	mybindir="."
fi

AC_PROG_CC
AC_PROG_MAKE_SET

# Add libraries to LIBS variable if required
AC_SEARCH_LIBS(forkpty, util)
AC_SEARCH_LIBS(pcre2_compile_8, pcre2-8, , [AC_MSG_ERROR([pcre2 header check failed. Is pcre2 development installed and linked? (pcre2, not pcre)])])

# Checks for header files.
AC_CHECK_HEADERS([pty.h util.h])

AC_CHECK_FILES([/dev/ptmx])
AC_CONFIG_FILES([Makefile])

AC_OUTPUT
