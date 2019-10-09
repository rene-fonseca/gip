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
#include <gip/RGBPixel.h>

namespace gip {

/**
  Pixel specified by luma and two chroma (blue and red) components.

  @short Y'CbCr color space pixel
  @ingroup pixels
  @version 1.0
*/

template<class COMPONENT>
struct YCbCrPixel {

  /** The quantity representative of luminance (luma) component (Y'). This is not the same as the CIE luminance! */
  COMPONENT luma;
  /** Color difference (chroma) component for blue channel (B' - Y'). */
  COMPONENT cb;
  /** Color difference (chroma) component for red channel (R' - Y'). */
  COMPONENT cr;
};

template<class COMPONENT>
class PixelTraits<YCbCrPixel<COMPONENT> > {
public:

  typedef COMPONENT Component;
  typedef YCbCrPixel<Component> Pixel;
  typedef typename PixelComponent<Component>::Arithmetic Arithmetic;

  enum {
    MINIMUM = 0x00,
    MAXIMUM = 0xff // TAG: should be 1.0 if floating-point type
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

_DK_SDU_MIP__BASE__PACKED__BEGIN
template<>
struct YCbCrPixel<unsigned char> {
  union {
    unsigned int ycbcr;
    struct {
      unsigned char luma;
      unsigned char cb;
      unsigned char cr;
    } _DK_SDU_MIP__BASE__PACKED;
  } _DK_SDU_MIP__BASE__PACKED;
} _DK_SDU_MIP__BASE__PACKED;
_DK_SDU_MIP__BASE__PACKED__END

/**
  This function returns the Y'CbCr pixel corresponding to the specified component values.

  @param luma The luma (not to be confused with the luminance) component.
  @param cb The chroma component for the blue channel.
  @param cr The chroma component for the red channel.

  @short Creates an Y'CbCr pixel
  @see YCbCrPixel
*/
template<class COMPONENT>
inline YCbCrPixel<COMPONENT> makeYCbCrPixel(COMPONENT luma, COMPONENT cb, COMPONENT cr) throw() {
  YCbCrPixel<COMPONENT> result;
  result.luma = luma;
  result.cb = cb;
  result.cr = cr;
  return result;
}

/**
  Converts an RGB pixel into the Y'CbCr color space. The components of the RGB pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline YCbCrPixel<COMPONENT> RGBToYCbCr(const RGBPixel<COMPONENT>& pixel) throw() {
  YCbCrPixel<COMPONENT> result;
  typedef typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic Arithmetic;

  // see ITU-R recommendation BT (map to range [0; 1])
  result.luma = 0.299 * static_cast<Arithmetic>(pixel.red) + 0.587 * static_cast<Arithmetic>(pixel.green) + 0.114 * static_cast<Arithmetic>(pixel.blue);

  result.cb = (pixel.blue - result.luma) * 0.5/(1.000-0.114); // map to range [-0.5; 0.5]
  result.cr = (pixel.red - result.luma) * 0.5/(1.000-0.299); // map to range [-0.5; 0.5]
  return result;
}

/**
  Converts an RGB pixel into the Y'CbCr color space. The components will be mapped from the range [0; 255] into [0; 255].
*/
template<>
inline YCbCrPixel<unsigned char> RGBToYCbCr<unsigned char>(const RGBPixel<unsigned char>& pixel) throw() {
  YCbCrPixel<unsigned char> result;
  typedef PixelTraits<RGBPixel<unsigned char> >::Arithmetic Arithmetic;
  // we know that overflow isn't possible: (299 + 587 + 114) * 255 * 255 <= PrimitiveTraits<PixelTraits<ColorPixel>::Arithmetic>::MAXIMUM
  // see ITU-R recommendation BT (map into range [0; 255])
  Arithmetic temp = 299 * static_cast<Arithmetic>(pixel.red) + 587 * static_cast<Arithmetic>(pixel.green) + 114 * static_cast<Arithmetic>(pixel.blue);
  result.luma = (temp + 1000/2)/1000; // round to nearest
  result.cb = ((1000 * static_cast<Arithmetic>(pixel.blue) - temp) + (255+1)*(1000-114))/(2 * (1000-114)); // round to nearest
  result.cr = ((1000 * static_cast<Arithmetic>(pixel.red) - temp) + (255+1)*(1000-299))/(2 * (1000-299)); // round to nearest
  return result;
}

/**
  Converts an Y'CbCr pixel into the RGB color space. The components of the Y'CbCr pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline RGBPixel<COMPONENT> YCbCrToRGB(const YCbCrPixel<COMPONENT>& pixel) throw() {
  typedef typename PixelTraits<YCbCrPixel<COMPONENT> >::Arithmetic Arithmetic;
  RGBPixel<COMPONENT> result;
   // map components into range [0; 1]
  result.red = static_cast<Arithmetic>(pixel.luma) + 2 * (1.000-0.299) * static_cast<Arithmetic>(pixel.cr);
  result.green = static_cast<Arithmetic>(pixel.luma) + (-0.114/0.587 * 2 * (1.000-0.114)) * static_cast<Arithmetic>(pixel.cb) + (-0.299/0.587 * 2 * (1.000-0.299)) * static_cast<Arithmetic>(pixel.cr);
  result.blue = static_cast<Arithmetic>(pixel.luma) + 2 * (1.000-0.114) * static_cast<Arithmetic>(pixel.cb);
  return result;
}

/**
  Converts an Y'CbCr pixel into the RGB color space. The components will be mapped from the range [0; 255] into [0; 255].
*/
template<>
inline RGBPixel<unsigned char> YCbCrToRGB(const YCbCrPixel<unsigned char>& pixel) throw() {
  typedef PixelTraits<YCbCrPixel<unsigned char> >::Arithmetic Arithmetic;
  RGBPixel<unsigned char> result;
  Arithmetic temp;
  temp = static_cast<Arithmetic>(pixel.luma) + (2*(1000-299) * static_cast<Arithmetic>(pixel.cr) - (1000-299)*255 + 1000/2)/1000; // round to nearest
  result.red = (temp >= 0) ? ((temp <= 255) ? temp : 255) : 0;
  temp = static_cast<Arithmetic>(pixel.luma) +
    (
      587 * 1000/2 + (114 * (1000-114)) * 255 + (299 * (1000-299)) * 255
      - (114 * (1000-114)) * 2 * static_cast<Arithmetic>(pixel.cb)
      - (299 * (1000-299)) * 2 * static_cast<Arithmetic>(pixel.cr)
    )/(587*1000); // round to nearest
  result.green = (temp >= 0) ? ((temp <= 255) ? temp : 255) : 0;
  temp = static_cast<Arithmetic>(pixel.luma) + (2*(1000-114) * static_cast<Arithmetic>(pixel.cb) - (1000-114)*255 + 1000/2)/1000; // round to nearest
  result.blue = (temp >= 0) ? ((temp <= 255) ? temp : 255) : 0;
  return result;
}

/** Writes the specified Y'CbCR color space pixel to the format stream using the format '(luma, cb, cr)'. */
template<class COMPONENT>
FormatOutputStream& operator<<(FormatOutputStream& stream, const YCbCrPixel<COMPONENT>& value) throw(IOException) {
  FormatOutputStream::PushContext pushContext(stream); // make current context the default context
  return stream << '(' << value.luma << ',' << value.cb << ',' << value.cr << ')';
}

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::YCbCrPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace
