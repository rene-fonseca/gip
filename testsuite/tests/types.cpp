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
#include <base/string/String.h>
#include <gip/Pixel.h>
#include <gip/YCbCrPixel.h>
#include <gip/HLSPixel.h>
#include <gip/HSVPixel.h>

using namespace base;
using namespace gip;

void writeHLSTest(const String& prefix, const RGBPixel<long double>& rgb) throw(IOException) {
  HLSPixel<long double> hls = RGBToHLS(rgb);
  fout << prefix << ": " << rgb << " HLS:" << setPrecision(3) << hls << " HLS->RGB:" << setPrecision(1) << HLSToRGB(hls) << EOL;
}

void writeHLSTest(const String& prefix, const RGBPixel<unsigned char>& rgb) throw(IOException) {
  HLSPixel<unsigned char> hls = RGBToHLS(rgb);
  fout << prefix << ": " << rgb << " HLS:" << hls << " HLS->RGB:" << HLSToRGB(hls) << EOL;
}

void writeHSVTest(const String& prefix, const RGBPixel<long double>& rgb) throw(IOException) {
  HSVPixel<long double> hsv = RGBToHSV(rgb);
  fout << prefix << ": " << rgb << " HSV:" << setPrecision(3) << hsv << " HSV->RGB:" << setPrecision(1) << HSVToRGB(hsv) << EOL;
}

void writeHSVTest(const String& prefix, const RGBPixel<unsigned char>& rgb) throw(IOException) {
  HSVPixel<unsigned char> hsv = RGBToHSV(rgb);
  fout << prefix << ": " << rgb << " HSV:" << hsv << " HSV->RGB:" << HSVToRGB(hsv) << EOL;
}

void writeYCbCrTest(const String& prefix, const RGBPixel<long double>& rgb) throw(IOException) {
  YCbCrPixel<long double> ycbcr = RGBToYCbCr(rgb);
  fout << prefix << ": " << rgb << " Y'CbCr:" << setPrecision(3) << ycbcr << " Y'CbCr->RGB:" << setPrecision(3) << YCbCrToRGB(ycbcr) << EOL;
}

void writeYCbCrTest(const String& prefix, const RGBPixel<unsigned char>& rgb) throw(IOException) {
  YCbCrPixel<unsigned char> ycbcr = RGBToYCbCr(rgb);
  fout << prefix << ": " << rgb << " Y'CbCr:" << ycbcr << " Y'CbCr->RGB:" << YCbCrToRGB(ycbcr) << EOL;
}

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
  YCbCrPixel<unsigned char> YCbCrArray[1024];
  HLSPixel<unsigned char> HLSArray[1024];

  fout << MESSAGE("Pixel Type Information:") << EOL
       << MESSAGE("  GrayPixel: single=") << sizeof(GrayPixel) << MESSAGE(" array=") << sizeof(grayArray) << EOL
       << MESSAGE("  GrayAlphaPixel8: single=") << sizeof(GrayAlphaPixel8) << MESSAGE(" array=") << sizeof(grayAlpha8Array) << EOL
       << MESSAGE("  GrayAlphaPixel16: single=") << sizeof(GrayAlphaPixel16) << MESSAGE(" array=") << sizeof(grayAlpha16Array) << EOL
       << MESSAGE("  ColorPixel: single=") << sizeof(ColorPixel) << MESSAGE(" array=") << sizeof(colorArray) << EOL
       << MESSAGE("  ColorAlphaPixel: single=") << sizeof(ColorAlphaPixel) << MESSAGE(" array=") << sizeof(colorAlphaArray) << EOL
       << MESSAGE("  float: single=") << sizeof(float) << MESSAGE(" array=") << sizeof(floatArray) << EOL
       << MESSAGE("  double: single=") << sizeof(double) << MESSAGE(" array=") << sizeof(doubleArray) << EOL
       << MESSAGE("  long double: single=") << sizeof(long double) << MESSAGE(" array=") << sizeof(longDoubleArray) << EOL
       << MESSAGE("  Complex: single=") << sizeof(Complex) << MESSAGE(" array=") << sizeof(complexArray) << EOL
       << MESSAGE("  YCbCrPixel<unsigned char>: single=") << sizeof(YCbCrPixel<unsigned char>) << MESSAGE(" array=") << sizeof(YCbCrArray) << EOL
       << MESSAGE("  HLSPixel<unsigned char>: single=") << sizeof(HLSPixel<unsigned char>) << MESSAGE(" array=") << sizeof(HLSArray) << EOL;

  fout << EOL << "HLSPixel<long double>: " << EOL;
  writeHLSTest(MESSAGE("red"), makeRGBPixel<long double>(1, 0, 0));
  writeHLSTest(MESSAGE("yellow"), makeRGBPixel<long double>(1, 1, 0));
  writeHLSTest(MESSAGE("green"), makeRGBPixel<long double>(0, 1, 0));
  writeHLSTest(MESSAGE("cyan"), makeRGBPixel<long double>(0, 1, 1));
  writeHLSTest(MESSAGE("blue"), makeRGBPixel<long double>(0, 0, 1));
  writeHLSTest(MESSAGE("magenta"), makeRGBPixel<long double>(1, 0, 1));
  writeHLSTest(MESSAGE("black"), makeRGBPixel<long double>(0, 0, 0));
  writeHLSTest(MESSAGE("white"), makeRGBPixel<long double>(1, 1, 1));

  fout << EOL << "HLSPixel<unsigned char>: " << EOL;
  writeHLSTest(MESSAGE("red"), makeRGBPixel<unsigned char>(255, 0, 0));
  writeHLSTest(MESSAGE("yellow"), makeRGBPixel<unsigned char>(255, 255, 0));
  writeHLSTest(MESSAGE("green"), makeRGBPixel<unsigned char>(0, 255, 0));
  writeHLSTest(MESSAGE("cyan"), makeRGBPixel<unsigned char>(0, 255, 255));
  writeHLSTest(MESSAGE("blue"), makeRGBPixel<unsigned char>(0, 0, 255));
  writeHLSTest(MESSAGE("magenta"), makeRGBPixel<unsigned char>(255, 0, 255));
  writeHLSTest(MESSAGE("black"), makeRGBPixel<unsigned char>(0, 0, 0));
  writeHLSTest(MESSAGE("white"), makeRGBPixel<unsigned char>(255, 255, 255));
  writeHLSTest(MESSAGE("special"), makeRGBPixel<unsigned char>(123, 213, 234));

  fout << EOL << "HSVPixel<long double>: " << EOL;
  writeHSVTest(MESSAGE("red"), makeRGBPixel<long double>(1, 0, 0));
  writeHSVTest(MESSAGE("yellow"), makeRGBPixel<long double>(1, 1, 0));
  writeHSVTest(MESSAGE("green"), makeRGBPixel<long double>(0, 1, 0));
  writeHSVTest(MESSAGE("cyan"), makeRGBPixel<long double>(0, 1, 1));
  writeHSVTest(MESSAGE("blue"), makeRGBPixel<long double>(0, 0, 1));
  writeHSVTest(MESSAGE("magenta"), makeRGBPixel<long double>(1, 0, 1));
  writeHSVTest(MESSAGE("black"), makeRGBPixel<long double>(0, 0, 0));
  writeHSVTest(MESSAGE("white"), makeRGBPixel<long double>(1, 1, 1));
  writeHSVTest(MESSAGE("special"), makeRGBPixel<long double>(123/255.0, 213/255.0, 234/255.0));

  fout << EOL << "HSVPixel<unsigned char>: " << EOL;
  writeHSVTest(MESSAGE("red"), makeRGBPixel<unsigned char>(255, 0, 0));
  writeHSVTest(MESSAGE("yellow"), makeRGBPixel<unsigned char>(255, 255, 0));
  writeHSVTest(MESSAGE("green"), makeRGBPixel<unsigned char>(0, 255, 0));
  writeHSVTest(MESSAGE("cyan"), makeRGBPixel<unsigned char>(0, 255, 255));
  writeHSVTest(MESSAGE("blue"), makeRGBPixel<unsigned char>(0, 0, 255));
  writeHSVTest(MESSAGE("magenta"), makeRGBPixel<unsigned char>(255, 0, 255));
  writeHSVTest(MESSAGE("black"), makeRGBPixel<unsigned char>(0, 0, 0));
  writeHSVTest(MESSAGE("white"), makeRGBPixel<unsigned char>(255, 255, 255));
  writeHSVTest(MESSAGE("special"), makeRGBPixel<unsigned char>(123, 213, 234));

  fout << EOL << "YCbCrPixel<long double>: " << EOL;
  writeYCbCrTest(MESSAGE("red"), makeRGBPixel<long double>(1, 0, 0));
  writeYCbCrTest(MESSAGE("yellow"), makeRGBPixel<long double>(1, 1, 0));
  writeYCbCrTest(MESSAGE("green"), makeRGBPixel<long double>(0, 1, 0));
  writeYCbCrTest(MESSAGE("cyan"), makeRGBPixel<long double>(0, 1, 1));
  writeYCbCrTest(MESSAGE("blue"), makeRGBPixel<long double>(0, 0, 1));
  writeYCbCrTest(MESSAGE("magenta"), makeRGBPixel<long double>(1, 0, 1));
  writeYCbCrTest(MESSAGE("black"), makeRGBPixel<long double>(0, 0, 0));
  writeYCbCrTest(MESSAGE("white"), makeRGBPixel<long double>(1, 1, 1));

  fout << EOL << "YCbCrPixel<unsigned char>: " << EOL;
  writeYCbCrTest(MESSAGE("red"), makeRGBPixel<unsigned char>(255, 0, 0));
  writeYCbCrTest(MESSAGE("yellow"), makeRGBPixel<unsigned char>(255, 255, 0));
  writeYCbCrTest(MESSAGE("green"), makeRGBPixel<unsigned char>(0, 255, 0));
  writeYCbCrTest(MESSAGE("cyan"), makeRGBPixel<unsigned char>(0, 255, 255));
  writeYCbCrTest(MESSAGE("blue"), makeRGBPixel<unsigned char>(0, 0, 255));
  writeYCbCrTest(MESSAGE("magenta"), makeRGBPixel<unsigned char>(255, 0, 255));
  writeYCbCrTest(MESSAGE("black"), makeRGBPixel<unsigned char>(0, 0, 0));
  writeYCbCrTest(MESSAGE("white"), makeRGBPixel<unsigned char>(255, 255, 255));
  writeYCbCrTest(MESSAGE("special"), makeRGBPixel<unsigned char>(123, 213, 234));

  return 0;
}
