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
  unsigned int getMajorVersion() const throw();

  /**
    Returns the minor version of the framework.
  */
  unsigned int getMinorVersion() const throw();

  /**
    Returns the micro version of the framework.
  */
  unsigned int getMicroVersion() const throw();

  /**
    Returns the release of the framework as a string. The format of the string
    is unspecified.
  */
  base::String getRelease() const throw();

  /**
    Returns the version of the framework as a string. The format of the string
    is 'major.minor.micro suffix' where suffix could be anything.
  */
  base::String getVersion() const throw();

  /**
    Returns the short commit id.
  */
  base::String getCommit() const throw();

  /*
    Returns the build date in seconds since epoch.
  */
  int64 getBuildDate() const throw();

  /**
    Returns a short banner (multiple lines) describing the framework.
  */
  base::String getBanner() const throw();
};
 
}; // end of gip namespace
