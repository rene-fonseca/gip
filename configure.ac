dnl Copyright 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>
dnl Process this file with autoconf to produce a configure script.

AC_INIT(Makefile.am)

ac_gip_build_date=$(date -u +"%D %T UTC")
ac_gip_build_host=$(uname -n)

AC_PREREQ(2.5)
dnl AC_CHECK_COMPILERS()
AC_PROG_CXX()
dnl AC_ENABLE_SHARED(yes)
dnl AC_ENABLE_STATIC(no)
AC_LIBTOOL_WIN32_DLL()
AC_PROG_LIBTOOL()

AC_CANONICAL_SYSTEM()
dnl perform program name transformation
AC_ARG_PROGRAM()

AM_INIT_AUTOMAKE(gip, 0.1)

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
  LIBS="${LIBS}${LIBS:+ }${ac_base_lib}/libbase.la"
fi

AC_GIP_DEFINE(CONFIGURE, [\"]$ac_configure_args[\"])
AC_GIP_DEFINE(BUILD_DATE, [\"]$ac_gip_build_date[\"])
AC_GIP_DEFINE(BUILD_HOST, [\"]$ac_gip_build_host[\"])
AC_GIP_DEFINE(BUILD_SYSTEM, [\"]$ac_cv_build[\"])
AC_GIP_DEFINE(TARGET_SYSTEM, [\"]$ac_cv_target[\"])

ac_gip_header=gip/configuration.h
echo "updating configuration..."
mv $ac_gip_config $ac_gip_header

AC_OUTPUT(Makefile gip/Makefile gip/docs/Makefile gip/docs/en/Makefile gip/transformation/Makefile gip/io/Makefile gip/analysis/Makefile )
