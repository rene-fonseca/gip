/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <base/Version.h>
#include <gip/features.h>

namespace gip {

/**
  This class provides information concerning the framework.
  
  @short Framework information.
  @version 1.0
*/

class _COM_AZURE_DEV__GIP__API Version : public base::Version {
private:

  /** Banner. */
  static const char banner[];
public:

  /**
    Returns the major version of the framework.
  */
  unsigned int getMajorVersion() const noexcept;

  /**
    Returns the minor version of the framework.
  */
  unsigned int getMinorVersion() const noexcept;

  /**
    Returns the micro version of the framework.
  */
  unsigned int getMicroVersion() const noexcept;

  /**
    Returns the release of the framework as a string. The format of the string
    is unspecified.
  */
  String getRelease() const noexcept;

  /**
    Returns the version of the framework as a string. The format of the string
    is 'major.minor.micro suffix' where suffix could be anything.
  */
  String getVersion() const noexcept;

  /**
    Returns the short commit id.
  */
  String getCommit() const noexcept;

  /*
    Returns the build date in seconds since epoch.
  */
  int64 getBuildDate() const noexcept;

  /**
    Returns a short banner (multiple lines) describing the framework.
  */
  String getBanner() const noexcept;
};
 
}; // end of gip namespace
