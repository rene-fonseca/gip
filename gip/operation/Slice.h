/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_OPERATION__SLICE_H
#define _DK_SDU_MIP__GIP_OPERATION__SLICE_H

#include <gip/RGBPixel.h>
#include <base/OutOfDomain.h>

namespace gip {

  /**
    Slice operator.
    
    @short Slice operator.
    @ingroup operator
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class PIXEL>
  class Slice : public UnaryOperation<PIXEL, PIXEL> {
  private:
    
    typedef PIXEL Pixel;
    const Pixel minimum;
    const Pixel maximum;
    const Pixel background;
  public:

    /**
      Initializes a slice operator with the specified slice region and background value.
      
      @param minimum The minimum value.
      @param maximum The maximum value.
      @param background The value to return for pixel value which fall outside the slice. The default is 0.
    */
    inline Slice(const Pixel _minimum, const Pixel _maximum, const Pixel background = 0) throw(OutOfDomain)
      : minimum(_minimum),
        maximum(_maximum),
        background(_background) {
      assert(minimum <= maximum, OutOfDomain(this));
    }

    /**
      Do slice operation on the specified value.
    */
    inline Pixel operator()(const Pixel value) const throw() {
      return ((value < minimum) || (value > maximum)) ? background : value;
    }
  };
  
}; // end of gip namespace

#endif
