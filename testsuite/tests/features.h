/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TESTSUITE_TESTS__FEATURES_H
#define _DK_SDU_MIP__GIP_TESTSUITE_TESTS__FEATURES_H

#include <gip/features.h>

#if (!((_DK_SDU_MIP__BASE__MAJOR_VERSION >= 0) && (_DK_SDU_MIP__BASE__MINOR_VERSION >= 9)))
#  error The Base Framework is too old
#endif

#if (!((_DK_SDU_MIP__GIP__MAJOR_VERSION >= 0) && (_DK_SDU_MIP__GIP__MINOR_VERSION >= 1)))
#  error The GIP Framework is too old
#endif

#endif
