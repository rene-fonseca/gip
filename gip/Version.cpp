/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/Version.h>

using namespace base;

namespace gip {

const char Version::banner[] =
"Generic Image Processing (GIP) Framework version "_DK_SDU_MIP__GIP__VERSION"\n"
"A framework for developing image processing applications\n"
"Copyright (C) 2000, 2001, 2002 Rene Moeller Fonseca\n\n"
"This framework is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"Please report bugs to <fonseca@mip.sdu.dk>.\n\n"
"configure: "_DK_SDU_MIP__GIP__CONFIGURE"\n"
"build data: "_DK_SDU_MIP__GIP__BUILD_DATE"\n"
"build user: "_DK_SDU_MIP__GIP__BUILD_USER"\n"
"build host: "_DK_SDU_MIP__GIP__BUILD_HOST"\n"
"build system: "_DK_SDU_MIP__GIP__BUILD_SYSTEM"\n"
"target system: "_DK_SDU_MIP__GIP__TARGET_SYSTEM"\n";

unsigned int Version::getMajorVersion() const throw() {
  return _DK_SDU_MIP__GIP__MAJOR_VERSION;
}

unsigned int Version::getMinorVersion() const throw() {
  return _DK_SDU_MIP__GIP__MINOR_VERSION;
}

unsigned int Version::getMicroVersion() const throw() {
  return _DK_SDU_MIP__GIP__MICRO_VERSION;
}

String Version::getVersion() const throw() {
  return MESSAGE(_DK_SDU_MIP__GIP__VERSION);
}

String Version::getBanner() const throw() {
  return MESSAGE(banner);
}
 
}; // end of namespace
