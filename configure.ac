dnl Copyright 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>
dnl Process this file with autoconf to produce a configure script.

AC_INIT(Makefile.am)

ac_gip_build_date=$(date -u +"%D %T UTC")
ac_gip_build_user=$(whoami)
ac_gip_build_host=$(uname -n)

AC_PREREQ(2.50)
dnl AC_CHECK_COMPILERS()
AC_PROG_CXX()
dnl AC_ENABLE_SHARED(yes)
dnl AC_ENABLE_STATIC(yes)
AC_LIBTOOL_WIN32_DLL()
AC_PROG_LIBTOOL()

AC_CANONICAL_SYSTEM()
dnl perform program name transformation
AC_ARG_PROGRAM()

AM_INIT_AUTOMAKE(gip-framework, 0.1.2)



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
  CPPFLAGS="-I${ac_base_include}${CPPFLAGS:+ }${CPPFLAGS}"
fi
if test "x$ac_base_lib" != "x"; then
  BASE_LIBADD="${ac_base_lib}/libbase.la"
  AC_SUBST(BASE_LIBADD)
fi



dnl Add VERSION resource to the dynamic library on Windows platforms (i.e. cygwin or mingw targets)
case "$ac_cv_target" in
*-*-cygwin|*-*-mingw32)
  dnl The -Wl option is used to circumvent libtool
  GIP_ADDITIONAL_LIBADD="${GIP_ADDITIONAL_LIBADD}${GIP_ADDITIONAL_LIBADD:+ }"'-Wl,winrcver.o'
  GIP_ADDITIONAL_DEPENDENCIES="${GIP_ADDITIONAL_DEPENDENCIES}${GIP_ADDITIONAL_DEPENDENCIES:+ }"'winrcver.o'
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
  CPPFLAGS='-I${top_srcdir}'"${CPPFLAGS:+ }${CPPFLAGS}"
else
  dnl We are building in a separate directory tree
  CPPFLAGS='-I${top_builddir} -I${top_srcdir}'"${CPPFLAGS:+ }${CPPFLAGS}"
fi

AC_SUBST(GIP_ADDITIONAL_LIBADD)
AC_SUBST(GIP_ADDITIONAL_LDFLAGS)
AC_SUBST(GIP_ADDITIONAL_DEPENDENCIES)

dnl Store the symbols
ac_gip_header=gip/configuration.h
echo "updating configuration..."
mkdir -p gip
mv $ac_gip_config $ac_gip_header

AC_OUTPUT(Makefile gip/Makefile gip/docs/Makefile gip/docs/en/Makefile gip/transformation/Makefile gip/analysis/Makefile gip/Makefile gip/draw/Makefile gip/io/Makefile gip/media/Makefile)
