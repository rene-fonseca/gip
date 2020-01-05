/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

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

namespace gip {

_COM_AZURE_DEV__BASE__PACKED__BEGIN
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
  } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END

  PCXEncoder::PCXEncoder() noexcept {
  }

  String PCXEncoder::getDescription() const noexcept {
    return Literal("Zsoft Corporation PC Paintbrush");
  }

  String PCXEncoder::getDefaultExtension() const noexcept {
    return Literal("pcx");
  }

  bool PCXEncoder::isValid(const String& filename) throw(IOException) {
    PCXHeader header;
    unsigned int size;

    {
      File file(filename, File::READ, 0);
      file.read(Cast::getAddress(header), sizeof(header));
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
    file.read(Cast::getAddress(header), sizeof(header));

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
    Allocator<uint8> encoded(maximum(bytesPerRow * 2, (unsigned int)256 * 3 + 1));
    Allocator<uint8> decoded(bytesPerRow);
    Allocator<ColorPixel> palette256(256);
    bool palettePresent = false;

    if (header.version == 5) { // do we need to look for palette
      file.setPosition(-(256 * 3 + 1), File::END);
      file.read(encoded.getElements(), 256 * 3 + 1);
      const uint8* current = encoded.getElements();
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

    uint8* src = nullptr;
    uint8* endOfBuffer = nullptr;
    unsigned int offset = sizeof(header);

    for (unsigned int row = 0; row < height; ++row) { // read all rows of image
      switch (header.encoding) {
      case 0: // scan line is not encoded
        file.read(decoded.getElements(), decoded.getSize());
        break;

      case 1: // scan line is run length encoded

        uint8* dest = decoded.getElements();

        for (unsigned int bytesToRead = bytesPerRow; bytesToRead > 0; ) {
          if (src >= endOfBuffer) { // if empty buffer then refill
            unsigned int count = file.read(
              encoded.getElements(),
              encoded.getSize(),
              true
            );
            bassert(count > 0, InvalidFormat(this));
            src = encoded.getElements();
            endOfBuffer = src + count;
          }

          uint8 value = *src++; // get next byte
          ++offset;
          if ((0xc0 & value) == 0xc0) {
            unsigned int count = 0x3f & value;
            bassert(count <= bytesToRead, InvalidFormat(this));

            if (src >= endOfBuffer) { // if empty buffer then refill
              unsigned int count = file.read(
                encoded.getElements(),
                encoded.getSize(),
                true
              );
              bassert(count > 0, InvalidFormat(this));
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
            const uint8* color = decoded.getElements();
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
            const uint8* red = decoded.getElements();
            const uint8* green = red + header.bytesPerLine;
            const uint8* blue = green + header.bytesPerLine;
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
    bassert((width > 0) && (height > 0), ImageException("Dimension of image not supported by encoder", this));

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

    file.write(Cast::getAddress(header), sizeof(header));

    Allocator<uint8> reordered(bytesPerLine * 3);
    Allocator<uint8> encoded(bytesPerLine * 3 * 2); // encoded data cannot exceed double size
    const ColorPixel* imageElement = image->getElements();

    for (unsigned int row = 0; row < height; ++row) {

      uint8* red = reordered.getElements();
      uint8* green = red + bytesPerLine;
      uint8* blue = green + bytesPerLine;
      for (unsigned int column = 0; column < width; ++column) {
        *red++ = imageElement->red;
        *green++ = imageElement->green;
        *blue++ = imageElement->blue;
        ++imageElement;
      }

      switch (header.encoding) {
      case 0: // no encoding
        file.write(reordered.getElements(), reordered.getSize());
        break;
      case 1: // run length encoding
        {
          uint8* dest = encoded.getElements();
          const uint8* src = reordered.getElements();
          const uint8* end = src + reordered.getSize();
          while (src < end) {
            const uint8 first = *src++;
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
          file.write(encoded.getElements(), dest - encoded.getElements());
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
    bassert(
      (width > 0) && (height > 0),
      ImageException("Dimension of image not supported by encoder", this)
    );
    
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

    file.write(Cast::getAddress(header), sizeof(header));

    Allocator<uint8> encoded(maximum(bytesPerLine * 2, 256U * 3 + 1));
    const GrayPixel* imageElement = image->getElements();

    for (unsigned int row = 0; row < height; ++row) {

      switch (header.encoding) {
      case 0: // no encoding
        {
          uint8* dest = encoded.getElements();
          for (unsigned int column = 0; column < width; ++column) {
            *dest++ = *imageElement;
            ++imageElement;
          }
          file.write(encoded.getElements(), bytesPerLine);
          break;
        }
      case 1: // run length encoding
        {
          uint8* dest = encoded.getElements();
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
          file.write(encoded.getElements(), dest - encoded.getElements());
          break;
        }
      }
    }

    // write palette
    uint8* palette = encoded.getElements();
    *palette++ = 12;
    for (unsigned int i = 0; i < 256; ++i) {
      *palette++ = i;
      *palette++ = i;
      *palette++ = i;
    }
    file.write(encoded.getElements(), 256 * 3 + 1);
    file.truncate(file.getPosition());
  }

  HashTable<String, AnyValue> PCXEncoder::getInformation(
    const String& filename) throw(IOException) {
    HashTable<String, AnyValue> result;
    
    PCXHeader header;
    
    {
      File file(filename, File::READ, 0);
      file.read(Cast::getAddress(header), sizeof(header));
    }
    
    result[(String)"encoder"] = Type::getType(*this);
    result[(String)"description"] = "Zsoft Corporation PC Paintbrush";
    result[(String)"manufacturer"] = static_cast<unsigned int>(header.manufacturer);
    result[(String)"version"] = static_cast<unsigned int>(header.version);
    result[(String)"encoding"] = static_cast<unsigned int>(header.encoding);
    result[(String)"bits per pixel"] = static_cast<unsigned int>(header.bitsPerPixel);
    result[(String)"min x"] = static_cast<unsigned int>(header.minX);
    result[(String)"min y"] = static_cast<unsigned int>(header.minY);
    result[(String)"max x"] = static_cast<unsigned int>(header.maxX);
    result[(String)"max y"] = static_cast<unsigned int>(header.maxY);
    result[(String)"horizontal resolution"] = static_cast<unsigned int>(header.horizontalResolution);
    result[(String)"vertical resolution"] = static_cast<unsigned int>(header.verticalResolution);
    result[(String)"planes"] = static_cast<unsigned int>(header.planes);
    result[(String)"horizontal screen size"] = static_cast<unsigned int>(header.horizontalScreenSize);
    result[(String)"vertical screen size"] = static_cast<unsigned int>(header.verticalScreenSize);
    return result;
  }

}; // end of gip namespace
