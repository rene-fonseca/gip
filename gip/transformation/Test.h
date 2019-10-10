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

#include <gip/transformation/UnaryTransformation.h>

namespace gip {

  /**
    This transformation generates a test image.
    
    @short Test transformation.
    @ingroup transformations
    @version 1.0
  */
  
  class _COM_AZURE_DEV__BASE__API Test : public UnaryTransformation<ColorImage> {
  public:

    Test(DestinationImage* destination) throw();

    void operator()() throw();
  };

}; // end of gip namespace
