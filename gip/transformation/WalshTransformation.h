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

#include <gip/gip.h>
#include <gip/transformation/Transformation.h>
#include <base/mem/Allocator.h>

namespace gip {

  /**
    Calculates the Fast Walsh Transformation of the specified gray image and
    stores the result in the destination image.

    @short Fast Walsh Transformation (FWT)
    @ingroup transformations
    @version 1.0
  */

  class _COM_AZURE_DEV__GIP__API WalshTransformation : public Transformation<FloatImage, FloatImage> {
  private:

    /** Lookup table for row indices. */
    Allocator<unsigned int> mappedRows;
    /** Lookup table for column indices. */
    Allocator<unsigned int> mappedColumns;
  public:

    /**
      Initializes Fast Walsh Transformation object.

      @param destination The destination image.
      @param source The source image.
    */
    WalshTransformation(DestinationImage* destination, const SourceImage* source);

    /**
      Calculate transformation.
    */
    void operator()() noexcept;
  };

}; // end of gip namespace
