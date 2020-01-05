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
#include <gip/config.h>

#define _COM_AZURE_DEV__GIP__RELEASE ""

namespace gip {

const char Version::banner[] =
"Generic Image Processing (GIP) Framework release " _COM_AZURE_DEV__GIP__RELEASE "\n"
"A framework for developing image processing applications\n"
"Copyright (C) 2001-2020 Rene Moeller Fonseca\n\n"
"This framework is distributed in the hope that it will be useful,\n"
"but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"Please report bugs to https://dev.azure.com/renefonseca/gip.\n\n"
"build data: " _COM_AZURE_DEV__GIP__BUILD_DATE "\n";

unsigned int Version::getMajorVersion() const noexcept
{
  return _COM_AZURE_DEV__GIP__MAJOR_VERSION;
}

unsigned int Version::getMinorVersion() const noexcept
{
  return _COM_AZURE_DEV__GIP__MINOR_VERSION;
}

unsigned int Version::getMicroVersion() const noexcept
{
  return _COM_AZURE_DEV__GIP__GIT_REVISION;
}

String Version::getRelease() const noexcept
{
  return Literal(_COM_AZURE_DEV__GIP__RELEASE);
}

String Version::getVersion() const noexcept
{
  return Literal(_COM_AZURE_DEV__GIP__VERSION);
}

String Version::getCommit() const noexcept
{
  return Literal(_COM_AZURE_DEV__GIP__GIT_COMMIT_SHORT);
}

int64 Version::getBuildDate() const noexcept
{
  return _COM_AZURE_DEV__GIP__BUILD_DATE_SECONDS;
}

String Version::getBanner() const noexcept
{
  return String(banner);
}

}; // end of gip namespace
