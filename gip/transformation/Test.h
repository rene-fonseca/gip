/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__TEST_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__TEST_H

#include <gip/transformation/UnaryTransformation.h>

namespace gip {

  /**
    Test transformation.

    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @ingroup transformations
    @version 1.0
  */

  class Test : public UnaryTransformation<ColorImage> {
  public:

    Test(DestinationImage* destination) throw();

    void operator()() throw();
  };

}; // end of gip namespace

#endif
