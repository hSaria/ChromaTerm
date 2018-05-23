## This program is protected under the GNU GPL (See COPYING)

AM_INIT_AUTOMAKE
AC_INIT
AC_CONFIG_SRCDIR([defs.h])
AC_CONFIG_HEADERS([config.h])

AC_SUBST(mybindir)
if [ test $bindir != "\${exec_prefix}/bin" ]; then
	mybindir="$bindir"
else
	mybindir="."
fi

AC_PROG_CC
AC_PROG_MAKE_SET


### Check for PCRE2. If not found, use legacy PCRE
AC_SUBST(menu, 1)

AC_SEARCH_LIBS(initscr, ncurses, , [
  menu=0
	AC_MSG_WARN([ncurses library was not found; interactive CT menu will not be available])])
AC_SEARCH_LIBS(post_menu, menu, , [
  menu=0
	AC_MSG_WARN([menu library was not found; interactive CT menu will not be available])])

if [ test $menu == 1]; then
  AC_CHECK_HEADERS([curses.h menu.h], , [AC_MSG_WARN([curses.h and/or menu.h libraries were not found; interactive CT menu will not be available])])
fi
###

### Check for PCRE2. If not found, use legacy PCRE
AC_SUBST(pcreversion, 2)

AC_SEARCH_LIBS(pcre2_compile_8, pcre2-8, , [
  pcreversion=1;
	AC_MSG_WARN([PCRE2 library check failed (is PCRE2 development installed and linked?). Trying legacy PCRE (not as fast)])])

if [ test $pcreversion == 2]; then  # Only run this check if the AC_SEARCH_LIBS above didn't fail
  AC_CHECK_HEADERS([pcre2.h], , [
    pcreversion=1
	  AC_MSG_WARN([PCRE2 header check failed (is PCRE2 development installed and linked?). Trying legacy PCRE (not as fast)])
  ], [#define PCRE2_CODE_UNIT_WIDTH 8])
fi

if [ test $pcreversion == 1 ]; then
  AC_SEARCH_LIBS(pcre_compile, pcre, , [AC_MSG_ERROR([PCRE library check failed (is PCRE development installed and linked?)])])
	AC_CHECK_HEADERS([pcre.h], , [AC_MSG_ERROR([PCRE header check failed (is PCRE development installed and linked?)])])
fi
###

AC_CONFIG_FILES([Makefile])

AC_OUTPUT
