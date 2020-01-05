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
    True color is the provision of three separate components for additive red, green, and blue reproduction.

    @short True color pixel
    @see GrayAlphaPixel RGBAPixel
    @ingroup pixels
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
    typedef typename PixelComponent<Component>::Arithmetic Arithmetic;

    enum {
      MINIMUM = 0x00,
      MAXIMUM = 0xff // TAG: should be 1.0 if floating-point type?
    };

    class GetOrder : public UnaryOperation<Pixel, Arithmetic> {
    public:
      
      // TAG: problem if components are negative
      inline Arithmetic operator()(const Pixel& pixel) const noexcept {
        return static_cast<Arithmetic>(mapToOneDimension(pixel.red)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.green)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.blue));
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
  struct RGBPixel<uint8> {
    union {
      unsigned int rgb;
      struct {
        uint8 red;
        uint8 green;
        uint8 blue;
      } _COM_AZURE_DEV__BASE__PACKED;
    } _COM_AZURE_DEV__BASE__PACKED;
  } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END

  /**
    The RGB pixel type represented by a single 32 bit word (with 8 bit
    components) is so commonplace that it has been given its own name. The
    remaining 8 bits are not used.
  */
  typedef RGBPixel<uint8> ColorPixel;

_COM_AZURE_DEV__BASE__PACKED__BEGIN
  struct HiColor {
    union {
      uint16 rgb;
      struct {
        uint16 red : 5;
        uint16 green : 6;
        uint16 blue : 5;
      } _COM_AZURE_DEV__BASE__PACKED;
    } _COM_AZURE_DEV__BASE__PACKED;
  } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END

  


  /**
    This function returns the RGB pixel corresponding to the specified component values.

    @param red The red component.
    @param green The green component.
    @param blue The blue component.
    
    @short Creates an RGB pixel
  */

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> makeRGBPixel(COMPONENT red, COMPONENT green, COMPONENT blue) noexcept {
    RGBPixel<COMPONENT> result;
    result.red = red;
    result.green = green;
    result.blue = blue;
    return result;
  }

  inline ColorPixel makeColorPixel(PixelTraits<ColorPixel>::Component red, PixelTraits<ColorPixel>::Component green, PixelTraits<ColorPixel>::Component blue) noexcept {
    ColorPixel result;
    result.red = red;
    result.green = green;
    result.blue = blue;
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
  inline RGBPixel<COMPONENT> blend(RGBPixel<COMPONENT> back, RGBPixel<COMPONENT> front, unsigned int opaque, unsigned int opacity) noexcept {
    RGBPixel<COMPONENT> result;
    unsigned int transparency = opaque - opacity;
    result.red = (transparency * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(back.red) + opacity * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(front.red))/opaque;
    result.green = (transparency * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(back.green) + opacity * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(front.green))/opaque;
    result.blue = (transparency * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(back.blue) + opacity * static_cast<typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic>(front.blue))/opaque;
    return result;
  }

  template<>
  inline ColorPixel blend(ColorPixel back, ColorPixel front, unsigned int opaque, unsigned int opacity) noexcept {
    ColorPixel result;
    if (opaque == 2 * opacity) { // handle average
      result.rgb = (((back.rgb ^ front.rgb) & 0xfefefefe) >> 1) + back.rgb & front.rgb; // no overflow
    } else {
      unsigned int transparency = opaque - opacity;
      result.red = (2 * transparency * static_cast<PixelTraits<ColorPixel>::Arithmetic>(back.red) + 2 * opacity * static_cast<PixelTraits<ColorPixel>::Arithmetic>(front.red) + opaque)/(2 * opaque);
      result.green = (2 * transparency * static_cast<PixelTraits<ColorPixel>::Arithmetic>(back.green) + 2 * opacity * static_cast<PixelTraits<ColorPixel>::Arithmetic>(front.green) + opaque)/(2 * opaque);
      result.blue = (2 * transparency * static_cast<PixelTraits<ColorPixel>::Arithmetic>(back.blue) + 2 * opacity * static_cast<PixelTraits<ColorPixel>::Arithmetic>(front.blue) + opaque)/(2 * opaque);
    }
    return result;
  }
  


  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, int factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }
  
  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, unsigned int factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, float factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, double factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator*(RGBPixel<COMPONENT> pixel, long double factor) noexcept {
    pixel.red *= factor;
    pixel.green *= factor;
    pixel.blue *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, int divisor) noexcept {
    pixel.red /= divisor;
    pixel.green /= divisor;
    pixel.blue /= divisor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, unsigned int divisor) noexcept {
    pixel.red /= divisor;
    pixel.green /= divisor;
    pixel.blue /= divisor;
    return pixel;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, float factor) noexcept {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, double factor) noexcept {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline RGBPixel<COMPONENT> operator/(RGBPixel<COMPONENT> pixel, long double factor) noexcept {
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

  /** Writes the specified RGB color space pixel to the format stream using the format '(red, green, blue)'. */
  template<class COMPONENT>
  FormatOutputStream& operator<<(FormatOutputStream& stream, const RGBPixel<COMPONENT>& value) throw(IOException) {
    FormatOutputStream::PushContext pushContext(stream); // make current context the default context
    return stream << '(' << value.red << ',' << value.green << ',' << value.blue << ')';
  }

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::RGBPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace
