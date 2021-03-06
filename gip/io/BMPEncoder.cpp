/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/BMPEncoder.h>
#include <base/mem/Allocator.h>
#include <base/io/File.h>
#include <base/io/FileReader.h>
#include <base/concurrency/Thread.h>
#include <base/Primitives.h>
#include <base/ByteOrder.h>
#include <base/string/String.h>
#include <base/Cast.h>

namespace gip {

  class FullPalette : public UnaryOperation<uint8, ColorPixel> {
  private:
    
    ColorPixel* palette = nullptr;
  public:
    
    inline FullPalette(ColorPixel* _palette) : palette(_palette) {
    }
    
    inline Result operator()(Argument value) const noexcept {
      return palette[value];
    }
  };

  class PartialPalette : public UnaryOperation<uint8, ColorPixel> {
  private:
    
    ColorPixel* palette = nullptr;
    unsigned int maxIndex = 0;
  public:
    
    inline PartialPalette(ColorPixel* _palette, unsigned int _maxIndex)
      : palette(_palette), maxIndex(_maxIndex) {
    }
    
    inline Result operator()(Argument value) const {
      bassert(
        value < maxIndex,
        bindCause(InvalidFormat("Color table index out of range", this), ImageEncoder::INVALID_COLOR)
      );
      return palette[value];
    }
  };

//template<class UNOPR>
//inline void xxx(ColorImage& image, FileReader& reader, unsigned int bytesPerLine, UNOPR opr) {
//  ColorPixel* dest = image.getElements();
//  Dimension dimension(image.getDimension());
//  for (unsigned int row = dimension.getHeight(); row > 0; --row) {
//    reader.peek(bytesPerLine);
//    for (unsigned int column = dimension.getWidth(); column > 0; --column) {
//      *dest++ = byteToColor(*src++);
//    }
//    reader.skip(bytesPerLine);
//  }
//}



  enum {BMP_RGB = 0, BMP_RLE8 = 1, BMP_RLE4 = 2, BMP_BITFIELDS = 3};

_COM_AZURE_DEV__BASE__PACKED__BEGIN
  struct BMPHeader {
    char identifier[2]; // the characters identifying the bitmap
    LittleEndian<uint32> fileSize; // complete file size in bytes
    LittleEndian<uint32> reserved; // reserved for later use
    LittleEndian<uint32> bitmapDataOffset; // offset from beginning of file to the beginning of the bitmap data
    LittleEndian<uint32> bitmapHeaderSize; // size of the header used to describe the bitmap colors and compression
    LittleEndian<uint32> width; // width of bitmap in pixels
    LittleEndian<uint32> height; // height of bitmap in pixels
    LittleEndian<uint16> planes; // number of planes in this bitmap
    LittleEndian<uint16> bitsPerPixel; // bits per pixel used to store palette entry information
    LittleEndian<uint32> compression; // specifies the compression
    LittleEndian<uint32> bitmapDataSize; // size of the bitmap data in bytes
    LittleEndian<uint32> horizontalResolution; // horizontal resolution expressed in pixels per meter
    LittleEndian<uint32> verticalResolution; // vertical resolution expressed in pixels per meter
    LittleEndian<uint32> colorsUsed; // number of colors used by this bitmap
    LittleEndian<uint32> importantColors; // number of important colors
  } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END

_COM_AZURE_DEV__BASE__PACKED__BEGIN
  struct BMPPaletteEntry {
    uint8 blue;
    uint8 green;
    uint8 red;
    uint8 reserved; // must be zero
  } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END

  BMPEncoder::BMPEncoder() noexcept {
  }

  String BMPEncoder::getDescription() const noexcept {
    return MESSAGE("Windows Bitmap File Format");
  }

  String BMPEncoder::getDefaultExtension() const noexcept {
    return MESSAGE("bmp");
  }

  bool BMPEncoder::isValid(const String& filename) {
    BMPHeader header;
    unsigned int size = 0;

    {
      File file(filename, File::READ, 0);
      file.read(Cast::getAddress(header), sizeof(header));
      size = file.getSize();
    }

    bool result = ((header.identifier[0] == 'B') && (header.identifier[1] == 'M') &&
                   (header.fileSize == size) &&
                   (header.bitmapHeaderSize == 0x28) &&
                   (header.width > 0) && (header.height > 0));

    switch (header.compression) {
    case BMPEncoder::RGB:
      if (
        ((header.planes == 1) && (header.bitsPerPixel == 1)) ||
        ((header.planes == 1) && (header.bitsPerPixel == 4)) ||
        ((header.planes == 1) && (header.bitsPerPixel == 8)) ||
        ((header.planes == 1) && (header.bitsPerPixel == 24)) ||
        ((header.planes == 1) && (header.bitsPerPixel == 32)) ||
        ((header.planes == 3) && (header.bitsPerPixel == 24)) ||
        ((header.planes == 3) && (header.bitsPerPixel == 32))) {
        break;
      }
      result = false;
    case BMPEncoder::RLE8:
      if ((header.planes == 1) && (header.bitsPerPixel == 8)) {
        break;
      }
      result = false;
    case BMPEncoder::RLE4:
      if ((header.planes == 1) && (header.bitsPerPixel == 4)) {
        break;
      }
      result = false;
    default:
      result = false;
    }

    return result;
  }

  ColorImage* BMPEncoder::read(const String& filename) {
    File file(filename, File::READ, 0);

    BMPHeader header;
    file.read(header);

    if (
      (header.identifier[0] == 'B') && (header.identifier[1] == 'M') &&
      (header.bitmapHeaderSize == 0x28) && // must be dword aligned
      (header.width > 0) && (header.height > 0)
    ) {

      switch (header.compression) {
      case BMPEncoder::RGB:
        if (
          ((header.planes == 1) && (header.bitsPerPixel == 1)) ||
          ((header.planes == 1) && (header.bitsPerPixel == 4)) ||
          ((header.planes == 1) && (header.bitsPerPixel == 8)) ||
          ((header.planes == 1) && (header.bitsPerPixel == 24)) ||
          ((header.planes == 1) && (header.bitsPerPixel == 32)) ||
          ((header.planes == 3) && (header.bitsPerPixel == 24)) ||
          ((header.planes == 3) && (header.bitsPerPixel == 32))) {
          break;
        }
        return 0;
      case BMPEncoder::RLE8:
        if ((header.planes == 1) && (header.bitsPerPixel == 8)) {
          break;
        }
        return 0;
      case BMPEncoder::RLE4:
        if ((header.planes == 1) && (header.bitsPerPixel == 4)) {
          break;
        }
        return 0;
      default:
        return 0;
      }

      // if (header.bitsPerPixel <= 8) {header.colorsUsed <= (1 << header.bitsPerPixel)}
      // check header.bitmapDataOffset >= ???

      const Dimension dimension(header.width, header.height);
      ColorImage* image = new ColorImage(dimension);

      // read or skip color table if present
      unsigned int numberOfColors = header.colorsUsed;
      ColorPixel palette[256];
      if ((numberOfColors > 0) || (header.bitsPerPixel <= 8)) { // is palette present
        file.setPosition(sizeof(BMPHeader), File::BEGIN); // move to beginning of palette
        if (header.bitsPerPixel <= 8) { // should palette be used
          unsigned int maximumNumberOfColors = 1 << header.bitsPerPixel;
          bassert(
            numberOfColors <= maximumNumberOfColors,
            bindCause(InvalidFormat(this), ImageEncoder::INVALID_COLOR_TABLE)
          );
          if (numberOfColors == 0) { // use maximum number of colors
            numberOfColors = maximumNumberOfColors;
          }
          BMPPaletteEntry srcPalette[256];
          file.read(
            Cast::getAddress(srcPalette),
            sizeof(BMPPaletteEntry) * numberOfColors
          ); // get palette
          for (unsigned int i = 0; i < numberOfColors; ++i) {
            palette[i].blue = srcPalette[i].blue;
            palette[i].green = srcPalette[i].green;
            palette[i].red = srcPalette[i].red;
          }
        } else {
          file.setPosition(numberOfColors * sizeof(BMPPaletteEntry), File::CURRENT); // skip palette
        }
      }

      FileReader reader(file, header.bitmapDataOffset);

      file.setPosition(header.bitmapDataOffset, File::BEGIN); // move to beginning of bitmap data
      ColorPixel* dest = image->getElements();

      switch (header.compression) {
      case BMPEncoder::RGB: // no compression
        {
          switch (header.bitsPerPixel) {
          case 1:
            {
              unsigned int bytesPerLine = (dimension.getWidth() + 31)/32*4; // includes zero-pads
              unsigned int numberOfFilledBlocks = dimension.getWidth() >> 3;
              unsigned int pixelsInLastBlock = dimension.getWidth() & 0x07;
              PartialPalette valueToColor(palette, numberOfColors);
              for (unsigned int row = dimension.getHeight(); row > 0; --row) {
                FileReader::ReadIterator src = reader.peek(bytesPerLine);
                for (unsigned int block = numberOfFilledBlocks; block > 0; --block) {
                  unsigned int value = *src++;
                  for (unsigned int bit = 8; bit > 0; --bit) {
                    *dest++ = valueToColor((value & 0x80) ? 1 : 0);
                    value <<= 1;
                  }
                }
                if (pixelsInLastBlock) {
                  unsigned int value = *src++;
                  for (unsigned int bit = pixelsInLastBlock; bit > 0; --bit) {
                    *dest++ = valueToColor((value & 0x80) ? 1 : 0);
                    value <<= 1;
                  }
                }
                reader.skip(bytesPerLine);
              }
            }
            break;
          case 4:
            {
              unsigned int bytesPerLine = ((dimension.getWidth() + 1)/2 + 3)/4*4; // includes zero-pads
              unsigned int numberOfFilledBytes = dimension.getWidth() >> 1;
              bool hasIncompleteBlock = dimension.getWidth() & 0x01;
              PartialPalette valueToColor(palette, numberOfColors);
              for (unsigned int row = dimension.getHeight(); row > 0; --row) {
                FileReader::ReadIterator src = reader.peek(bytesPerLine);
                for (unsigned int block = numberOfFilledBytes; block > 0; --block) {
                  unsigned int value = *src++;
                  *dest++ = valueToColor(value >> 4);
                  *dest++ = valueToColor(value & 0x0f);
                }
                if (hasIncompleteBlock) {
                  *dest++ = valueToColor((*src++) >> 4);
                }
                reader.skip(bytesPerLine);
              }
            }
            break;
          case 8:
            {
              unsigned int bytesPerLine = (dimension.getWidth() + 3)/4*4; // includes zero-pads
              PartialPalette valueToColor(palette, numberOfColors);
              for (unsigned int row = dimension.getHeight(); row > 0; --row) {
                FileReader::ReadIterator src = reader.peek(bytesPerLine);
                for (unsigned int column = dimension.getWidth(); column > 0; --column) {
                  *dest++ = valueToColor(*src++);
                }
                reader.skip(bytesPerLine);
              }
            }
            break;
          case 24:
            {
              unsigned int bytesPerLine = (dimension.getWidth() * 3 + 3)/4*4; // includes zero-pads
              for (unsigned int row = dimension.getHeight(); row > 0; --row) {
                FileReader::ReadIterator src = reader.peek(bytesPerLine);
                for (unsigned int column = dimension.getWidth(); column > 0; --column) {
                  uint8 blue = *src++;
                  uint8 green = *src++;
                  uint8 red = *src++;
                  *dest++ = makeColorPixel(red, green, blue);
                }
                reader.skip(bytesPerLine);
              }
            }
            break;
          case 32:
            {
              unsigned int bytesPerLine = dimension.getWidth() * 4; // no zero-pads
              for (unsigned int row = dimension.getHeight(); row > 0; --row) {
                FileReader::ReadIterator src = reader.peek(bytesPerLine);
                for (unsigned int column = dimension.getWidth(); column > 0; --column) {
                  uint8 blue = *src++;
                  uint8 green = *src++;
                  uint8 red = *src++;
                  ++src;
                  *dest++ = makeColorPixel(red, green, blue);
                }
                reader.skip(bytesPerLine);
              }
            }
            break;
          }
        }
        break;
      case BMPEncoder::RLE8: // 8 bit run-length encoding
        {
          Allocator<uint8> buffer(header.bitmapDataSize); // all image data
          file.read(buffer.getElements(), header.bitmapDataSize); // read all image data
          const uint8* src = buffer.getElements();
          const uint8* srcEnd = src + header.bitmapDataSize;
          ColorPixel* elements = image->getElements();
          unsigned int row = 0;
          unsigned int column = 0;

          while (src < srcEnd) {
            unsigned int first = *src++;
            unsigned int second = *src++;

            if (first == 0) { // escape follows - or absolute mode
              switch (second) {
              case 0: // end of line
                ++row;
                column = 0;
                break;
              case 1: // end of image
                row = dimension.getHeight();
                column = 0;
                break;
              case 2: // delta
                column += *src++;
                row += *src++;
                break;
              default: // absolute mode
                ColorPixel* dest = elements + dimension.getWidth() * row + column;
                for (unsigned int count = second; count > 0; --count) {
                  unsigned int index = *src++;
                  bassert(
                    index < numberOfColors,
                    bindCause(InvalidFormat("Color table index out of range", this), ImageEncoder::INVALID_COLOR)
                  );
                  *dest++ = palette[index];
                }
                if (second & 0x01) { // skip to word boundary
                  ++src;
                }
                column += second;
              }
            } else {
              bassert(
                second < numberOfColors,
                bindCause(InvalidFormat("Color table index out of range", this), ImageEncoder::INVALID_COLOR)
              );
              fill<ColorPixel>(elements + dimension.getWidth() * row + column, first, palette[second]);
              column += first;
            }
          }
        }
        break;
      case BMPEncoder::RLE4: // 4 bit run-length encoding
        {
          Allocator<uint8> buffer(header.bitmapDataSize); // all image data
          file.read(buffer.getElements(), header.bitmapDataSize); // read all image data
          const uint8* src = buffer.getElements();
          const uint8* srcEnd = src + header.bitmapDataSize;
          ColorPixel* elements = image->getElements();
          unsigned int row = 0;
          unsigned int column = 0;

          while (src < srcEnd) {
            unsigned int first = *src++;
            unsigned int second = *src++;

            if (first == 0) { // escape follows - or absolute mode
              switch (second) {
              case 0: // end of line
                ++row;
                column = 0;
                break;
              case 1: // end of image
                row = dimension.getHeight();
                column = 0;
                break;
              case 2: // delta
                column += *src++;
                row += *src++;
                break;
              default: // absolute mode
                ColorPixel* dest = elements + dimension.getWidth() * row + column;
                for (unsigned int count = second >> 1; count > 0; --count) {
                  unsigned int value = *src++;
                  bassert(
                    ((value >> 4) < numberOfColors) && ((value & 0x0f) < numberOfColors),
                    bindCause(InvalidFormat("Color table index out of range", this), ImageEncoder::INVALID_COLOR)
                  );
                  *dest++ = palette[value >> 4];
                  *dest++ = palette[value & 0x0f];
                }
                if (second & 0x01) { // set last pixel and skip to word boundary
                  unsigned int index = *src++ >> 4;
                  bassert(
                    index < numberOfColors,
                    bindCause(InvalidFormat("Color table index out of range", this), ImageEncoder::INVALID_COLOR)
                  );
                  *dest++ = palette[index];
                  ++src;
                }
                column += second;
              }
            } else {
              ColorPixel* dest = elements + dimension.getWidth() * row + column;
              bassert(
                ((second >> 4) < numberOfColors) && ((second & 0x0f) < numberOfColors),
                bindCause(InvalidFormat("Color table index out of range", this), ImageEncoder::INVALID_COLOR)
              );
              ColorPixel color = palette[second >> 4];
              ColorPixel previousColor = palette[second & 0x0f];
              const ColorPixel* end = dest + first;
              while (dest < end) {
                *dest++ = color;
                swapper(color, previousColor);
              }
              column += first;
            }
          }
        }
        break;
      }
      return image;
    }
    return 0;
  }

  void BMPEncoder::write(const String& filename, const ColorImage* image) {
    BMPHeader header;

    File file(filename, File::WRITE, File::CREATE);

    unsigned int bytesPerRow = image->getDimension().getWidth() * 3;
    unsigned int bytesPerPaddedRow = (bytesPerRow + 3)/4*4;
    unsigned long long sizeOfFile = sizeof(header) + bytesPerPaddedRow * image->getDimension().getHeight();
    unsigned int zeroPad = bytesPerPaddedRow - bytesPerRow;

    bassert(
      sizeOfFile <= 0xffffffff,
      bindCause(ImageException("Dimension of image exceeds limit supported by encoder", this), ImageEncoder::DIMENSION_NOT_SUPPORTED)
    );

    header.identifier[0] = 'B';
    header.identifier[1] = 'M';
    header.fileSize = sizeOfFile;
    header.reserved = 0;
    header.bitmapDataOffset = sizeof(header); // only without palette
    header.bitmapHeaderSize = 0x28;
    header.width = image->getDimension().getWidth();
    header.height = image->getDimension().getHeight();
    header.planes = 1;
    header.bitsPerPixel = 24;
    header.compression = 0;
    header.bitmapDataSize = sizeOfFile - header.bitmapDataOffset;
    header.horizontalResolution = 0;
    header.verticalResolution = 0;
    header.colorsUsed = 0;
    header.importantColors = 0;

    file.write(Cast::getAddress(header), sizeof(header));

    const ColorPixel* sourceElement = image->getElements();
    Allocator<uint8> buffer(16 * 1024);
    uint8* beginOfBuffer = buffer.getElements();
    uint8* endOfBuffer = beginOfBuffer + buffer.getSize();
    uint8* p = beginOfBuffer;

    for (unsigned int row = 0; row < header.height; ++row) {
      const ColorPixel* endOfSourceRow = sourceElement + header.width;
      while (sourceElement < endOfSourceRow) {

        unsigned int pixelCount = (endOfBuffer - p)/3;
        if (pixelCount == 0) {
          file.write(beginOfBuffer, p - beginOfBuffer); // empty buffer
          p = beginOfBuffer;
          pixelCount = (endOfBuffer - p)/3;
        }

        const ColorPixel* endOfBlock = minimum(sourceElement + pixelCount, endOfSourceRow);
        while (sourceElement < endOfBlock) {
          ColorPixel pixel = *sourceElement++;
          *p++ = pixel.blue;
          *p++ = pixel.green;
          *p++ = pixel.red;
        }
      }

      if (((endOfBuffer - p)/3) == 0) {
        file.write(beginOfBuffer, p - beginOfBuffer); // empty buffer
        p = beginOfBuffer;
      }

      switch (zeroPad) {
      case 3:
        *p++ = 0;
      case 2:
        *p++ = 0;
      case 1:
        *p++ = 0;
      }
    }

    file.write(beginOfBuffer, p - beginOfBuffer); // empty buffer
    file.truncate(sizeOfFile);
  }

  void BMPEncoder::writeGray(const String& filename, const GrayImage* image) {
    BMPHeader header;

    File file(filename, File::WRITE, File::CREATE);

    unsigned int bytesPerRow = image->getDimension().getWidth();
    unsigned int bytesPerPaddedRow = (bytesPerRow + 3)/4*4;
    unsigned long long sizeOfFile = sizeof(header) + 256 * sizeof(BMPPaletteEntry) +
      bytesPerPaddedRow * image->getDimension().getHeight();
    unsigned int zeroPad = bytesPerPaddedRow - bytesPerRow;

    bassert(
      sizeOfFile <= 0xffffffff,
      bindCause(
        ImageException("Dimension of image exceeds limit supported by encoder", this),
        ImageEncoder::DIMENSION_NOT_SUPPORTED
      )
    );

    header.identifier[0] = 'B';
    header.identifier[1] = 'M';
    header.fileSize = sizeOfFile;
    header.reserved = 0;
    header.bitmapDataOffset = sizeof(header) + 256 * sizeof(BMPPaletteEntry);
    header.bitmapHeaderSize = 0x28;
    header.width = image->getDimension().getWidth();
    header.height = image->getDimension().getHeight();
    header.planes = 1;
    header.bitsPerPixel = 8;
    header.compression = 0;
    header.bitmapDataSize = sizeOfFile - header.bitmapDataOffset;
    header.horizontalResolution = 0;
    header.verticalResolution = 0;
    header.colorsUsed = 256;
    header.importantColors = 256;

    file.write(Cast::getAddress(header), sizeof(header));
    
    Allocator<uint8> buffer(image->getDimension().getWidth() * 16);
    
    // make gray palette
    BMPPaletteEntry palette[256];
    for (unsigned int i = 0; i < 256; ++i) {
      palette[i].blue = i;
      palette[i].green = i;
      palette[i].red = i;
      palette[i].reserved = 0;
    }
    file.write(Cast::getAddress(palette), sizeof(palette)); // store palette

    const GrayPixel* sourceElement = image->getElements();
    uint8* beginOfBuffer = buffer.getElements();
    uint8* endOfBuffer = beginOfBuffer + buffer.getSize();
    uint8* p = beginOfBuffer;

    for (unsigned int row = 0; row < header.height; ++row) {

      const GrayPixel* endOfSourceRow = sourceElement + header.width;
      while (sourceElement < endOfSourceRow) {

        unsigned int pixelCount = endOfBuffer - p;
        if (pixelCount == 0) {
          file.write(beginOfBuffer, p - beginOfBuffer); // empty buffer
          p = beginOfBuffer;
          pixelCount = endOfBuffer - p;
        }

        const GrayPixel* endOfBlock = minimum(sourceElement + pixelCount, endOfSourceRow);
        for ( ; sourceElement < endOfBlock; ++sourceElement) {
          *p++ = *sourceElement;
        }
      }

      if (endOfBuffer - p == 0) {
        file.write(beginOfBuffer, p - beginOfBuffer); // empty buffer
        p = beginOfBuffer;
      }

      switch (zeroPad) {
      case 3:
        *p++ = 0;
      case 2:
        *p++ = 0;
      case 1:
        *p++ = 0;
      }
    }

    file.write(beginOfBuffer, p - beginOfBuffer); // empty buffer
    file.truncate(sizeOfFile);
  }

  ArrayMap<String, AnyValue> BMPEncoder::getInformation(const String& filename)
  {
    BMPHeader header;
    
    {
      File file(filename, File::READ, 0);
      file.read(Cast::getAddress(header), sizeof(header));
    }
    
    return {
      {MESSAGE("encoder"), Type::getType(*this)},
      {MESSAGE("description"), MESSAGE("Windows Bitmap File Format")},
      {MESSAGE("width"), static_cast<unsigned int>(header.width)},
      {MESSAGE("height"), static_cast<unsigned int>(header.height)},
      {MESSAGE("planes"), static_cast<unsigned int>(header.planes)},
      {MESSAGE("bits per pixel"), static_cast<unsigned int>(header.bitsPerPixel)},
      {MESSAGE("compression"), static_cast<unsigned int>(header.compression)},
      {MESSAGE("horizontal resolution"), static_cast<unsigned int>(header.horizontalResolution)},
      {MESSAGE("vertical resolution"), static_cast<unsigned int>(header.verticalResolution)},
      {MESSAGE("colors"), static_cast<unsigned int>(header.colorsUsed)},
      {MESSAGE("important colors"), static_cast<unsigned int>(header.importantColors)}
    };
  }

}; // end of gip namespace
