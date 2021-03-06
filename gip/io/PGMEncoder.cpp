/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

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

  PGMEncoder::PGMEncoder() noexcept {
  }

  String PGMEncoder::getDescription() const noexcept {
    return MESSAGE("Portable Graymap Format");
  }
  
  String PGMEncoder::getDefaultExtension() const noexcept {
    return MESSAGE("pgm");
  }
  
  bool PGMEncoder::isValid(const String& filename) {
    return true;
  }
  
  ColorImage* PGMEncoder::read(const String& filename) {
    return 0;
  }
  
  void PGMEncoder::write(
    const String& filename,
    const ColorImage* image) {
    _throw NotSupported(this);
  }
  
  void PGMEncoder::writeGray(
    const String& filename,
    const GrayImage* image)
  {
    if (!image) {
      _throw NullPointer(this);
    }
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
    // unsigned int row = 0;
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
  
  ArrayMap<String, AnyValue> PGMEncoder::getInformation(const String& filename)
  {
    return {
      {MESSAGE("encoder"), Type::getType(*this)},
      {MESSAGE("description"), MESSAGE("Portable Graymap Format")}
      // MESSAGE("bits per pixel")] = static_cast<unsigned int>(header.pixelDepth)
      // MESSAGE("width")] = static_cast<unsigned int>(header.width)
      // MESSAGE("height")] = static_cast<unsigned int>(header.height)
    };
  }

}; // end of gip namespace
