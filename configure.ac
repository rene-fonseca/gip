dnl ***************************************************************************
dnl    Generic Image Processing (GIP) Framework
dnl    A framework for developing image processing applications
dnl
dnl    See COPYRIGHT.txt for details.
dnl
dnl    This framework is distributed in the hope that it will be useful,
dnl    but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
dnl
dnl    For the licensing terms refer to the file 'LICENSE'.
dnl ***************************************************************************

dnl Process this file with autoconf to produce a configure script

AC_INIT(Makefile.am)
AC_CONFIG_SRCDIR(gip/features.h)

ac_gip_build_date=$(date --iso-8601=seconds)
ac_gip_build_user=$(whoami)
ac_gip_build_host=$(uname -n)

AC_PREREQ(2.52)
AC_COPYRIGHT([Copyright (C) 2001-2002 by Rene Moeller Fonseca)
AC_REVISION($Revision$)

AC_CANONICAL_SYSTEM()

dnl perform program name transformation
AC_ARG_PROGRAM()

AM_INIT_AUTOMAKE(gip-framework, 0.1.2)

dnl AC_CHECK_COMPILERS()
dnl AC_EXEEXT() dnl TAG: required before 2.50
dnl AC_PROG_MAKE_SET()
AC_PROG_CXX()
AC_PROG_CPP()
AC_CHECK_TOOL(WINDRES, windres)
dnl AC_ENABLE_SHARED(yes)
dnl AC_ENABLE_STATIC(yes)
AC_LIBTOOL_WIN32_DLL()
AC_PROG_LIBTOOL()



ac_gip_config=configuration.tmp
rm -f $ac_gip_config

dnl AC_GIP_DEFINE(symbol name, value)
AC_DEFUN(AC_GIP_DEFINE, [dnl
ifelse([$2], , ac_gip_temp="_DK_SDU_MIP__GIP__$1", ac_gip_temp="_DK_SDU_MIP__GIP__$1 $2")
cat >> $ac_gip_config << EOF
[#define] ${ac_gip_temp}
EOF
])

AC_ARG_WITH(base,
  [  --with-base=DIR         where the Base Framework is installed],
  [
     ac_base_include="$withval/include"
     ac_base_lib="$withval/lib"
  ])

AC_ARG_WITH(base-include,
  [  --with-base-include=DIR location of The Base Framework includes],
  [
    ac_base_include="$withval"
  ])

AC_ARG_WITH(base-lib,
  [  --with-base-lib=DIR     location of The Base Framework libraries],
  [
    ac_base_lib="$withval"
  ])

if test "x$ac_base_include" != "x"; then
  BASE_INCLUDES_ADD="-I${ac_base_include}${BASE_INCLUDES_ADD:+ }${BASE_INCLUDES_ADD}"
fi
if test "x$ac_base_lib" != "x"; then
  BASE_LDFLAGS_ADD="-L${ac_base_lib}"
fi



dnl Add VERSION resource to the dynamic library on Windows platforms (i.e. cygwin or mingw targets)
case "$ac_cv_target" in
*-*-cygwin|*-*-mingw32)
  dnl The -Wl option is used to circumvent libtool
  GIP_LIBADD_ADD="${GIP_LIBADD_ADD}${GIP_LIBADD_ADD:+ }"'-Wl,win32rc.o'
  GIP_DEPENDENCIES_ADD="${GIP_DEPENDENCIES_ADD}${GIP_DEPENDENCIES_ADD:+ }"'win32rc.o'
  ;;
esac



dnl Define some describing symbols
AC_GIP_DEFINE(CONFIGURE, [\"]$ac_configure_args[\"])
AC_GIP_DEFINE(BUILD_DATE, [\"]$ac_gip_build_date[\"])
AC_GIP_DEFINE(BUILD_USER, [\"]$ac_gip_build_user[\"])
AC_GIP_DEFINE(BUILD_HOST, [\"]$ac_gip_build_host[\"])
AC_GIP_DEFINE(BUILD_SYSTEM, [\"]$ac_cv_build[\"])
AC_GIP_DEFINE(TARGET_SYSTEM, [\"]$ac_cv_target[\"])

dnl Tell the compiler where it should look for the generated and static framework headers
if (test -f gip/features.h) then
  dnl We are building directly in the source tree
  GIP_INCLUDES_ADD='-I${top_srcdir}'"${GIP_INCLUDES_ADD:+ }${GIP_INCLUDES_ADD}"
else
  dnl We are building in a separate directory tree
  GIP_INCLUDES_ADD='-I${top_builddir} -I${top_srcdir}'"${GIP_INCLUDES_ADD:+ }${GIP_INCLUDES_ADD}"
fi

AC_SUBST(BASE_INCLUDES_ADD)
AC_SUBST(BASE_LDFLAGS_ADD)
AC_SUBST(GIP_INCLUDES_ADD)
AC_SUBST(GIP_LIBADD_ADD)
AC_SUBST(GIP_LDFLAGS_ADD)
AC_SUBST(GIP_DEPENDENCIES_ADD)

dnl Store the symbols
ac_gip_header=gip/configuration.h
echo "updating configuration..."
mkdir -p gip
mv $ac_gip_config $ac_gip_header

dnl Generate make files
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(gip/Makefile)
AC_CONFIG_FILES(gip/docs/Makefile)
AC_CONFIG_FILES(gip/docs/en/Makefile)
AC_CONFIG_FILES(gip/operation/Makefile)
AC_CONFIG_FILES(gip/transformation/Makefile)
AC_CONFIG_FILES(gip/analysis/Makefile)
AC_CONFIG_FILES(gip/canvas/Makefile)
AC_CONFIG_FILES(gip/io/Makefile)
AC_CONFIG_FILES(gip/media/Makefile)
AC_CONFIG_FILES(gip/video/Makefile)

AC_OUTPUT()
