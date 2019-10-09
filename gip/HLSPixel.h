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
  Pixel specified by hue, lightness, and saturation (HLS).

  @short HLS color space pixel
  @see HSVPixel
  @ingroup pixels
  @version 1.0
*/

template<class COMPONENT>
struct HLSPixel {

  /** Hue component. */
  COMPONENT hue;
  /** Lightness. */
  COMPONENT lightness;
  /** Saturation component which represents where the color is on a scale from achromatic white to the pure hue. */
  COMPONENT saturation;
};

template<class COMPONENT>
class PixelTraits<HLSPixel<COMPONENT> > {
public:

  typedef COMPONENT Component;
  typedef HLSPixel<Component> Pixel;
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
struct HLSPixel<unsigned char> {
  union {
    unsigned int hls;
    struct {
      unsigned char hue;
      unsigned char lightness;
      unsigned char saturation;
    } _DK_SDU_MIP__BASE__PACKED;
  } _DK_SDU_MIP__BASE__PACKED;
} _DK_SDU_MIP__BASE__PACKED;
_DK_SDU_MIP__BASE__PACKED__END

/**
  This function returns the HLS pixel corresponding to the specified component values.

  @param hue The hue.
  @param lightness The lightness.
  @param saturation The saturation.

  @short Creates an HLS pixel
  @see HLSPixel
*/
template<class COMPONENT>
inline HLSPixel<COMPONENT> makeHLSPixel(COMPONENT hue, COMPONENT lightness, COMPONENT saturation) throw() {
  HLSPixel<COMPONENT> result;
  result.hue = hue;
  result.lightness = lightness;
  result.saturation = saturation;
  return result;
}

/**
  Converts an RGB pixel into the HLS color space. The components of the RGB pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline HLSPixel<COMPONENT> RGBToHLS(const RGBPixel<COMPONENT>& pixel) throw() {
  typedef typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic Arithmetic;
  HLSPixel<COMPONENT> result;

  COMPONENT max = maximum(pixel.red, pixel.green, pixel.blue);
  COMPONENT min = minimum(pixel.red, pixel.green, pixel.blue);
  typename PixelTraits<HLSPixel<COMPONENT> >::Arithmetic sum = max + min;
  typename PixelTraits<HLSPixel<COMPONENT> >::Arithmetic diff = max - min;

  result.lightness = sum/2;

  if (diff == 0) { // TAG: problem if COMPONENT is floating-point type
    result.saturation = 0;
    result.hue = 0; // undefined
    return result;
  }

  if (sum <= 1) { // lightness <= 0.5
    result.saturation = diff/sum;
  } else {
    result.saturation = diff/(2 - sum);
  }

  if (pixel.red == max) {
    result.hue = (0 + (static_cast<Arithmetic>(pixel.green) - static_cast<Arithmetic>(pixel.blue))/diff)/6;
  } else if (pixel.green == max) {
    result.hue = (2 + (static_cast<Arithmetic>(pixel.blue) - static_cast<Arithmetic>(pixel.red))/diff)/6;
  } else { // pixel.blue == max
    result.hue = (4 + (static_cast<Arithmetic>(pixel.red) - static_cast<Arithmetic>(pixel.green))/diff)/6;
  }
  if (result.hue < 0) {
    result.hue += 1;
  }
  return result;
}

/**
  Converts an RGB pixel into the HLS color space. The components will be mapped from the range [0; 255] into [0; 255].
*/
template<>
inline HLSPixel<unsigned char> RGBToHLS<unsigned char>(const RGBPixel<unsigned char>& pixel) throw() {
  typedef PixelTraits<RGBPixel<unsigned char> >::Arithmetic Arithmetic;
  HLSPixel<unsigned char> result;

  unsigned char max = maximum(pixel.red, pixel.green, pixel.blue);
  unsigned char min = minimum(pixel.red, pixel.green, pixel.blue);
  PixelTraits<HLSPixel<unsigned char> >::Arithmetic sum = max + min;
  PixelTraits<HLSPixel<unsigned char> >::Arithmetic diff = max - min;

  result.lightness = (sum + 1)/2; // round to nearest

  if (diff == 0) {
    result.saturation = 0;
    result.hue = 0; // undefined
    return result;
  }

  if (sum <= 255) { // lightness <= 0.5
    result.saturation = (2 * 255 * diff + sum)/(2 * sum); // round to nearest
  } else {
    result.saturation = (2 * 255 * diff + (2 * 255 - sum))/(2 * (2 * 255 - sum)); // round to nearest
  }

  Arithmetic temp;
  if (pixel.red == max) {
    if (static_cast<Arithmetic>(pixel.green) - static_cast<Arithmetic>(pixel.blue) >= 0) {
      temp = ((6/2 * 255 * 0) * diff + 255 * (static_cast<Arithmetic>(pixel.green) - static_cast<Arithmetic>(pixel.blue)))/(6 * diff);
    } else {
      temp = ((6/2 + 255 * 6) * diff + 255 * (static_cast<Arithmetic>(pixel.green) - static_cast<Arithmetic>(pixel.blue)))/(6 * diff);
    }
  } else if (pixel.green == max) {
    temp = ((6/2 + 255 * 2) * diff + 255 * (static_cast<Arithmetic>(pixel.blue) - static_cast<Arithmetic>(pixel.red)))/(6 * diff);
  } else { // pixel.blue == max
    temp = ((6/2 + 255 * 4) * diff + 255 * (static_cast<Arithmetic>(pixel.red) - static_cast<Arithmetic>(pixel.green)))/(6 * diff);
  }
  ASSERT((temp >= 0) && (temp <= 255));
  result.hue = temp; // 0 <= temp <= 255
  return result;
}

/**
  Converts an HLS pixel into the RGB color space. The components of the HLS pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline RGBPixel<COMPONENT> HLSToRGB(const HLSPixel<COMPONENT>& pixel) throw() {
  typedef typename PixelTraits<HLSPixel<COMPONENT> >::Arithmetic Arithmetic;
  RGBPixel<COMPONENT> result;

  if (pixel.saturation == 0) { // TAG: problem if COMPONENT is floating-point type
    result.red = pixel.lightness;
    result.green = pixel.lightness;
    result.blue = pixel.lightness;
    return result;
  }

  Arithmetic value;
  if (2 * pixel.lightness <= 1) { // lightness <= 0.5
    value = pixel.lightness * (1 + pixel.saturation);
  } else {
    value = pixel.lightness + (1 - pixel.lightness) * pixel.saturation;
  }

  Arithmetic temp = static_cast<Arithmetic>(pixel.hue) * 360/60;
  int section = static_cast<int>(temp); // floor // TAG: problem if hue = 1 => section = 6 and fraction = 0
  Arithmetic m = 2 * pixel.lightness - value;

  switch (section) {
  case 0:
    result.red = value;
    result.green = m + (value - m) * (temp - section);
    result.blue = m;
    break;
  case 1:
    result.red = value - (value - m) * (temp - section);
    result.green = value;
    result.blue = m;
    break;
  case 2:
    result.red = m;
    result.green = value;
    result.blue = m + (value - m) * (temp - section);
    break;
  case 3:
    result.red = m;
    result.green = value - (value - m) * (temp - section);
    result.blue = value;
    break;
  case 4:
    result.red = m + (value - m) * (temp - section);
    result.green = m;
    result.blue = value;
    break;
  default: // both 5 and 6
    result.red = value;
    result.green = m;
    result.blue = value - (value - m) * (temp - section);
    break;
  }
  return result;
}

/**
  Converts an HLS pixel into the RGB color space. The components of the HLS pixel must be in the range [0; 255].
*/
template<>
inline RGBPixel<unsigned char> HLSToRGB(const HLSPixel<unsigned char>& pixel) throw() {
  typedef PixelTraits<HLSPixel<unsigned char> >::Arithmetic Arithmetic;
  RGBPixel<unsigned char> result;

  if (pixel.saturation == 0) {
    result.red = pixel.lightness;
    result.green = pixel.lightness;
    result.blue = pixel.lightness;
    return result;
  }

  Arithmetic value; // scaled with 2 * 255^2
  if (static_cast<Arithmetic>(pixel.lightness) <= 255/2) { // lightness <= 0.5
    value = 2 * static_cast<Arithmetic>(pixel.lightness) * (255 + static_cast<Arithmetic>(pixel.saturation));
  } else {
    value = 2 * (255 * static_cast<Arithmetic>(pixel.lightness) + (255 - static_cast<Arithmetic>(pixel.lightness)) * static_cast<Arithmetic>(pixel.saturation));
  }

  const Arithmetic temp = static_cast<Arithmetic>(pixel.hue) * 360/60;
  const int section = temp/255; // floor // TAG: problem if hue = 255 => section = 6 and fraction = 0
  const Arithmetic fraction = temp - section * 255; // scaled with 255
  const Arithmetic m = 2 * 2 * 255 * static_cast<Arithmetic>(pixel.lightness) - value; // scaled with 2 * 255^2

  switch (section) {
  case 0:
    result.red = (value + 255)/(2*255);
    result.green = (m * (255 - fraction) + value * fraction + 255*255)/(2*255*255); // scaled with 2 * 255^3
    result.blue = (m + 255)/(2*255);
    break;
  case 1:
    result.red = (value * (255 - fraction) + m * fraction + 255*255)/(2*255*255); // scaled with 2 * 255^3
    result.green = (value + 255)/(2*255);
    result.blue = (m + 255)/(2*255);
    break;
  case 2:
    result.red = (m + 255)/(2*255);
    result.green = (value + 255)/(2*255);
    result.blue = (m * (255 - fraction) + value * fraction + 255*255)/(2*255*255); // scaled with 2 * 255^3
    break;
  case 3:
    result.red = (m + 255)/(2*255);
    result.green = (value * (255 - fraction) + m * fraction + 255*255)/(2*255*255); // scaled with 2 * 255^3
    result.blue = (value + 255)/(2*255);
    break;
  case 4:
    result.red = (m * (255 - fraction) + value * fraction + 255*255)/(2*255*255); // scaled with 2 * 255^3
    result.green = (m + 255)/(2*255);
    result.blue = (value + 255)/(2*255);
    break;
  default: // both 5 and 6
    result.red = (value + 255)/(2*255);
    result.green = (m + 255)/(2*255);
    result.blue = (value * (255 - fraction) + m * fraction + 255*255)/(2*255*255); // scaled with 2 * 255^3
    break;
  }
  return result;
}

/** Writes the specified HLS color space pixel to the format stream using the format '(hue, lightness, saturation)'. */
template<class COMPONENT>
FormatOutputStream& operator<<(FormatOutputStream& stream, const HLSPixel<COMPONENT>& value) throw(IOException) {
  FormatOutputStream::PushContext pushContext(stream); // make current context the default context
  return stream << '(' << value.hue << ',' << value.lightness << ',' << value.saturation << ')';
}

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::HLSPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace
