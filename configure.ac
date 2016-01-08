AC_PREREQ(2.59)
AC_INIT([groonga-edit-distance], 0.0.0, [naoya@createfield.com])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([functions/edit_distance.c])
AM_CONFIG_HEADER([config.h])

AM_INIT_AUTOMAKE(foreign)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_PROG_CC

AC_PROG_LIBTOOL

if test "$GCC" = "yes"; then
  CFLAGS="$CFLAGS -Wall -Wextra"
  CFLAGS="$CFLAGS -Wmissing-declarations -Wmissing-prototypes"
fi

AC_ARG_ENABLE(debug,
  [AS_HELP_STRING([--enable-debug],
                  [use debug flags (default=no)])],
  [sample_debug="$enableval"],
  [sample_debug="no"])
if test "x$sample_debug" != "xno"; then
  if test "$CLANG" = "yes"; then
    CFLAGS="$CFLAGS -O0 -g"
  elif test "$GCC" = "yes"; then
    CFLAGS="$CFLAGS -O0 -g3"
  fi
fi

GROONGA_REQUIRED_VERSION=4.0.3
PKG_CHECK_MODULES([GROONGA], [groonga >= ${GROONGA_REQUIRED_VERSION}])

_PKG_CONFIG(GROONGA_PLUGINS_DIR, [variable=pluginsdir],    [groonga])
_PKG_CONFIG(GROONGA,             [variable=groonga],       [groonga])

GROONGA_PLUGINS_DIR="${pkg_cv_GROONGA_PLUGINS_DIR}"
GROONGA="${pkg_cv_GROONGA}"
GROONGA_HTTPD="${pkg_cv_GROONGA_HTTPD}"

AC_SUBST(GROONGA_PLUGINS_DIR)
AC_SUBST(GROONGA)
AC_SUBST(GROONGA_HTTPD)

functions_pluginsdir="\${GROONGA_PLUGINS_DIR}/functions"
AC_SUBST(functions_pluginsdir)

AC_CONFIG_FILES([
  Makefile
  functions/Makefile
  test/Makefile
])

AC_OUTPUT

echo "$PACKAGE_NAME $PACKAGE_VERSION configuration:"
echo "-----------------------"
echo "  Compiler:              ${CC}"
echo "  CFLAGS:                ${CFLAGS}"
echo "  CXXFLAGS:              ${CXXFLAGS}"
echo "  Libraries:             ${LIBS}"
echo
echo "groonga-edit-distance"
echo "  CFLAGS:                ${GROONGA_CFLAGS}"
echo "  Libraries:             ${GROONGA_LIBS}"
echo "  install directory:     ${functions_pluginsdir}"
