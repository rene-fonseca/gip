/***************************************************************************
    The Base Framework
    A framework for developing platform independent applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__FEATURES_H
#define _DK_SDU_MIP__GIP__FEATURES_H

#include <gip/configuration.h>
#include <base/features.h>

#define _DK_SDU_MIP__GIP__MAJOR_VERSION 0
#define _DK_SDU_MIP__GIP__MINOR_VERSION 1
#define _DK_SDU_MIP__GIP__MICRO_VERSION 1
#define _DK_SDU_MIP__GIP__VERSION "0.1.1"

#define _DK_SDU_MIP__GIP__REQUIRE(major, minor, micro) \
  ((major <= _DK_SDU_MIP__GIP__MAJOR_VERSION) && \
  (minor <= _DK_SDU_MIP__GIP__MINOR_VERSION) && \
  (micro <= _DK_SDU_MIP__GIP__MICRO_VERSION))

#if (!((_DK_SDU_MIP__BASE__MAJOR_VERSION >= 0) && (_DK_SDU_MIP__BASE__MINOR_VERSION >= 9)))
  #error The Base Framework is too old
#endif

/**
  This is the root namespace of <strong>The GIP Framework</strong>. All the
  symbols defined by the framework are contained within this particular
  namespace. You should not add your own symbols to this namespace. Ignoring
  this rule will put your application at risk of clashing with a future release
  of this framework. You should therefore consider this namespace reserved
  solely for this project just like the 'std' namespace is allocated for the
  <strong>Standard Template Library</strong> (and <strong>C++</strong>
  language).

  @short The root namespace of the framework
*/

namespace gip {
};

#endif
