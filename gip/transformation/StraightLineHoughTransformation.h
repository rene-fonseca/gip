/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

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
    Calculates the straight line Hough transformation of the specified image and
    stores the result in the destination image.

    @short Straight line Hough transformation
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  class StraightLineHoughTransformation : public Transformation<FloatImage, GrayImage> {
  private:

    struct Entry {
      long double cosine;
      long double sine;
    };
    
    /** Lookup table for cosine and sine. */
    Allocator<Entry> lookup;
  public:

    /**
      Initializes the straight line Hough transformation.

      @param destination The destination image.
      @param source The source image.
    */
    StraightLineHoughTransformation(DestinationImage* destination, const SourceImage* source) throw(ImageException);

    /**
      Calculate transformation.
    */
    void operator()() throw();
  };

}; // end of gip namespace
