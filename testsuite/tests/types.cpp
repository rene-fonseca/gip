/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <base/string/FormatOutputStream.h>
#include <gip/Pixel.h>

using namespace base;
using namespace gip;

int main() {
  GrayPixel grayArray[1024];
  GrayAlphaPixel8 grayAlpha8Array[1024];
  GrayAlphaPixel16 grayAlpha16Array[1024];
  ColorPixel colorArray[1024];
  ColorAlphaPixel colorAlphaArray[1024];
  float floatArray[1024];
  double doubleArray[1024];
  long double longDoubleArray[1024];
  Complex complexArray[1024];

  fout << MESSAGE("Pixel Type Information:") << EOL
       << MESSAGE("  GrayPixel: single=") << sizeof(GrayPixel) << MESSAGE(" array=") << sizeof(grayArray) << EOL
       << MESSAGE("  GrayAlphaPixel8: single=") << sizeof(GrayAlphaPixel8) << MESSAGE(" array=") << sizeof(grayAlpha8Array) << EOL
       << MESSAGE("  GrayAlphaPixel16: single=") << sizeof(GrayAlphaPixel16) << MESSAGE(" array=") << sizeof(grayAlpha16Array) << EOL
       << MESSAGE("  ColorPixel: single=") << sizeof(ColorPixel) << MESSAGE(" array=") << sizeof(colorArray) << EOL
       << MESSAGE("  ColorAlphaPixel: single=") << sizeof(ColorAlphaPixel) << MESSAGE(" array=") << sizeof(colorAlphaArray) << EOL
       << MESSAGE("  float: single=") << sizeof(float) << MESSAGE(" array=") << sizeof(floatArray) << EOL
       << MESSAGE("  double: single=") << sizeof(double) << MESSAGE(" array=") << sizeof(doubleArray) << EOL
       << MESSAGE("  long double: single=") << sizeof(long double) << MESSAGE(" array=") << sizeof(longDoubleArray) << EOL
       << MESSAGE("  Complex: single=") << sizeof(Complex) << MESSAGE(" array=") << sizeof(complexArray) << EOL;
  return 0;
}
