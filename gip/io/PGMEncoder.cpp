/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/PGMEncoder.h>
#include <base/io/FileInputStream.h>
#include <base/io/FileOutputStream.h>
#include <base/string/FormatOutputStream.h>
#include <base/NotSupported.h>

namespace gip {

  PGMEncoder::PGMEncoder() throw() {
  }

  String PGMEncoder::getDescription() const throw() {
    return MESSAGE("Portable Graymap Format");
  }
  
  String PGMEncoder::getDefaultExtension() const throw() {
    return MESSAGE("pgm");
  }
  
  bool PGMEncoder::isValid(const String& filename) throw(IOException) {
    return true;
  }
  
  ColorImage* PGMEncoder::read(const String& filename) throw(InvalidFormat, IOException) {
    return 0;
  }
  
  void PGMEncoder::write(
    const String& filename,
    const ColorImage* image) throw(ImageException, IOException) {
    throw NotSupported(this);
  }
  
  void PGMEncoder::writeGray(
    const String& filename,
    const GrayImage* image) throw(ImageException, IOException) {
    assert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    
    FileOutputStream file(
      filename, FileOutputStream::CREATE | FileOutputStream::TRUNCATE
    );
    FormatOutputStream out(file);
    
    out << MESSAGE("P2") << EOL
        << dimension.getWidth() << ' ' << dimension.getHeight() << EOL
        << 255 << EOL;
    out << setWidth(4);
    //FormatOutputStream::Context push(out);

    const GrayPixel* end = image->getElements();
    const GrayPixel* src = end + dimension.getSize();
    unsigned int row = 0;
    while (src > end) {
      const GrayPixel* endOfRow = src;
      src -= dimension.getWidth();
      while (src < endOfRow) {
        int c = minimum<MemorySize>(endOfRow - src, 17); // do not exceed 70 chars per line
        while (c--) {
          out << setWidth(4) << *src++;
        }
        out << EOL;
      }
      src -= dimension.getWidth();
    }
  }
  
  HashTable<String, AnyValue> PGMEncoder::getInformation(const String& filename) throw(IOException) {
    HashTable<String, AnyValue> result;
    result[MESSAGE("encoder")] = Type::getType(*this);
    result[MESSAGE("description")] = MESSAGE("Portable Graymap Format");
//     result[MESSAGE("bits per pixel")] = static_cast<unsigned int>(header.pixelDepth);
//     result[MESSAGE("width")] = static_cast<unsigned int>(header.width);
//     result[MESSAGE("height")] = static_cast<unsigned int>(header.height);
    return result;
  }

}; // end of gip namespace
