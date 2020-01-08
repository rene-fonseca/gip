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
#include <base/Module.h>
#include <base/Version.h>
#include <gip/Version.h>

using namespace com::azure::dev::gip;

class VersionApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  VersionApplication() noexcept
    : Application(MESSAGE("gip"))
  {
  }

  void main() noexcept
  {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << EOL
         << gip::Version().getBanner() << EOL
         << EOL
         << base::Version().getBanner() << ENDL;

    auto& manager = ModuleManager::getManager();
    if (!manager.traverseModules()) {
      setExitCode(1);
      return;
    }
  }

};

APPLICATION_STUB(VersionApplication);
