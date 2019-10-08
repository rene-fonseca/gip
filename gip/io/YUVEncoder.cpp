/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/YUVEncoder.h>
#include <base/io/FileInputStream.h>
#include <base/io/FileOutputStream.h>
#include <base/string/FormatOutputStream.h>
#include <base/NotSupported.h>

namespace gip {

  YUVEncoder::YUVEncoder() throw() {
  }

  String YUVEncoder::getDescription() const throw() {
    return MESSAGE("Abekas YUV");
  }
  
  String YUVEncoder::getDefaultExtension() const throw() {
    return MESSAGE("yuv");
  }
  
  bool YUVEncoder::isValid(const String& filename) throw(IOException) {
    return true;
  }
  
  ColorImage* YUVEncoder::read(const String& filename) throw(InvalidFormat, IOException) {
//    for (y = 0; y < height/2; ++y) {
//      for (x = 0; x < width/2; ++x) {
//        U[y][x] = orig_cb[y][x] - 128;
//        V[y][x] = orig_cr[y][x] - 128;
//      }
//    }
//
//    for (y = 0; y < height; ++y) {
//      for (x = 0; x < width; ++x) {
//        Y[y][x] = orig_y[y][x] - 16;
//      }
//    }
//
//    for (y = 0; y < height; ++y) {
//      for (x = 0; x < width; ++x) {
//
//        tempR = 104635 * V[y/2][x/2];
//        tempG = -25690 * U[y/2][x/2] + -53294 * V[y/2][x/2];
//        tempB = 132278 * U[y/2][x/2];
//
//        tempR += (Y[y][x] * 76310);
//        tempG += (Y[y][x] * 76310);
//        tempB += (Y[y][x] * 76310);
//
//        makeRGBPixel(
//          tempR >> 16, // clamp to 0-255
//          tempG >> 16, // clamp to 0-255
//          tempB >> 16 // clamp to 0-255
//       );
//      }
//    }
    throw InvalidFormat(this); // TAG: fixme
    return 0;
  }
  
  void YUVEncoder::write(const String& filename, const ColorImage* image) throw(ImageException, IOException) {
    bassert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    
//    FileOutputStream file(filename, FileOutputStream::CREATE | FileOutputStream::TRUNCATE);
  }
  
  void YUVEncoder::writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException) {
    throw NotSupported(this);
  }
  
  HashTable<String, AnyValue> YUVEncoder::getInformation(const String& filename) throw(IOException) {
    HashTable<String, AnyValue> result;
    result[MESSAGE("encoder")] = Type::getType(*this);
    result[MESSAGE("description")] = MESSAGE("Abekas YUV");
    result[MESSAGE("width")] = static_cast<unsigned int>(WIDTH);
    result[MESSAGE("height")] = static_cast<unsigned int>(HEIGHT);
    return result;
  }

}; // end of gip namespace
