/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__GRAY_ALPHA_PIXEL_H
#define _DK_SDU_MIP__GIP__GRAY_ALPHA_PIXEL_H

#include <gip/PixelTraits.h>
#include <base/Functor.h>
#include <base/string/FormatOutputStream.h>

namespace gip {

  /**
    Pixel with gray (achromatic) and alpha components.

    @short Gray (achromatic) pixel with alpha component
    @see RGBPixel RGBAPixel
    @ingroup pixels
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  template<class COMPONENT>
  struct GrayAlphaPixel {
    COMPONENT gray;
    COMPONENT alpha;
  };

  template<class COMPONENT>
  class PixelTraits<GrayAlphaPixel<COMPONENT> > {
  public:
    
    typedef COMPONENT Component;
    typedef GrayAlphaPixel<Component> Pixel;
    typedef typename PixelComponent<Component>::Arithmetic Arithmetic;
    
    enum {
      MINIMUM = 0x00,
      MAXIMUM = 0xff,
      ALPHA_IS_OPACITY = true,
      ALPHA_OPAQUE_IS_MAXIMUM = true
    };

    class GetOrder : public UnaryOperation<Pixel, Arithmetic> {
    public:

      // TAG: problem if components are negative
      inline Arithmetic operator()(const Pixel& pixel) const throw() {
        return static_cast<Arithmetic>(mapToOneDimension(pixel.red)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.alpha)); // TAG: alpha should not be included
      }
    };

    class Clamp : public UnaryOperation<Arithmetic, Arithmetic> {
    public:

      inline Arithmetic operator()(const Arithmetic& value) const throw() {
        if (value >= MAXIMUM) {
          return MAXIMUM;
        } else if (value < MINIMUM) {
          return MINIMUM;
        } else {
          return value;
        }
      }
    };

  };
  
  template<>
  struct GrayAlphaPixel<unsigned char> {
    union {
      unsigned int ga;
      struct {
        unsigned char gray;
        unsigned char alpha;
      } _DK_SDU_MIP__BASE__PACKED;
    } _DK_SDU_MIP__BASE__PACKED;
  } _DK_SDU_MIP__BASE__PACKED;

  typedef GrayAlphaPixel<unsigned char> GrayAlphaPixel8;

  template<>
  struct GrayAlphaPixel<unsigned short> {
    union {
      unsigned int ga;
      struct {
        unsigned short gray;
        unsigned short alpha;
      } _DK_SDU_MIP__BASE__PACKED;
    } _DK_SDU_MIP__BASE__PACKED;
  } _DK_SDU_MIP__BASE__PACKED;
  
  typedef GrayAlphaPixel<unsigned short> GrayAlphaPixel16;

  template<>
  class PixelTraits<GrayAlphaPixel16> {
  public:
    
    typedef unsigned short Component;
    typedef GrayAlphaPixel16 Pixel;
    typedef PixelComponent<unsigned short>::Arithmetic Arithmetic;
    
    enum {
      MINIMUM = 0x0000,
      MAXIMUM = 0xffff,
      ALPHA_IS_OPACITY = true,
      ALPHA_OPAQUE_IS_MAXIMUM = true
    };
  };
  


  /**
    This function returns the GrayAlpha pixel corresponding to the specified
    component values.

    @param gray The gray component.
    @param alpha The alpha component.
    
    @short Creates a GrayAlpha pixel
  */

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> makeGrayAlphaPixel(COMPONENT gray, COMPONENT alpha) throw() {
    GrayAlphaPixel<COMPONENT> result;
    result.gray = gray;
    result.alpha = alpha;
    return result;
  }

  inline GrayAlphaPixel8 makeGrayAlphaPixel8(PixelTraits<GrayAlphaPixel8>::Component gray, PixelTraits<GrayAlphaPixel8>::Component alpha) throw() {
    GrayAlphaPixel8 result;
    result.gray = gray;
    result.alpha = alpha;
    return result;
  }

  inline GrayAlphaPixel16 makeGrayAlphaPixel16(PixelTraits<GrayAlphaPixel16>::Component gray, PixelTraits<GrayAlphaPixel16>::Component alpha) throw() {
    GrayAlphaPixel16 result;
    result.gray = gray;
    result.alpha = alpha;
    return result;
  }



  /**
    Blends two pixels according to the specified level of opacity.

    @param back The pixel in the background.
    @param front The pixel in the foreground.
    @param opaque The level indicating a fully opaque pixel.
    @param opacity The opacity level of the new pixel. This must be in the range [0; opaque].
  */
  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> blend(GrayAlphaPixel<COMPONENT> back, GrayAlphaPixel<COMPONENT> front, unsigned int opaque, unsigned int opacity) throw() {
    GrayAlphaPixel<COMPONENT> result;
    unsigned int transparency = opaque - opacity;
    result.gray = (transparency * static_cast<typename PixelTraits<GrayAlphaPixel<COMPONENT> >::Arithmetic>(back.gray) + opacity * static_cast<typename PixelTraits<GrayAlphaPixel<COMPONENT> >::Arithmetic>(front.red))/opaque;
    result.alpha = (transparency * static_cast<typename PixelTraits<GrayAlphaPixel<COMPONENT> >::Arithmetic>(back.alpha) + opacity * static_cast<typename PixelTraits<GrayAlphaPixel<COMPONENT> >::Arithmetic>(front.green))/opaque;
    return result;
  }

  

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator*(GrayAlphaPixel<COMPONENT> pixel, int factor) throw() {
    pixel.gray *= factor;
    pixel.alpha *= factor;
    return pixel;
  }
  
  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator*(GrayAlphaPixel<COMPONENT> pixel, unsigned int factor) throw() {
    pixel.gray *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator*(GrayAlphaPixel<COMPONENT> pixel, float factor) throw() {
    pixel.gray *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator*(GrayAlphaPixel<COMPONENT> pixel, double factor) throw() {
    pixel.gray *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator*(GrayAlphaPixel<COMPONENT> pixel, long double factor) throw() {
    pixel.gray *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator/(GrayAlphaPixel<COMPONENT> pixel, int divisor) throw() {
    pixel.gray *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator/(GrayAlphaPixel<COMPONENT> pixel, unsigned int divisor) throw() {
    pixel.gray *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator/(GrayAlphaPixel<COMPONENT> pixel, float factor) throw() {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator/(GrayAlphaPixel<COMPONENT> pixel, double factor) throw() {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline GrayAlphaPixel<COMPONENT> operator/(GrayAlphaPixel<COMPONENT> pixel, long double factor) throw() {
    return pixel * 1/factor;
  }



  template<class COMPONENT>
  class IsGrayAlphaPixel<GrayAlphaPixel<COMPONENT> > {
  public:
    enum {IS_GRAY_ALPHA_PIXEL = true};
  };



  template<class COMPONENT>
  class HasGrayComponent<GrayAlphaPixel<COMPONENT> > {
  public:
    enum {HAS_GRAY_COMPONENT = true};
  };

  template<class COMPONENT>
  class HasAlphaComponent<GrayAlphaPixel<COMPONENT> > {
  public:
    enum {HAS_ALPHA_COMPONENT = true};
  };

  /** Writes the specified GrayAlpha pixel to the format stream using the format '(gray, alpha)'. */
  template<class COMPONENT>
  FormatOutputStream& operator<<(FormatOutputStream& stream, const GrayAlphaPixel<COMPONENT>& value) throw(IOException) {
    FormatOutputStream::PushContext pushContext(stream); // make current context the default context
    return stream << '(' << value.gray << ',' << value.alpha << ')';
  }

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::GrayAlphaPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace

#endif
