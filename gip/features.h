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

#include <gip/configuration.h>
#include <base/features.h>

#define _COM_AZURE_DEV__GIP__MAJOR_VERSION 0
#define _COM_AZURE_DEV__GIP__MINOR_VERSION 2
#define _COM_AZURE_DEV__GIP__MICRO_VERSION 1
#define _COM_AZURE_DEV__GIP__VERSION "0.2.1"

/** The ID of this module. Use for consumer when registering dependencies. */
#define _COM_AZURE_DEV__GIP__THIS_MODULE "com.azure.dev:renefonseca_gip"

/** MODULE_REGISTER("your.module.id", _COM_AZURE_DEV__GIP__MODULE_INFO). See <base/Module.h>. */
#define _COM_AZURE_DEV__GIP__MODULE_INFO \
  {"PREFIX=com.azure.dev", \
   "NAME=gip", \
   "VERSION=" _COM_AZURE_DEV__GIP__VERSION, \
   "URL=https://dev.azure.com/renefonseca/gip/" \
   "LICENSE=GPL-2.0"}

#define _COM_AZURE_DEV__GIP__REQUIRE(major, minor, micro) \
  ((major <= _COM_AZURE_DEV__GIP__MAJOR_VERSION) && \
   (minor <= _COM_AZURE_DEV__GIP__MINOR_VERSION) && \
   (micro <= _COM_AZURE_DEV__GIP__MICRO_VERSION))

#if (!((_COM_AZURE_DEV__BASE__MAJOR_VERSION >= 0) && \
       (_COM_AZURE_DEV__BASE__MINOR_VERSION >= 9)))
#  error The Base Framework is too old
#endif

/**
  This is the root namespace of <strong>The GIP Framework</strong>. All the
  symbols defined by the framework are contained within this particular
  namespace. You should not add your own symbols to this namespace. Ignoring
  this rule will put your application at risk of clashing with a future release
  of this framework. You should therefore consider this namespace reserved
  solely for this project just like the 'std' namespace is allocated for the
  <strong>Standard C++ Library</strong> (and <strong>C++</strong>
  language).

  @short The root namespace of the framework
*/

namespace gip {
  
  // this framework depends on the base framework
  using namespace com::azure::dev::base;
}

namespace com {
  namespace azure {
    namespace dev {
      namespace gip {
        using namespace ::gip;
      }
    }
  }
}
