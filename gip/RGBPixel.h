/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__RGB_PIXEL_H
#define _DK_SDU_MIP__GIP__RGB_PIXEL_H

#include <gip/PixelTraits.h>
#include <base/Functor.h>

namespace gip {

  /**
    Pixel with red, green, and blue components.

    @see GrayAlphaPixel RGBAPixel
  */
  template<class COMPONENT>
  struct RGBPixel {
    COMPONENT red;
    COMPONENT green;
    COMPONENT blue;
  };
  
  template<class COMPONENT>
  class PixelTraits<RGBPixel<COMPONENT> > {
  public:
    
    typedef COMPONENT Component;
    typedef RGBPixel<Component> Pixel;
    typedef PixelComponent<Component>::Arithmetic Arithmetic;

    enum {
      MINIMUM = 0x00,
      MAXIMUM = 0xff // TAG: should be 1.0 if floating-point type?
    };

    class GetOrder : public UnaryOperation<Pixel, Arithmetic> {
    public:
      
      // TAG: problem if components are negative
      inline Arithmetic operator()(const Pixel& pixel) const throw() {
        return static_cast<Arithmetic>(mapToOneDimension(pixel.red)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.green)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.blue));
      }
    };
    
  };
  
  template<>
  struct RGBPixel<unsigned char> {
    union {
      unsigned int rgb;
      struct {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
      } __attribute__ ((packed));
    } __attribute__ ((packed));
  } __attribute__ ((packed));

  /**
    The RGB pixel type represented by a single 32 bit word (with 8 bit
    components) is so commonplace that it has been given its own name. The
    remaining 8 bits are not used.
  */
  typedef RGBPixel<unsigned char> ColorPixel;



  /**
    This function returns the RGB pixel corresponding to the specified component values.

    @param red The red component.
    @param green The green component.
    @param blue The blue component.
    
    @short Creates an RGB pixel
  */

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> makeRGBPixel(COMPONENT red, COMPONENT green, COMPONENT blue) throw() {
    RGBPixel<COMPONENT> result;
    result.red = red;
    result.green = green;
    result.blue = blue;
    return result;
  }

  inline ColorPixel makeColorPixel(PixelTraits<ColorPixel>::Component red, PixelTraits<ColorPixel>::Component green, PixelTraits<ColorPixel>::Component blue) throw() {
    ColorPixel result;
    result.red = red;
    result.green = green;
    result.blue = blue;
    return result;
  }

  

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, int factor) throw() {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }
  
  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, unsigned int factor) throw() {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, float factor) throw() {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, double factor) throw() {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, long double factor) throw() {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, int divisor) throw() {
    pixel.red /= divisor;
    pixel.green /= divisor;
    pixel.blue /= divisor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, unsigned int divisor) throw() {
    pixel.red /= divisor;
    pixel.green /= divisor;
    pixel.blue /= divisor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, float factor) throw() {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, double factor) throw() {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, long double factor) throw() {
    return pixel * 1/factor;
  }



  template<class COMPONENT>
  class IsRGBPixel<RGBPixel<COMPONENT> > {
  public:
    enum {IS_RGB_PIXEL = true};
  };

  template<class COMPONENT>
  class HasRedComponent<RGBPixel<COMPONENT> > {
  public:
    enum {HAS_RED_COMPONENT = true};
  };
  
  template<class COMPONENT>
  class HasGreenComponent<RGBPixel<COMPONENT> > {
  public:
    enum {HAS_GREEN_COMPONENT = true};
  };
  
  template<class COMPONENT>
  class HasBlueComponent<RGBPixel<COMPONENT> > {
  public:
    enum {HAS_BLUE_COMPONENT = true};
  };

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::RGBPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace

#endif