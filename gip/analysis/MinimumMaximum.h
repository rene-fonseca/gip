/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_ANALYSIS__MINIMUM_MAXIMUM_H
#define _DK_SDU_MIP__GIP_ANALYSIS__MINIMUM_MAXIMUM_H

#include <gip/Pixel.h>
#include <base/Functor.h>

namespace gip {
  
  /**
    This unary operation is used to collect the minimum and maximum values of
    some group of pixels.
    
    @short Unary operation used to find the minimum and maximum values
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class PIXEL>
  class MinimumMaximum : public UnaryOperation<PIXEL, void> {
  private:
    
    typedef PIXEL Pixel;
    
    Pixel minimumValue;
    Pixel maximumValue;
  public:
    
    inline MinimumMaximum() throw()
      : maximumValue(PixelTraits<Pixel>::MINIMUM),
        minimumValue(PixelTraits<Pixel>::MAXIMUM) {
    }
    
    inline void operator()(const Pixel& value) throw() {
      if (value > maximumValue) {
        maximumValue = value;
      } else if (value < minimumValue) {
        minimumValue = value;
      }
    }
    
    inline Pixel getMinimum() const throw() {
      return minimumValue;
    }
    
    inline Pixel getMaximum() const throw() {
      return maximumValue;
    }
  };

}; // end of gip namespace

#endif
