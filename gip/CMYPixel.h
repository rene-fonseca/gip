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
    The CMY color model stands for Cyan, Magenta and Yellow which are the
    complements of red, green and blue respectively. The CMY color space
    is primarily used for printing. Cyan, magenta, and yellow are called the
    "subtractive primaries".

    @short Subtractive primaries color space
    @see CMYKPixel RGBPixel
    @ingroup pixels
  */
  template<class COMPONENT>
  struct CMYPixel {
    COMPONENT cyan;
    COMPONENT magenta;
    COMPONENT yellow;
  };
  
  template<class COMPONENT>
  class PixelTraits<CMYPixel<COMPONENT> > {
  public:
    
    typedef COMPONENT Component;
    typedef CMYPixel<Component> Pixel;
    typedef PixelComponent<Component>::Arithmetic Arithmetic;

    enum {
      MINIMUM = 0x00,
      MAXIMUM = 0xff // TAG: should be 1.0 if floating-point type?
    };

    class GetOrder : public UnaryOperation<Pixel, Arithmetic> {
    public:
      
      // TAG: problem if components are negative
      inline Arithmetic operator()(const Pixel& pixel) const throw() {
        return static_cast<Arithmetic>(mapToOneDimension(pixel.cyan)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.magenta)) +
          static_cast<Arithmetic>(mapToOneDimension(pixel.yellow));
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
  struct CMYPixel<unsigned char> {
    union {
      unsigned int cmy;
      struct {
        unsigned char cyan;
        unsigned char magenta;
        unsigned char yellow;
      } _DK_SDU_MIP__BASE__PACKED;
    } _DK_SDU_MIP__BASE__PACKED;
  } _DK_SDU_MIP__BASE__PACKED;



  template<class COMPONENT>
  struct CMYKPixel {
    COMPONENT cyan;
    COMPONENT magenta;
    COMPONENT yellow;
    COMPONENT black;
  };

  /**
    This function returns the CMY pixel corresponding to the specified component values.

    @param cyan The cyan component.
    @param magenta The magenta component.
    @param yellow The yellow component.
    
    @short Creates an CMY pixel
  */

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> makeCMYPixel(COMPONENT cyan, COMPONENT magenta, COMPONENT yellow) throw() {
    CMYPixel<COMPONENT> result;
    result.cyan = cyan;
    result.magenta = magenta;
    result.yellow = yellow;
    return result;
  }

  inline CMYKPixel<COMPONENT> makeCMYKPixel(COMPONENT cyan, COMPONENT magenta, COMPONENT yellow, COMPONENT black) throw() {
    CMYKPixel<COMPONENT> result;
    result.cyan = cyan;
    result.magenta = magenta;
    result.yellow = yellow;
    result.black = black;
    return result;
  }

  // TAG: replace this with conversion function/class
  template<class COMPONENT>
  inline CMYKPixel<COMPONENT> makeCMYKPixel(COMPONENT cyan, COMPONENT magenta, COMPONENT yellow) throw() {
    CMYKPixel<COMPONENT> result;
    result.black = minimum(cyan, magenta, yellow);
    result.cyan = (cyan - result.black)/(1 - result.black);
    result.magenta = (magenta - result.black)/(1 - result.black);
    result.yellow = (yellow - result.black)/(1 - result.black);
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
  inline CMYPixel<COMPONENT> blend(CMYPixel<COMPONENT> back, CMYPixel<COMPONENT> front, unsigned int opaque, unsigned int opacity) throw() {
    CMYPixel<COMPONENT> result;
    unsigned int transparency = opaque - opacity;
    result.cyan = (transparency * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(back.cyan) + opacity * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(front.cyan))/opaque;
    result.magenta = (transparency * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(back.magenta) + opacity * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(front.magenta))/opaque;
    result.yellow = (transparency * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(back.yellow) + opacity * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(front.yellow))/opaque;
    return result;
  }

  template<class COMPONENT>
  class Blend : BinaryOperation<CMYPixel<COMPONENT>, CMYPixel<COMPONENT>, CMYPixel<COMPONENT> > {
  private:

    typedef PixelTraits<CMYPixel<COMPONENT> >::Arithmetic Arithmetic;

    /** The level of a fully opaque pixel. */
    Arithmetic opaque;
    /** The opacity of the front pixel. */
    Arithmetic opacity;
    /** The opacity of the background pixel. */
    Arithmetic transparency;
  public:

    inline Blend(Arithmetic _opaque, Arithmetic _opacity) throw()
       : opaque(_opaque),
         opacity(_opacity),
         transparency(opaque - opacity) {
    }

    inline CMYPixel<COMPONENT> operator()(CMYPixel<COMPONENT> back, CMYPixel<COMPONENT> front) const throw() {
      CMYPixel<COMPONENT> result;
      result.cyan = (transparency * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(back.cyan) + opacity * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(front.cyan))/opaque;
      result.magenta = (transparency * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(back.magenta) + opacity * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(front.magenta))/opaque;
      result.yellow = (transparency * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(back.yellow) + opacity * static_cast<PixelTraits<CMYPixel<COMPONENT> >::Arithmetic>(front.yellow))/opaque;
      return result;
    }
  };



  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator*(CMYPixel<COMPONENT> pixel, int factor) throw() {
    pixel.cyan *= factor;
    pixel.magenta *= factor;
    pixel.yellow *= factor;
    return pixel;
  }
  
  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator*(CMYPixel<COMPONENT> pixel, unsigned int factor) throw() {
    pixel.cyan *= factor;
    pixel.magenta *= factor;
    pixel.yellow *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator*(CMYPixel<COMPONENT> pixel, float factor) throw() {
    pixel.cyan *= factor;
    pixel.magenta *= factor;
    pixel.yellow *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator*(CMYPixel<COMPONENT> pixel, double factor) throw() {
    pixel.cyan *= factor;
    pixel.magenta *= factor;
    pixel.yellow *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator*(CMYPixel<COMPONENT> pixel, long double factor) throw() {
    pixel.cyan *= factor;
    pixel.magenta *= factor;
    pixel.yellow *= factor;
    return pixel;
  }

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator/(CMYPixel<COMPONENT> pixel, int divisor) throw() {
    pixel.cyan /= divisor;
    pixel.magenta /= divisor;
    pixel.yellow /= divisor;
    return pixel;
  }

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator/(CMYPixel<COMPONENT> pixel, unsigned int divisor) throw() {
    pixel.cyan /= divisor;
    pixel.magenta /= divisor;
    pixel.yellow /= divisor;
    return pixel;
  }

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator/(CMYPixel<COMPONENT> pixel, float factor) throw() {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator/(CMYPixel<COMPONENT> pixel, double factor) throw() {
    return pixel * 1/factor;
  }

  template<class COMPONENT>
  inline CMYPixel<COMPONENT> operator/(CMYPixel<COMPONENT> pixel, long double factor) throw() {
    return pixel * 1/factor;
  }



  /** Writes the specified CMY color space pixel to the format stream using the format '(cyan, magenta, yellow)'. */
  template<class COMPONENT>
  FormatOutputStream& operator<<(FormatOutputStream& stream, const CMYPixel<COMPONENT>& value) throw(IOException) {
    FormatOutputStream::PushContext pushContext(stream); // make current context the default context
    return stream << '(' << value.cyan << ',' << value.magenta << ',' << value.yellow << ')';
  }

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::CMYPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace
