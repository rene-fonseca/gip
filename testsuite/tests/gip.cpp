/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Version.h>
#include <gip/Version.h>
#include <gip/io/BMPEncoder.h>

using namespace dk::sdu::mip::gip;

class VersionApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  VersionApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("gip"), numberOfArguments, arguments, environment) {
  }

  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("http://www.mip.sdu.dk/~fonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002-2019 by Rene Moeller Fonseca") << EOL
         << EOL
         << gip::Version().getBanner() << EOL
         << EOL
         << base::Version().getBanner() << ENDL;
  }

};

STUB(VersionApplication);
