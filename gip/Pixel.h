/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__PIXEL_H
#define _DK_SDU_MIP__GIP__PIXEL_H

#include <base/Functor.h>
#include <base/mathematics/Complex.h>

using namespace base;

namespace gip {

/**
  The default intensity used by image elements (8 bit).

  @short Image element.
*/
typedef unsigned char Intensity;

static const Intensity MINIMUM_INTENSITY = 0x00;
static const Intensity MAXIMUM_INTENSITY = 0xff;

/**
  Image element wrapper for a single intensity (e.g. gray level) with the
  intensity represented by one byte.

  @short Image element.
*/
typedef Intensity __attribute__ ((aligned (4) )) GrayPixel;

/**
  Image element wrapper for red, green, and blue intensities with each
  intensity represented by one byte.

  @short Image element.
*/
typedef union {
  unsigned int rgb;
  struct {
    Intensity blue;
    Intensity green;
    Intensity red;
  };
} __attribute__ ((packed)) RGBPixel;

inline RGBPixel makeRGBPixel(Intensity blue, Intensity green, Intensity red) throw() {
  RGBPixel result;
  result.rgb = (static_cast<unsigned int>(red) << 8 + green) << 8 + blue;
  return result;
}

static const RGBPixel RGB_PIXEL_BLACK = {0x000000};
static const RGBPixel RGB_PIXEL_WHITE = {0xffffff};

/**
  Image element wrapper for red, green, blue, and tranparency intensities with
  each intensity represented by one byte.

  @short Image element.
*/
typedef union {
  unsigned int rgbt;
  struct {
    Intensity blue;
    Intensity green;
    Intensity red;
    Intensity transparency;
  };
} __attribute__ ((packed)) RGBTPixel;

typedef long double FloatPixel;

typedef struct {
  double blue;
  double green;
  double red;
} FloatColorPixel;

typedef Complex ComplexPixel;

template<class DEST, class SRC>
inline DEST convertPixel(const SRC& value) throw() {return value;}

template<>
inline ComplexPixel convertPixel<ComplexPixel, FloatPixel>(const FloatPixel& value) throw() {
  return ComplexPixel(value);
}

template<>
inline FloatPixel convertPixel<FloatPixel, GrayPixel>(const GrayPixel& value) throw() {
  return FloatPixel(value);
}

template<>
inline GrayPixel convertPixel<GrayPixel, RGBPixel>(const RGBPixel& value) throw() {
  return (GrayPixel){((unsigned int)value.blue + value.green + value.red)/3}; // looses information
}

template<>
inline FloatPixel convertPixel<FloatPixel, ComplexPixel>(const ComplexPixel& value) throw() {
  return FloatPixel(value.getReal()); // looses information
}

template<>
inline GrayPixel convertPixel<GrayPixel, FloatPixel>(const FloatPixel& value) throw() {
  return GrayPixel(static_cast<Intensity>(value)); // looses information
}

template<>
inline RGBPixel convertPixel<RGBPixel, GrayPixel>(const GrayPixel& value) throw() {
  RGBPixel result;
  result.blue = value;
  result.green = value;
  result.red = value;
  return result;
}



template<>
inline void copy<RGBPixel>(RGBPixel* dest, const RGBPixel* src, unsigned int count) {
  copy<char>((char*)dest, (const char*)src, count * sizeof(RGBPixel));
}

typedef RGBPixel ColorPixel;

//inline double RGBToGray(const ColorPixel& value) throw() {
//  return ((unsigned int)value.blue + value.green + value.red)/3;
//}

inline ColorPixel GrayToRGB(Intensity value) throw() {
  ColorPixel result;
  result.blue = value;
  result.green = value;
  result.red = value;
  return result;
}



//template<class TYPE>
//inline ComplexPixel PixelToComplex(const TYPE& value) throw() {
//  return ComplexPixel(0, 0);
//}
//
//template<>
//inline ComplexPixel PixelToComplex<FloatPixel>(const FloatPixel& value) throw() {
//  return ComplexPixel(value, 0);
//}
//
//template<>
//inline ComplexPixel PixelToComplex<ColorPixel>(const ColorPixel& value) throw() {
//  return ComplexPixel(RGBToGray(value), 0);
//}


}; // end of namespace

#endif
