/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/PCXEncoder.h>
#include <base/mem/Allocator.h>
#include <base/io/File.h>
#include <base/concurrency/Thread.h>
#include <base/Primitives.h>
#include <base/ByteOrder.h>

using namespace base;

namespace gip {

  struct PCXHeader {
    uint8 manufacturer; // 10 = ZSoft .pcx
    uint8 version; // version information
    uint8 encoding; // 1 = run length encoding
    uint8 bitsPerPixel; // number of bits to represent a pixel (per plane) - 1, 2, 4, or 8
    LittleEndian<int16> minX; // minimum x
    LittleEndian<int16> minY; // minimum y
    LittleEndian<int16> maxX; // maximum x
    LittleEndian<int16> maxY; // maximum y
    LittleEndian<int16> horizontalResolution; // horizontal resolution in DPI
    LittleEndian<int16> verticalResolution; // vertical resolution in DPI
    uint8 palette[48]; // palette
    uint8 reserved; // reserved - set to 0
    uint8 planes; // number of color planes
    LittleEndian<int16> bytesPerLine; // number of bytes per scanline (even)
    LittleEndian<int16> paletteType; // palette: 1 - color, 2 - gray
    LittleEndian<int16> horizontalScreenSize; // horizontal screen size in pixels
    LittleEndian<int16> verticalScreenSize; // vertical screen size in pixels
    uint8 zeros[54]; // make header 128 bytes - set to 0
  } _DK_SDU_MIP__BASE__PACKED;

  PCXEncoder::PCXEncoder() throw() {
  }

  String PCXEncoder::getDescription() const throw() {
    return MESSAGE("Zsoft Corporation PC Paintbrush");
  }

  String PCXEncoder::getDefaultExtension() const throw() {
    return MESSAGE("pcx");
  }

  bool PCXEncoder::isValid(const String& filename) throw(IOException) {
    PCXHeader header;
    unsigned int size;

    {
      File file(filename, File::READ, 0);
      file.read(getCharAddress(header), sizeof(header));
      size = file.getSize();
    }

    return (header.manufacturer == 10) &&
      (header.version == 5) &&
      ((header.encoding == 0) || (header.encoding == 1)) &&
      (header.bitsPerPixel == 8) &&
      (header.minX <= header.maxX) && (header.minY <= header.maxY) &&
      ((header.planes == 1) || (header.planes == 3)) &&
      (header.bytesPerLine % 2 == 0) &&
      (header.paletteType == 1);
  }

  ColorImage* PCXEncoder::read(const String& filename) throw(InvalidFormat, IOException) {

    File file(filename, File::READ, 0);

    PCXHeader header;
    file.read(getCharAddress(header), sizeof(header));

    bool valid = (header.manufacturer == 10) &&
      (header.version == 5) &&
      ((header.encoding == 0) || (header.encoding == 1)) &&
      (header.bitsPerPixel == 8) &&
      (header.minX <= header.maxX) && (header.minY <= header.maxY) &&
      ((header.planes == 1) || (header.planes == 3)) &&
      (header.bytesPerLine % 2 == 0) &&
      (header.paletteType == 1);

    if (!valid) {
      return 0;
    }

    unsigned int width = header.maxX - header.minX + 1;
    unsigned int height = header.maxY - header.minY + 1;
    unsigned int bytesPerRow = header.planes * static_cast<unsigned int>(header.bytesPerLine);

    ColorImage image = ColorImage(Dimension(width, height));
    ColorPixel* imageElement = image.getElements();
    Allocator<unsigned char> encoded(maximum(bytesPerRow * 2, (unsigned int)256 * 3 + 1));
    Allocator<unsigned char> decoded(bytesPerRow);
    Allocator<ColorPixel> palette256(256);
    bool palettePresent = false;

    if (header.version == 5) { // do we need to look for palette
      file.setPosition(-(256 * 3 + 1), File::END);
      file.read(pointer_cast<char*>(encoded.getElements()), 256 * 3 + 1);
      const unsigned char* current = encoded.getElements();
      if (*current++ == 12) {
        palettePresent = true;
        ColorPixel* color = palette256.getElements();
        for (unsigned int i = 0; i < 256; ++i) {
          color->red = *current++;
          color->green = *current++;
          color->blue = *current++;
          ++color;
        }
      }
      file.setPosition(sizeof(PCXHeader), File::BEGIN);
    }

    if (!palettePresent) {
      ColorPixel* color = palette256.getElements();
      for (unsigned int i = 0; i < 256; ++i) {
        color->blue = i;
        color->green = i;
        color->red = i;
        ++color;
      }
    }

    unsigned char* src = 0;
    unsigned char* endOfBuffer = 0;
    unsigned int offset = sizeof(header);

    for (unsigned int row = 0; row < height; ++row) { // read all rows of image
      switch (header.encoding) {
      case 0: // scan line is not encoded
        file.read(pointer_cast<char*>(decoded.getElements()), decoded.getSize());
        break;

      case 1: // scan line is run length encoded

        unsigned char* dest = decoded.getElements();

        for (unsigned int bytesToRead = bytesPerRow; bytesToRead > 0; ) {
          if (src >= endOfBuffer) { // if empty buffer then refill
            unsigned int count = file.read(pointer_cast<char*>(encoded.getElements()), encoded.getSize(), true);
            assert(count > 0, InvalidFormat(this));
            src = encoded.getElements();
            endOfBuffer = src + count;
          }

          unsigned char value = *src++; // get next byte
          ++offset;
          if ((0xc0 & value) == 0xc0) {
            unsigned int count = 0x3f & value;
            assert(count <= bytesToRead, InvalidFormat(this));

            if (src >= endOfBuffer) { // if empty buffer then refill
              unsigned int count = file.read(pointer_cast<char*>(encoded.getElements()), encoded.getSize(), true);
              assert(count > 0, InvalidFormat(this));
              src = encoded.getElements();
              endOfBuffer = src + count;
            }
            value = *src++; // get next byte
            ++offset;
            for (unsigned int i = 0; i < count; i++) {
              *dest++ = value;
            }
            bytesToRead -= count;
          } else {
            *dest++ = value;
            --bytesToRead;
          }
        }
        //fout << HEX << offset << FLUSH;

        // skip???

        break;
      }

      // store row in image
      switch (header.bitsPerPixel) {
      case 8:
        switch (header.planes) {
        case 1:
          {
            const unsigned char* color = decoded.getElements();
            //fout << "  F: " << (unsigned int)*color;
            const ColorPixel* palette = palette256.getElements();
            for (unsigned int count = width; count > 0; --count) {
              *imageElement++ = palette[*color++];
            }
            //fout << "  L: " << (unsigned int)*--color << ENDL;
          }
          break;
        case 3:
          {
            const unsigned char* red = decoded.getElements();
            const unsigned char* green = red + header.bytesPerLine;
            const unsigned char* blue = green + header.bytesPerLine;
            //fout << "  F: " << (unsigned int)*red << " " << (unsigned int)*green << " " << (unsigned int)*blue;
            for (unsigned int count = width; count > 0; --count) {
              imageElement->blue = *blue++;
              imageElement->green = *green++;
              imageElement->red = *red++;
              ++imageElement;
            }
            //fout << "  L: " << (unsigned int)*--red << " " << (unsigned int)*--green << " " << (unsigned int)*--blue << ENDL;
          }
          break;
        }
        break;
      }
    }

    return new ColorImage(image);
  }

  void PCXEncoder::write(const String& filename, const ColorImage* image) throw(ImageException, IOException) {
    PCXHeader header;
    File file(filename, File::WRITE, File::CREATE);

    unsigned int width = image->getDimension().getWidth();
    unsigned int height = image->getDimension().getHeight();
    unsigned int bytesPerLine = (width + 1)/2*2;
    assert((width > 0) && (height > 0), ImageException("Dimension of image not supported by encoder", this));

    clear(header);
    header.manufacturer = 10;
    header.version = 5;
    header.encoding = 1;
    header.bitsPerPixel = 8;
    header.minX = 0;
    header.minY = 0;
    header.maxX = width - 1;
    header.maxY = height - 1;
    header.planes = 3;
    header.bytesPerLine = bytesPerLine;
    header.paletteType = 1;

    file.write((const char*)&header, sizeof(header));

    Allocator<unsigned char> reordered(bytesPerLine * 3);
    Allocator<unsigned char> encoded(bytesPerLine * 3 * 2); // encoded data cannot exceed double size
    const ColorPixel* imageElement = image->getElements();

    for (unsigned int row = 0; row < height; ++row) {

      unsigned char* red = reordered.getElements();
      unsigned char* green = red + bytesPerLine;
      unsigned char* blue = green + bytesPerLine;
      for (unsigned int column = 0; column < width; ++column) {
        *red++ = imageElement->red;
        *green++ = imageElement->green;
        *blue++ = imageElement->blue;
        ++imageElement;
      }

      switch (header.encoding) {
      case 0: // no encoding
        file.write((char*)reordered.getElements(), reordered.getSize());
        break;
      case 1: // run length encoding
        {
          unsigned char* dest = encoded.getElements();
          const unsigned char* src = reordered.getElements();
          const unsigned char* end = src + reordered.getSize();
          while (src < end) {
            const unsigned char first = *src++;
            unsigned int count = 1;
            while ((src < end) && (first == *src) && (count < 0x3f)) {
              ++src;
              ++count;
            }
            if (count > 1) {
              *dest++ = 0xc0 + count;
            } else if ((first & 0xc0) == 0xc0) { // special case
              *dest++ = 0xc0 + 1;
            }
            *dest++ = first;
          }
          file.write((char*)encoded.getElements(), dest - encoded.getElements());
          break;
        }
      }
    }
    file.truncate(file.getPosition());
  }

  void PCXEncoder::writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException) {
    PCXHeader header;
    File file(filename, File::WRITE, File::CREATE);

    unsigned int width = image->getDimension().getWidth();
    unsigned int height = image->getDimension().getHeight();
    unsigned int bytesPerLine = (width + 1)/2*2;
    assert((width > 0) && (height > 0), ImageException("Dimension of image not supported by encoder", this));

    clear(header);
    header.manufacturer = 10;
    header.version = 5;
    header.encoding = 1;
    header.bitsPerPixel = 8;
    header.minX = 0;
    header.minY = 0;
    header.maxX = width - 1;
    header.maxY = height - 1;
    header.planes = 1;
    header.bytesPerLine = bytesPerLine;
    header.paletteType = 1;

    file.write((const char*)&header, sizeof(header));

    Allocator<unsigned char> encoded(maximum(bytesPerLine * 2, 256U * 3 + 1));
    const GrayPixel* imageElement = image->getElements();

    for (unsigned int row = 0; row < height; ++row) {

      switch (header.encoding) {
      case 0: // no encoding
        {
          unsigned char* dest = encoded.getElements();
          for (unsigned int column = 0; column < width; ++column) {
            *dest++ = *imageElement;
            ++imageElement;
          }
          file.write((char*)encoded.getElements(), bytesPerLine);
          break;
        }
      case 1: // run length encoding
        {
          unsigned char* dest = encoded.getElements();
          const GrayPixel* end = imageElement + width;
          while (imageElement < end) {
            const GrayPixel first = *imageElement++;
            unsigned int count = 1;
            while ((imageElement < end) && (first == *imageElement) && (count < 0x3f)) {
              ++imageElement;
              ++count;
            }
            if (count > 1) {
              *dest++ = 0xc0 + count;
            } else if ((first & 0xc0) == 0xc0) { // special case
              *dest++ = 0xc0 + 1;
            }
            *dest++ = first;
          }
          file.write((char*)encoded.getElements(), dest - encoded.getElements());
          break;
        }
      }
    }

    // write palette
    unsigned char* palette = encoded.getElements();
    *palette++ = 12;
    for (unsigned int i = 0; i < 256; ++i) {
      *palette++ = i;
      *palette++ = i;
      *palette++ = i;
    }
    file.write((char*)encoded.getElements(), 256 * 3 + 1);
    file.truncate(file.getPosition());
  }

  FormatOutputStream& PCXEncoder::getInfo(FormatOutputStream& stream, const String& filename) throw(IOException) {
    PCXHeader header;
    
    {
      File file(filename, File::READ, 0);
      file.read(getCharAddress(header), sizeof(header));
    }
    
    stream << "PCXEncoder (Zsoft Corporation PC Paintbrush):" << EOL
           << "  manufacturer=" << static_cast<unsigned int>(header.manufacturer) << EOL
           << "  version=" << static_cast<unsigned int>(header.version) << EOL
           << "  encoding=" << static_cast<unsigned int>(header.encoding) << EOL
           << "  bitsPerPixel=" << static_cast<unsigned int>(header.bitsPerPixel) << EOL
           << "  minX=" << header.minX << EOL
           << "  minY=" << header.minY << EOL
           << "  maxX=" << header.maxX << EOL
           << "  maxY=" << header.maxY << EOL
           << "  horizontalResolution=" << header.horizontalResolution << EOL
           << "  verticalResolution=" << header.verticalResolution << EOL
           << "  planes=" << static_cast<unsigned int>(header.planes) << EOL
           << "  bytesPerLine=" << header.bytesPerLine << EOL
           << "  paletteType=" << header.paletteType << EOL
           << "  horizontalScreenSize=" << header.horizontalScreenSize << EOL
           << "  verticalScreenSize=" << header.verticalScreenSize << EOL;
    return stream;
  }

}; // end of namespace
