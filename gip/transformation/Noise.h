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
#include <base/Random.h>

namespace gip {

  template<class PIXEL>
  class NoiseOperation {
  public:
    
    typedef PIXEL Pixel;

    inline Pixel operator()() const throw() {
      return Random::random<double>() * PixelTraits<Pixel>::MAXIMUM;
    }
  };
  
  template<class COMPONENT>
  class NoiseOperation<RGBPixel<COMPONENT> > {
  public:

    typedef RGBPixel<COMPONENT> Pixel;
    
    inline Pixel operator()() const throw() {
      return makeRGBPixel(
        Random::random<double>() * PixelTraits<Pixel>::MAXIMUM,
        Random::random<double>() * PixelTraits<Pixel>::MAXIMUM,
        Random::random<double>() * PixelTraits<Pixel>::MAXIMUM
      );
    }
  };

  template<>
  class NoiseOperation<ColorPixel> {
  public:
    
    inline ColorPixel operator()() const throw() {
      ColorPixel result;
      result.rgb = Random::random<int>();
      return result;
    }
  };

  /**
    Fills an image with noise.

    @short Fills the destination with noise.
    @ingroup transformations
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
        UnaryTransformation<DEST>::destination->getElements(),
        UnaryTransformation<DEST>::destination->getDimension().getSize(),
        NoiseOperation<Pixel>()
      );
    }
  };

}; // end of gip namespace
