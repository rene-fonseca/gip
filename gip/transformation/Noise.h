/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__NOISE_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__NOISE_H

#include <gip/transformation/UnaryTransformation.h>
#include <base/mathematics/Random.h>

namespace gip {

  template<class PIXEL>
  class NoiseOperation {
  public:
    
    typedef PIXEL Pixel;

    inline Pixel operator()() const throw() {
      return Random::getLongDouble() * PixelTraits<Pixel>::MAXIMUM;
    }
  };
  
  template<class COMPONENT>
  class NoiseOperation<RGBPixel<COMPONENT> > {
  public:

    typedef RGBPixel<COMPONENT> Pixel;
    
    inline Pixel operator()() const throw() {
      return makeRGBPixel(
        Random::getLongDouble() * PixelTraits<Pixel>::MAXIMUM,
        Random::getLongDouble() * PixelTraits<Pixel>::MAXIMUM,
        Random::getLongDouble() * PixelTraits<Pixel>::MAXIMUM
      );
    }
  };

  template<>
  class NoiseOperation<ColorPixel> {
  public:
    
    inline ColorPixel operator()() const throw() {
      ColorPixel result;
      result.rgb = Random::getInteger();
      return result;
    }
  };

  /**
    Fills an image with noise.

    @short Fills the destination with noise.
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  
  template<class DEST>
  class Noise : public UnaryTransformation<DEST> {
  public:
    
    typedef typename UnaryTransformation<DEST>::DestinationImage DestinationImage;
    typedef typename DestinationImage::Pixel Pixel;    
    
    /**
      Initializes noise object.

      @param destination The destination image.
    */
    Noise(DestinationImage* destination) throw()
      : UnaryTransformation<DestinationImage>(destination) {
    }
    
    /**
      Fills the destination image with noise.
    */
    void operator()() throw() {
      forEach(
        destination->getElements(),
        destination->getDimension().getSize(),
        NoiseOperation<Pixel>()
      );
    }
  };

}; // end of gip namespace

#endif
