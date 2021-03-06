/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

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

namespace gip {

  PPMEncoder::PPMEncoder() noexcept {
  }

  String PPMEncoder::getDescription() const noexcept {
    return MESSAGE("Portable Pixmap Format");
  }
  
  String PPMEncoder::getDefaultExtension() const noexcept {
    return MESSAGE("ppm");
  }
  
  bool PPMEncoder::isValid(const String& filename) {
    return true;
  }
  
  ColorImage* PPMEncoder::read(
    const String& filename) {
    return 0;
  }
  
  void PPMEncoder::write(
    const String& filename,
    const ColorImage* image)
  {
    if (!image) {
      _throw NullPointer(this);
    }
    Dimension dimension = image->getDimension();
    
    FileOutputStream file(
      filename, FileOutputStream::CREATE | FileOutputStream::TRUNCATE);
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
        int c = minimum<MemorySize>(endOfRow - src, 5); // do not exceed 70 chars per line
        while (c--) {
          ColorPixel pixel = *src++;
          out << setWidth(4) << pixel.red
              << setWidth(4) << pixel.green
              << setWidth(4) << pixel.blue;
          if (c > 0) {
            out << MESSAGE("  ");
          }
        }
        out << EOL;
      }
      src -= dimension.getWidth();
    }
  }
  
  void PPMEncoder::writeGray(
    const String& filename,
    const GrayImage* image) {
    _throw NotSupported(this);
  }
  
  ArrayMap<String, AnyValue> PPMEncoder::getInformation(const String& filename)
  {
    return {
      {MESSAGE("encoder"), Type::getType(*this)},
      {MESSAGE("description"), MESSAGE("Portable Pixmap Format")}
    };
    // result[MESSAGE("bits per pixel")] = static_cast<unsigned int>(header.pixelDepth);
    // result[MESSAGE("width")] = static_cast<unsigned int>(header.width);
    // result[MESSAGE("height")] = static_cast<unsigned int>(header.height);
  }

}; // end of gip namespace
