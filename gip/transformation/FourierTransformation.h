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
    Calculates the Fast Fourier Transform of the specified complex image and
    stores the result in the destination complex image.
    
    @short Fast Fourier Transformation (FFT)
    @ingroup transformations
    @version 1.0
  */

  class _COM_AZURE_DEV__GIP__API FourierTransformation : public Transformation<ComplexImage, ComplexImage> {
  private:

    /** Specifies that a forward Fourier transformation has been requested. */
    bool forward = true;
    /** Lookup table for row indices. */
    Allocator<unsigned int> mappedRows;
    /** Lookup table for column indices. */
    Allocator<unsigned int> mappedColumns;
  public:
    
    /**
      Initializes Fast Fourier transformation object.

      @param destination The destination image.
      @param source The source image.
      @param forward Requests a forward Fourier transformation (inverse transformation if false). Default is true.
    */
    FourierTransformation(DestinationImage* destination, const SourceImage* source, bool forward = true);

    /**
      Fast Fourier transformation.
    */
    void operator()() noexcept;
  };

}; // end of gip namespace
