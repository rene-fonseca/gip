/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/PPMEncoder.h>
#include <base/io/FileInputStream.h>
#include <base/io/FileOutputStream.h>
#include <base/string/FormatOutputStream.h>
#include <base/NotSupported.h>

using namespace base;

namespace gip {

  PPMEncoder::PPMEncoder() throw() {
  }

  String PPMEncoder::getDescription() const throw() {
    return MESSAGE("Portable Pixmap Format");
  }
  
  String PPMEncoder::getDefaultExtension() const throw() {
    return MESSAGE("ppm");
  }
  
  bool PPMEncoder::isValid(const String& filename) throw(IOException) {
    return true;
  }
  
  ColorImage* PPMEncoder::read(const String& filename) throw(InvalidFormat, IOException) {
    return 0;
  }
  
  void PPMEncoder::write(const String& filename, const ColorImage* image) throw(ImageException, IOException) {
    assert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    
    FileOutputStream file(filename, FileOutputStream::CREATE | FileOutputStream::TRUNCATE);
    FormatOutputStream out(file);
    
    out << MESSAGE("P3") << EOL
        << dimension.getWidth() << ' ' << dimension.getHeight() << EOL
        << 255 << EOL;
    out << setWidth(4);
    //FormatOutputStream::Context push(out);

    const ColorPixel* end = image->getElements();
    const ColorPixel* src = end + dimension.getSize();
    unsigned int row = 0;
    while (src > end) {
      const ColorPixel* endOfRow = src;
      src -= dimension.getWidth();
      while (src < endOfRow) {
        int c = minimum(endOfRow - src, 5); // do not exceed 70 chars per line
        while (c--) {
          ColorPixel pixel = *src++;
          out << setWidth(4) << pixel.red << setWidth(4) << pixel.green << setWidth(4) << pixel.blue;
          if (c > 0) {
            out << MESSAGE("  ");
          }
        }
        out << EOL;
      }
      src -= dimension.getWidth();
    }
  }
  
  void PPMEncoder::writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException) {
    throw NotSupported(this);
  }
  
  HashTable<String, AnyValue> PPMEncoder::getInformation(const String& filename) throw(IOException) {
    HashTable<String, AnyValue> result;
    result[MESSAGE("encoder")] = Type::getType(*this);
    result[MESSAGE("description")] = MESSAGE("Portable Pixmap Format");
//     result[MESSAGE("bits per pixel")] = static_cast<unsigned int>(header.pixelDepth);
//     result[MESSAGE("width")] = static_cast<unsigned int>(header.width);
//     result[MESSAGE("height")] = static_cast<unsigned int>(header.height);
    return result;
  }

}; // end of gip namespace
