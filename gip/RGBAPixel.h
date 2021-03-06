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

#include <gip/PixelTraits.h>
#include <base/Functor.h>

namespace gip {

  /**
    Pixel with red, green, blue, and alpha components. By convention the alpha
    component specifies the level of opacity with the maximum and minimum value
    meaning respectively fully opaeque and fully transparent. If the alpha
    component is given a different meaning it should be explicitly described in
    the documentation.

    @short True color pixel with alpha component
    @see GrayAlphaPixel RGBPixel
    @ingroup pixels
  */
  template<class COMPONENT>
  struct RGBAPixel {
    COMPONENT red;
    COMPONENT green;
    COMPONENT blue;
    COMPONENT alpha;
  };
  
  template<class COMPONENT>
  class PixelTraits<RGBAPixel<COMPONENT> > {
  public:
    
    typedef COMPONENT Component;
    typedef RGBAPixel<Component> Pixel;
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
      inline Arithmetic operator()(const Pixel& pixel) const noexcept {
        return static_cast<Arithmetic>(mapToOneDimension(pixel.red)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.green)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.blue)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.alpha)); // TAG: alpha should not be included
      }
    };

    class Clamp : public UnaryOperation<Arithmetic, Arithmetic> {
    public:

      inline Arithmetic operator()(const Arithmetic& value) const noexcept {
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

_COM_AZURE_DEV__BASE__PACKED__BEGIN
  template<>
  struct RGBAPixel<unsigned char> {
    union {
      unsigned int rgba;
      struct {
        unsigned char red;
        unsigned char green;
        unsigned char blue;
        unsigned char alpha;
      } _COM_AZURE_DEV__BASE__PACKED;
    } _COM_AZURE_DEV__BASE__PACKED;
  } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END

  /**
    The RGBA pixel type represented by a single 32 bit word (with 8 bit
    components) is so commonplace that it has been given its own name.
  */
  typedef RGBAPixel<unsigned char> ColorAlphaPixel;



  /**
    This function returns the RGBA pixel corresponding to the specified component values.

    @param red The red component.
    @param green The green component.
    @param blue The blue component.
    @param alpha The alpha (opacity) component.
    
    @short Creates an RGBA pixel
  */

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> makeRGBAPixel(COMPONENT red, COMPONENT green, COMPONENT blue, COMPONENT alpha) noexcept {
    RGBAPixel<COMPONENT> result;
    result.red = red;
    result.green = green;
    result.blue = blue;
    result.alpha = alpha;
    return result;
  }
  
  inline ColorAlphaPixel makeColorAlphaPixel(PixelTraits<ColorAlphaPixel>::Component red, PixelTraits<ColorAlphaPixel>::Component green, PixelTraits<ColorAlphaPixel>::Component blue, PixelTraits<ColorAlphaPixel>::Component alpha) noexcept {
    ColorAlphaPixel result;
    result.red = red;
    result.green = green;
    result.blue = blue;
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
  inline RGBAPixel<COMPONENT> blend(RGBAPixel<COMPONENT> back, RGBAPixel<COMPONENT> front, unsigned int opaque, unsigned int opacity) noexcept {
    RGBAPixel<COMPONENT> result;
    unsigned int transparency = opaque - opacity;
    result.red = (transparency * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(back.red) + opacity * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(front.red))/opaque;
    result.green = (transparency * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(back.green) + opacity * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(front.green))/opaque;
    result.blue = (transparency * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(back.blue) + opacity * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(front.blue))/opaque;
    result.alpha = (transparency * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(back.blue) + opacity * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(front.alpha))/opaque;
    return result;
  }



  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator*(RGBAPixel<COMPONENT> pixel, int factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    pixel.alpha *= factor;
    return pixel;
  }
  
  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator*(RGBAPixel<COMPONENT> pixel, unsigned int factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator*(RGBAPixel<COMPONENT> pixel, float factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator*(RGBAPixel<COMPONENT> pixel, double factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator*(RGBAPixel<COMPONENT> pixel, long double factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    pixel.alpha *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator/(RGBAPixel<COMPONENT> pixel, int divisor) noexcept {
    pixel.red /= divisor;
    pixel.green /= divisor;
    pixel.blue /= divisor;
    pixel.alpha /= divisor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator/(RGBAPixel<COMPONENT> pixel, unsigned int divisor) noexcept {
    pixel.red /= divisor;
    pixel.green /= divisor;
    pixel.blue /= divisor;
    pixel.alpha /= divisor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator/(RGBAPixel<COMPONENT> pixel, float factor) noexcept {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator/(RGBAPixel<COMPONENT> pixel, double factor) noexcept {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline RGBAPixel<COMPONENT> operator/(RGBAPixel<COMPONENT> pixel, long double factor) noexcept {
    return pixel * 1/factor;
  }



  template<class COMPONENT>
  class IsRGBAPixel<RGBAPixel<COMPONENT> > {
  public:
    enum {IS_RGBA_PIXEL = true};
  };

  template<class COMPONENT>
  class HasRedComponent<RGBAPixel<COMPONENT> > {
  public:
    enum {HAS_RED_COMPONENT = true};
  };
  
  template<class COMPONENT>
  class HasGreenComponent<RGBAPixel<COMPONENT> > {
  public:
    enum {HAS_GREEN_COMPONENT = true};
  };
  
  template<class COMPONENT>
  class HasBlueComponent<RGBAPixel<COMPONENT> > {
  public:
    enum {HAS_BLUE_COMPONENT = true};
  };

  template<class COMPONENT>
  class HasAlphaComponent<RGBAPixel<COMPONENT> > {
  public:
    enum {HAS_ALPHA_COMPONENT = true};
  };

  /** Writes the specified RGBA pixel to the format stream using the format '(red, green, blue, alpha)'. */
  template<class COMPONENT>
  FormatOutputStream& operator<<(FormatOutputStream& stream, const RGBAPixel<COMPONENT>& value) {
    FormatOutputStream::PushContext pushContext(stream); // make current context the default context
    return stream << '(' << value.red << ',' << value.green << ',' << value.blue << ',' << value.alpha << ')';
  }

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::RGBAPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace
