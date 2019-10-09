/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/Version.h>

/*
#define _DK_SDU_MIP__GIP__RELEASE "1.0"
*/
#define _DK_SDU_MIP__GIP__CONFIGURE ""
#define _DK_SDU_MIP__GIP__BUILD_DATE ""

namespace gip {

const char Version::banner[] =
"Generic Image Processing (GIP) Framework release " _DK_SDU_MIP__GIP__RELEASE "\n"
"A framework for developing image processing applications\n"
"Copyright (C) 2001-2019 Rene Moeller Fonseca\n\n"
"This framework is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"Please report bugs to https://dev.azure.com/renefonseca/gip.\n\n"
"configure: " _DK_SDU_MIP__GIP__CONFIGURE "\n"
"build data: " _DK_SDU_MIP__GIP__BUILD_DATE "\n";

unsigned int Version::getMajorVersion() const throw() {
  return _DK_SDU_MIP__GIP__MAJOR_VERSION;
}

unsigned int Version::getMinorVersion() const throw() {
  return _DK_SDU_MIP__GIP__MINOR_VERSION;
}

unsigned int Version::getMicroVersion() const throw() {
  return _DK_SDU_MIP__GIP__MICRO_VERSION;
}

String Version::getRelease() const throw() {
  return Literal(_DK_SDU_MIP__GIP__RELEASE);
}

String Version::getVersion() const throw() {
  return Literal(_DK_SDU_MIP__GIP__VERSION);
}

String Version::getBanner() const throw() {
  return String(banner);
}
 
}; // end of gip namespace
