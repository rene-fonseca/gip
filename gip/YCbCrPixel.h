/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__YUV_PIXEL_H
#define _DK_SDU_MIP__GIP__YUV_PIXEL_H

#include <gip/PixelTraits.h>

namespace gip {

/**
  Y'CbCr color space pixel.

  @short Y'CbCr color space pixel
  @ingroup pixels
  @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
  @version 1.0
*/

template<class COMPONENT>
class YCbCrPixel {

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
  typedef PixelComponent<Component>::Arithmetic Arithmetic;

  enum {
    MINIMUM = 0x00,
    MAXIMUM = 0xff // TAG: should be 1.0 if floating-point type
  };
};

template<>
struct YCbCrPixel<unsigned char> {
  union {
    unsigned int ycbcr;
    struct {
      unsigned char luma;
      unsigned char cb;
      unsigned char cr;
    } __attribute__ ((packed));
  } __attribute__ ((packed));
} __attribute__ ((packed));

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
template<COMPONENT>
inline YCbCrPixel RGBToCbCr<RGBPixel<COMPONENT> >(const RGBPixel<COMPONENT>& pixel) throw() {
  YCbCrPixel result;
  typedef PixelTraits<ColorPixel>::Artihmetic Arithmetic;

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
inline YCbCrPixel<unsigned char> RGBToCbCr<RGBPixel<unsigned char> >(const RGBPixel<unsigned char>& pixel) throw() {
  YCbCrPixel<unsigned char> result;
  typedef PixelTraits<RGBPixel<unsigned char> >::Artihmetic Arithmetic;
  // we know that overflow isn't possible: (299 + 587 + 114) * 255 * 255 <= PrimitiveTraits<PixelTraits<ColorPixel>::Arithmetic>::MAXIMUM
  // see ITU-R recommendation BT (map into range [0; 255])
  Arithmetic temp = 299 * static_cast<Arithmetic>(pixel.red) + 587 * static_cast<Arithmetic>(pixel.green) + 114 * static_cast<Arithmetic>(pixel.blue);
  result.luma = temp/1000;
  result.cb = (1000 * static_cast<Arithmetic>(pixel.blue) - temp) * 255/(2 * 1000 * (1000-114)) + 128;
  result.cr = (1000 * static_cast<Arithmetic>(pixel.red) - temp) * 255/(2 * 1000 * (1000-299)) + 128;
  return result;
}

/**
  Converts an Y'CbCr pixel into the RGB color space. The components of the Y'CbCr pixel must be in the range [0; 1].
*/
template<class COMPONENT>
inline RGBPixel<COMPONENT> YCbCrToRGB(const YCbCrPixel<COMPONENT>& pixel) throw() {
  RGBPixel<COMPONENT> result;
  typedef PixelTraits<YCbCrPixel<COMPONENT> >::Arithmetic Arithmetic;
   // map components into range [0; 1]
  result.red = static_cast<Arithmetic>(pixel.luma) + 2 * (1.000-0.299) * static_cast<Arithmetic>(pixel.cr);
  result.green = static_cast<Arithmetic>(pixel.luma) + (-0.114/0.587 * 2 * (1.000-0.114)) * static_cast<Arithmetic>(pixel.cb) + (-0.299/0.587 * 2 * (1.000-0.299)) * static_cast<Arithmetic>(pixel.cr);
  result.blue = static_cast<Arithmetic>(pixel.luma) + 2 * (1.000-0.114) * static_cast<Arithmetic>(pixel.cb);
  return result;
}

}; // end of gip namespace

namespace base {

  template<class COMPONENT>
  class Relocateable<gip::YCbCrPixel<COMPONENT> > {
  public:
    enum {IS_RELOCATEABLE = Relocateable<COMPONENT>::IS_RELOCATEABLE};
  };

}; // end of base namespace

#endif
