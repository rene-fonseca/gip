/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__VERSION_H
#define _DK_SDU_MIP__GIP__VERSION_H

#include <base/Version.h>
#include <gip/features.h>

namespace gip {

/**
  This class returns information concerning the framework.

  @short Framework information.
  @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
  @version 1.0
*/

class Version : public base::Version {
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
    Returns the version of the framework as a string. The format of the string
    is 'major.minor.micro suffix' where suffix could be anything.
  */
  base::String getVersion() const throw();

  /**
    Returns a short banner (multiple lines) describing the framework.
  */
  base::String getBanner() const throw();
};
 
}; // end of namespace

#endif
