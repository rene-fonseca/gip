/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__HSL_PIXEL_H
#define _DK_SDU_MIP__GIP__HSL_PIXEL_H

#include <gip/PixelTraits.h>
#include <gip/RGBPixel.h>

namespace gip {

/**
  HSL color space pixel.

  @short HSL color space pixel
  @ingroup pixels
  @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
  @version 1.0
*/

template<class COMPONENT>
struct HSLPixel {

  /** Hue component. */
  COMPONENT hue;
  /** Saturation component which represents where the color is on a scale from achromatic white to the pure hue. */
  COMPONENT saturation;
  /** Luminance. */
  COMPONENT luminance;
};

template<class COMPONENT>
class PixelTraits<HSLPixel<COMPONENT> > {
public:

  typedef COMPONENT Component;
  typedef HSLPixel<Component> Pixel;
  typedef PixelComponent<Component>::Arithmetic Arithmetic;

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

template<>
struct HSLPixel<unsigned char> {
  union {
    unsigned int hsl;
    struct {
      unsigned char hue;
      unsigned char saturation;
      unsigned char luminance;
    } __attribute__ ((packed));
  } __attribute__ ((packed));
} __attribute__ ((packed));

/**
  This function returns the HSL pixel corresponding to the specified component values.

  @param hue The hue.
  @param saturation The saturation.
  @param luminance The luminance.

  @short Creates an HSL pixel
  @see HSLPixel
*/
template<class COMPONENT>
inline HSLPixel<COMPONENT> makeHSLPixel(COMPONENT hue, COMPONENT saturation, COMPONENT luminance) throw() {
  HSLPixel<COMPONENT> result;
  result.hue = hue;
  result.saturation = saturation;
  result.luminance = luminance;
  return result;
}

/**
  Converts an RGB pixel into the HSL color space. The components of the RGB pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline HSLPixel<COMPONENT> RGBToHSL(const RGBPixel<COMPONENT>& pixel) throw() {
  HSLPixel<COMPONENT> result;
  typedef PixelTraits<RGBPixel<COMPONENT> >::Arithmetic Arithmetic;

  COMPONENT max = maximum(pixel.red, pixel.green, pixel.blue);
  COMPONENT min = minimum(pixel.red, pixel.green, pixel.blue);
  Arithmetic sum = max + min;
  Arithmetic diff = max - min;

  result.luminance = sum/2;

  if (max == min) { // TAG: problem if COMPONENT is floating-point type
    result.saturation = 0;
    result.hue = 0; // undefined
    return result;
  }

  if (result.luminance <= 0.5) {
    result.saturation = diff/sum;
  } else {
    result.saturation = diff/(2 - sum);
  }

  if (pixel.red == max) {
    result.hue = 0.0/6.0 + (pixel.green - pixel.blue)/(6.0 * diff);
  } else if (pixel.green == max) {
    result.hue = 2.0/6.0 + (pixel.blue - pixel.red)/(6.0 * diff);
  } else { // pixel.blue == max
    result.hue = 4.0/6.0 + (pixel.red - pixel.green)/(6.0 * diff);
  }
  if (result.hue < 0) {
    result.hue += 1;
  }
  return result;
}

/**
  Converts an RGB pixel into the HSL color space. The components will be mapped from the range [0; 255] into [0; 255].
*/
template<>
inline HSLPixel<unsigned char> RGBToHSL<unsigned char>(const RGBPixel<unsigned char>& pixel) throw() {
  HSLPixel<unsigned char> result;
  typedef PixelTraits<RGBPixel<unsigned char> >::Arithmetic Arithmetic;

  unsigned char max = maximum(pixel.red, pixel.green, pixel.blue);
  unsigned char min = minimum(pixel.red, pixel.green, pixel.blue);
  Arithmetic sum = max + min;
  Arithmetic diff = max - min;

  result.luminance = (sum + 1)/2; // round to nearest

  if (max == min) {
    result.saturation = 0;
    result.hue = 0; // undefined
    return result;
  }

  if (result.luminance <= 127) {
    result.saturation = 255 * diff/sum;
  } else {
    result.saturation = 255 * diff/(2 * 255 - sum);
  }

  Arithmetic temp;
  if (pixel.red == max) {
    temp = 255 * 0/6 + 255 * (pixel.green - pixel.blue)/(6 * diff);
  } else if (pixel.green == max) {
    temp = 255 * 2/6 + 255 * (pixel.blue - pixel.red)/(6 * diff);
  } else { // pixel.blue == max
    temp = 255 * 4/6 + 255 * (pixel.red - pixel.green)/(6 * diff);
  }
  if (temp < 0) {
    temp += 255;
  }
  result.hue = temp;
  return result;
}

/**
  Converts an HSL pixel into the RGB color space. The components of the HSL pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline RGBPixel<COMPONENT> HSLToRGB(const HSLPixel<COMPONENT>& pixel) throw() {
  RGBPixel<COMPONENT> result;
  typedef PixelTraits<HSLPixel<COMPONENT> >::Arithmetic Arithmetic;

  if (pixel.saturation == 0) { // TAG: problem if COMPONENT is floating-point type
    result.red = pixel.luminance;
    result.green = pixel.luminance;
    result.blue = pixel.luminance;
    return result;
  }

  Arithmetic v;
  if (pixel.luminance <= 0.5) {
    v = pixel.luminance * (1 + pixel.saturation);
  } else {
    v = pixel.luminance + (1 - pixel.luminance) * pixel.saturation;
  }

  Arithmetic temp = static_cast<Arithmetic>(pixel.hue) * 360/60;
  int section = static_cast<int>(temp); // floor // TAG: problem if hue = 1 => section = 6 and fraction = 0
  Arithmetic m = 2 * pixel.luminance - v;

  switch (section) {
  case 0:
    result.red = v;
    result.green = m + (v - m) * (temp - section);
    result.blue = m;
    break;
  case 1:
    result.red = v - (v - m) * (temp - section);
    result.green = v;
    result.blue = m;
    break;
  case 2:
    result.red = m;
    result.green = v;
    result.blue = m + (v - m) * (temp - section);
    break;
  case 3:
    result.red = m;
    result.green = v - (v - m) * (temp - section);
    result.blue = v;
    break;
  case 4:
    result.red = m + (v - m) * (temp - section);
    result.green = m;
    result.blue = v;
    break;
  default: // both 5 and 6
    result.red = v;
    result.green = m;
    result.blue = v - (v - m) * (temp - section);
    break;
  }
  return result;
}

/**
  Converts an HSL pixel into the RGB color space. The components of the HSL pixel must be in the range [0; 255].
*/
template<>
inline RGBPixel<unsigned char> HSLToRGB(const HSLPixel<unsigned char>& pixel) throw() {
  RGBPixel<unsigned char> result;
  typedef PixelTraits<HSLPixel<unsigned char> >::Arithmetic Arithmetic;

  if (pixel.saturation == 0) {
    result.red = pixel.luminance;
    result.green = pixel.luminance;
    result.blue = pixel.luminance;
    return result;
  }

  Arithmetic v; // scaled with 255^2
  if (static_cast<Arithmetic>(pixel.luminance) <= 127) {
    v = static_cast<Arithmetic>(pixel.luminance) * (255 + static_cast<Arithmetic>(pixel.saturation));
  } else {
    v = 255 * static_cast<Arithmetic>(pixel.luminance) + (255 - static_cast<Arithmetic>(pixel.luminance)) * static_cast<Arithmetic>(pixel.saturation);
  }

  Arithmetic temp = static_cast<Arithmetic>(pixel.hue) * 360/60;
  int section = temp/255; // floor // TAG: problem if hue = 255 => section = 6 and fraction = 0
  Arithmetic fraction = temp - section * 255;
  Arithmetic m = 2 * 255 * static_cast<Arithmetic>(pixel.luminance) - v; // scaled with 255^2

  switch (section) {
  case 0:
    result.red = (v + 255/2)/255; // round to nearest
    result.green = (m * (255 - fraction) + v * fraction + 255*255/2)/(255*255); // scaled with 255^3 - round to nearest
    result.blue = (m + 255/2)/255; // round to nearest
    break;
  case 1:
    result.red = (v * (255 - fraction) + m * fraction + 255*255/2)/(255*255); // scaled with 255^3 - round to nearest
    result.green = (v + 255/2)/255; // round to nearest
    result.blue = (m + 255/2)/255; // round to nearest
    break;
  case 2:
    result.red = (m + 255/2)/255; // round to nearest
    result.green = (v + 255/2)/255; // round to nearest
    result.blue = (m * (255 - fraction) + v * fraction + 255*255/2)/(255*255); // scaled with 255^3 - round to nearest
    break;
  case 3:
    result.red = (m + 255/2)/255; // round to nearest
    result.green = (v * (255 - fraction) + m * fraction + 255*255/2)/(255*255); // scaled with 255^3 - round to nearest
    result.blue = (v + 255/2)/255; // round to nearest
    break;
  case 4:
    result.red = (m * (255 - fraction) + v * fraction + 255*255/2)/(255*255); // scaled with 255^3 - round to nearest
    result.green = (m + 255/2)/255; // round to nearest
    result.blue = (v + 255/2)/255; // round to nearest
    break;
  default: // both 5 and 6
    result.red = (v + 255/2)/255; // round to nearest
    result.green = (m + 255/2)/255; // round to nearest
    result.blue = (v * (255 - fraction) + m * fraction + 255*255/2)/(255*255); // scaled with 255^3 - round to nearest
    break;
  }
  return result;
}

/** Writes the specified HSL color space pixel to the format stream using the format '(hue, saturation, luminance)'. */
template<class COMPONENT>
FormatOutputStream& operator<<(FormatOutputStream& stream, const HSLPixel<COMPONENT>& value) throw(IOException) {
  FormatOutputStream::PushContext pushContext(stream); // make current context the default context
  return stream << '(' << value.hue << ',' << value.saturation << ',' << value.luminance << ')';
}

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::HSLPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace

#endif
