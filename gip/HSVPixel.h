/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__HSV_PIXEL_H
#define _DK_SDU_MIP__GIP__HSV_PIXEL_H

#include <gip/PixelTraits.h>
#include <gip/RGBPixel.h>

namespace gip {

/**
  Pixel specified by hue, saturation, and value.

  @short HSV color space pixel
  @see HLSPixel
  @ingroup pixels
  @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
  @version 1.0
*/

template<class COMPONENT>
struct HSVPixel {

  /** Hue component. */
  COMPONENT hue;
  /** Saturation component which represents where the color is on a scale from achromatic white to the pure hue. */
  COMPONENT saturation;
  /** Value. */
  COMPONENT value;
};

template<class COMPONENT>
class PixelTraits<HSVPixel<COMPONENT> > {
public:

  typedef COMPONENT Component;
  typedef HSVPixel<Component> Pixel;
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

template<>
struct HSVPixel<unsigned char> {
  union {
    unsigned int hsv;
    struct {
      unsigned char hue;
      unsigned char saturation;
      unsigned char value;
    } __attribute__ ((packed));
  } __attribute__ ((packed));
} __attribute__ ((packed));

/**
  This function returns the HSV pixel corresponding to the specified component values.

  @param hue The hue.
  @param saturation The saturation.
  @param value The value.

  @short Creates an HSV pixel
  @see HSVPixel
*/
template<class COMPONENT>
inline HSVPixel<COMPONENT> makeHSVPixel(COMPONENT hue, COMPONENT saturation, COMPONENT value) throw() {
  HSVPixel<COMPONENT> result;
  result.hue = hue;
  result.saturation = saturation;
  result.value = value;
  return result;
}

/**
  Converts an RGB pixel into the HSV color space. The components of the RGB pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline HSVPixel<COMPONENT> RGBToHSV(const RGBPixel<COMPONENT>& pixel) throw() {
  typedef typename PixelTraits<RGBPixel<COMPONENT> >::Arithmetic Arithmetic;
  HSVPixel<COMPONENT> result;

  COMPONENT max = maximum(pixel.red, pixel.green, pixel.blue);
  COMPONENT min = minimum(pixel.red, pixel.green, pixel.blue);
  typename PixelTraits<HSVPixel<COMPONENT> >::Arithmetic diff = max - min;

  result.value = max;
  if (max == 0) { // TAG: problem if COMPONENT is floating-point type
    result.saturation = 0;
  } else {
    result.saturation = diff/max;
  }
  if (result.saturation == 0) {
    result.hue = 0; // undefined
    return result;
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
  Converts an RGB pixel into the HSV color space. The components will be mapped from the range [0; 255] into [0; 255].
*/
template<>
inline HSVPixel<unsigned char> RGBToHSV<unsigned char>(const RGBPixel<unsigned char>& pixel) throw() {
  typedef PixelTraits<RGBPixel<unsigned char> >::Arithmetic Arithmetic;
  HSVPixel<unsigned char> result;

  unsigned char max = maximum(pixel.red, pixel.green, pixel.blue);
  unsigned char min = minimum(pixel.red, pixel.green, pixel.blue);
  PixelTraits<HSVPixel<unsigned char> >::Arithmetic diff = max - min; // >= 0

  result.value = max;
  if (max == 0) {
    result.saturation = 0;
  } else {
    result.saturation = (2 * 255 * diff + max)/(2 * max); // round to nearest
  }
  if (result.saturation == 0) {
    result.hue = 0; // undefined
    return result;
  }

  PixelTraits<HSVPixel<unsigned char> >::Arithmetic temp;
  if (pixel.red == max) {
    if (static_cast<Arithmetic>(pixel.green) - static_cast<Arithmetic>(pixel.blue) >= 0) {
      temp = ((6/2 + 255 * 0) * diff + 255 * (static_cast<Arithmetic>(pixel.green) - static_cast<Arithmetic>(pixel.blue)))/(6 * diff);
    } else {
      temp = ((6/2 + 255 * 6) * diff + 255 * (static_cast<Arithmetic>(pixel.green) - static_cast<Arithmetic>(pixel.blue)))/(6 * diff);
    }
  } else if (pixel.green == max) {
    temp = ((6/2 + 255 * 2) * diff + 255 * (static_cast<Arithmetic>(pixel.blue) - static_cast<Arithmetic>(pixel.red)))/(6 * diff);
  } else { // pixel.blue == max
    temp = ((6/2 + 255 * 4) * diff + 255 * (static_cast<Arithmetic>(pixel.red) - static_cast<Arithmetic>(pixel.green)))/(6 * diff);
  }
  ASSERT((temp >= 0) && (temp <= 255));
  result.hue = temp;
  return result;
}

/**
  Converts an HSV pixel into the RGB color space. The components of the HSV pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline RGBPixel<COMPONENT> HSVToRGB(const HSVPixel<COMPONENT>& pixel) throw() {
  typedef typename PixelTraits<HSVPixel<COMPONENT> >::Arithmetic Arithmetic;
  RGBPixel<COMPONENT> result;

  if (pixel.saturation == 0) { // TAG: problem if COMPONENT is floating-point type
    result.red = pixel.value;
    result.green = pixel.value;
    result.blue = pixel.value;
    return result;
  }

  Arithmetic temp = static_cast<Arithmetic>(pixel.hue) * 360/60;
  const int section = static_cast<int>(temp); // floor // TAG: problem if hue = 1 => section = 6 and fraction = 0
  const Arithmetic fraction = temp - section;

  switch (section) {
  case 0:
    result.red = pixel.value;
    result.green = pixel.value * (1.0 - (pixel.saturation * (1.0 - fraction)));
    result.blue = pixel.value * (1.0 - pixel.saturation);
    break;
  case 1:
    result.red = pixel.value * (1.0 - pixel.saturation * fraction);
    result.green = pixel.value;
    result.blue = pixel.value * (1.0 - pixel.saturation);
    break;
  case 2:
    result.red = pixel.value * (1.0 - pixel.saturation);
    result.green = pixel.value;
    result.blue = pixel.value * (1.0 - (pixel.saturation * (1.0 - fraction)));
    break;
  case 3:
    result.red = pixel.value * (1.0 - pixel.saturation);
    result.green = pixel.value * (1.0 - pixel.saturation * fraction);
    result.blue = pixel.value;
    break;
  case 4:
    result.red = pixel.value * (1.0 - (pixel.saturation * (1.0 - fraction)));
    result.green = pixel.value * (1.0 - pixel.saturation);
    result.blue = pixel.value;
    break;
  default: // both 5 and 6
    result.red = pixel.value;
    result.green = pixel.value * (1.0 - pixel.saturation);
    result.blue = pixel.value * (1.0 - pixel.saturation * fraction);
    break;
  }
  return result;
}

/**
  Converts an HSV pixel into the RGB color space. The components of the HSV pixel must be in the range [0; 255].
*/
template<>
inline RGBPixel<unsigned char> HSVToRGB(const HSVPixel<unsigned char>& pixel) throw() {
  typedef PixelTraits<HSVPixel<unsigned char> >::Arithmetic Arithmetic;
  RGBPixel<unsigned char> result;

  if (pixel.saturation == 0) {
    result.red = pixel.value;
    result.green = pixel.value;
    result.blue = pixel.value;
    return result;
  }

  Arithmetic temp = static_cast<Arithmetic>(pixel.hue) * 360/60;
  const int section = temp/255; // floor // TAG: problem if hue = 255 => section = 6 and fraction = 0
  const Arithmetic fraction = temp - section * 255; // scaled with 255

  switch (section) {
  case 0:
    result.red = pixel.value;
    result.green = (255*255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 * 255 - (pixel.saturation * (255 - fraction))))/(2 * 255 * 255);
    result.blue = (255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 - pixel.saturation))/(2 * 255);
    break;
  case 1:
    result.red = (255*255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 * 255 - pixel.saturation * fraction))/(2 * 255 * 255);
    result.green = pixel.value;
    result.blue = (255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 - pixel.saturation))/(2 * 255);
    break;
  case 2:
    result.red = (255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 - pixel.saturation))/(2 * 255);
    result.green = pixel.value;
    result.blue = (255*255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 * 255 - (pixel.saturation * (255 - fraction))))/(2 * 255 * 255);
    break;
  case 3:
    result.red = (255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 - pixel.saturation))/(2 * 255);
    result.green = (255*255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 * 255 - pixel.saturation * fraction))/(2 * 255 * 255);
    result.blue = pixel.value;
    break;
  case 4:
    result.red = (255*255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 * 255 - (pixel.saturation * (255 - fraction))))/(2 * 255 * 255);
    result.green = (255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 - pixel.saturation))/(2 * 255);
    result.blue = pixel.value;
    break;
  default: // both 5 and 6
    result.red = pixel.value;
    result.green = (255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 - pixel.saturation))/(2 * 255);
    result.blue = (255*255 + 2 * static_cast<Arithmetic>(pixel.value) * (255 * 255 - pixel.saturation * fraction))/(2 * 255 * 255);
    break;
  }
  return result;
}

/** Writes the specified HSV color space pixel to the format stream using the format '(hue, saturation, value)'. */
template<class COMPONENT>
FormatOutputStream& operator<<(FormatOutputStream& stream, const HSVPixel<COMPONENT>& value) throw(IOException) {
  FormatOutputStream::PushContext pushContext(stream); // make current context the default context
  return stream << '(' << value.hue << ',' << value.saturation << ',' << value.value << ')';
}

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::HSVPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace

#endif
