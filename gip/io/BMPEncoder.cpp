/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

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
#include <base/Type.h>
#include <base/ByteOrder.h>

using namespace base;

namespace gip {

class FullPalette : public UnaryOperation<byte, ColorPixel> {
private:
  ColorPixel* palette;
public:
  inline FullPalette(ColorPixel* p) : palette(p) {}
  inline Result operator()(Argument value) const throw() {return palette[value];}
};

class PartialPalette : public UnaryOperation<byte, ColorPixel> {
private:
  ColorPixel* palette;
  unsigned int maxIndex;
public:
  inline PartialPalette(ColorPixel* p, unsigned int m) : palette(p), maxIndex(m) {}
  inline Result operator()(Argument value) const throw() {
    assert(value < maxIndex, InvalidFormat("Color table index out of range"));
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

typedef struct {
  char identifier[2]; // the characters identifying the bitmap
  LittleEndian::UnsignedInt fileSize; // complete file size in bytes
  LittleEndian::UnsignedInt reserved; // reserved for later use
  LittleEndian::UnsignedInt bitmapDataOffset; // offset from beginning of file to the beginning of the bitmap data
  LittleEndian::UnsignedInt bitmapHeaderSize; // size of the header used to describe the bitmap colors and compression
  LittleEndian::UnsignedInt width; // width of bitmap in pixels
  LittleEndian::UnsignedInt height; // height of bitmap in pixels
  LittleEndian::UnsignedShort planes; // number of planes in this bitmap
  LittleEndian::UnsignedShort bitsPerPixel; // bits per pixel used to store palette entry information
  LittleEndian::UnsignedInt compression; // specifies the compression
  LittleEndian::UnsignedInt bitmapDataSize; // size of the bitmap data in bytes
  LittleEndian::UnsignedInt horizontalResolution; // horizontal resolution expressed in pixels per meter
  LittleEndian::UnsignedInt verticalResolution; // vertical resolution expressed in pixels per meter
  LittleEndian::UnsignedInt colorsUsed; // number of colors used by this bitmap
  LittleEndian::UnsignedInt importantColors; // number of important colors
} __attribute__ ((packed)) BMPHeader;

typedef unsigned char byte;

typedef struct {
  byte blue;
  byte green;
  byte red;
  byte reserved; // must be zero
} __attribute__ ((packed)) BMPPaletteEntry;

BMPEncoder::BMPEncoder(const String& f) throw() : filename(f) {
}

String BMPEncoder::getDescription() const throw() {
  return MESSAGE("Windows Bitmap File Format");
}

String BMPEncoder::getDefaultExtension() const throw() {
  return MESSAGE("bmp");
}

bool BMPEncoder::isValid() throw(IOException) {
  BMPHeader header;
  unsigned int size;

  {
    File file(filename, File::READ, 0);
    file.read((char*)&header, sizeof(header));
    size = file.getSize();
  }

  bool result = ((header.identifier[0] == 'B') && (header.identifier[1] == 'M') &&
                 (header.fileSize == size) &&
                 (header.bitmapHeaderSize == 0x28) &&
                 (header.width > 0) && (header.height > 0));

  switch (header.compression) {
  case BI_RGB:
    if (
      (header.planes == 1) && (header.bitsPerPixel == 1) ||
      (header.planes == 1) && (header.bitsPerPixel == 4) ||
      (header.planes == 1) && (header.bitsPerPixel == 8) ||
      (header.planes == 1) && (header.bitsPerPixel == 24) ||
      (header.planes == 1) && (header.bitsPerPixel == 32) ||
      (header.planes == 3) && (header.bitsPerPixel == 24) ||
      (header.planes == 3) && (header.bitsPerPixel == 32)) {
      break;
    }
    result = false;
  case BI_RLE8:
    if ((header.planes == 1) && (header.bitsPerPixel == 8)) {
      break;
    }
    result = false;
  case BI_RLE4:
    if ((header.planes == 1) && (header.bitsPerPixel == 4)) {
      break;
    }
    result = false;
  default:
    result = false;
  }

  return result;
}

ColorImage* BMPEncoder::read() throw(IOException) {
  File file(filename, File::READ, 0);

  BMPHeader header;
  file.read(header);

  if (
    (header.identifier[0] == 'B') && (header.identifier[1] == 'M') &&
    (header.bitmapHeaderSize == 0x28) && // must be dword aligned
    (header.width > 0) && (header.height > 0)
  ) {

    switch (header.compression) {
    case BI_RGB:
      if (
        (header.planes == 1) && (header.bitsPerPixel == 1) ||
        (header.planes == 1) && (header.bitsPerPixel == 4) ||
        (header.planes == 1) && (header.bitsPerPixel == 8) ||
        (header.planes == 1) && (header.bitsPerPixel == 24) ||
        (header.planes == 1) && (header.bitsPerPixel == 32) ||
        (header.planes == 3) && (header.bitsPerPixel == 24) ||
        (header.planes == 3) && (header.bitsPerPixel == 32)) {
        break;
      }
      return 0;
    case BI_RLE8:
      if ((header.planes == 1) && (header.bitsPerPixel == 8)) {
        break;
      }
      return 0;
    case BI_RLE4:
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
      file.setPosition(header.bitmapHeaderSize, File::BEGIN); // move to beginning of palette
      if (header.bitsPerPixel <= 8) { // should palette be used
        unsigned int maximumNumberOfColors = 1 << header.bitsPerPixel;
        assert(numberOfColors < maximumNumberOfColors, InvalidFormat());
        if (numberOfColors == 0) { // use maximum number of colors
          numberOfColors = maximumNumberOfColors;
        }
        // MappingSequence<BMPPaletteEntry> map(file, header.bitmapHeaderSize, numberOfColors);
        // BMPPaletteEntry* srcPalette = map.getElements();
        BMPPaletteEntry srcPalette[numberOfColors];
        file.read((char*)&srcPalette, sizeof(srcPalette)); // get palette
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
    case BI_RGB: // no compression
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
                Intensity blue = *src++;
                Intensity green = *src++;
                Intensity red = *src++;
                *dest++ = makeRGBPixel(blue, green, red); // order of args is blue, green, and red
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
                Intensity blue = *src++;
                Intensity green = *src++;
                Intensity red = *src++;
                ++src;
                *dest++ = makeRGBPixel(blue, green, red); // order of args is blue, green, and red
              }
              reader.skip(bytesPerLine);
            }
          }
          break;
        }
      }
      break;
    case BI_RLE8: // 8 bit run-length encoding
      {
        Allocator<byte> buffer(header.bitmapDataSize); // all image data
        file.read((char*)buffer.getElements(), header.bitmapDataSize); // read all image data
        const byte* src = buffer.getElements();
        const byte* srcEnd = src + header.bitmapDataSize;
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
                assert(index < numberOfColors, InvalidFormat("Color table index out of range"));
                *dest++ = palette[index];
              }
              if (second & 0x01) { // skip to word boundary
                ++src;
              }
              column += second;
            }
          } else {
            assert(second < numberOfColors, InvalidFormat("Color table index out of range"));
            fill<ColorPixel>(elements + dimension.getWidth() * row + column, first, palette[second]);
            column += first;
          }
        }
      }
      break;
    case BI_RLE4: // 4 bit run-length encoding
      {
        Allocator<byte> buffer(header.bitmapDataSize); // all image data
        file.read((char*)buffer.getElements(), header.bitmapDataSize); // read all image data
        const byte* src = buffer.getElements();
        const byte* srcEnd = src + header.bitmapDataSize;
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
                assert(
                  ((value >> 4) < numberOfColors) && ((value & 0x0f) < numberOfColors),
                  InvalidFormat("Color table index out of range")
                );
                *dest++ = palette[value >> 4];
                *dest++ = palette[value & 0x0f];
              }
              if (second & 0x01) { // set last pixel and skip to word boundary
                unsigned int index = *src++ >> 4;
                assert(index < numberOfColors, InvalidFormat("Color table index out of range"));
                *dest++ = palette[index];
                ++src;
              }
              column += second;
            }
          } else {
            ColorPixel* dest = elements + dimension.getWidth() * row + column;
            assert(
              ((second >> 4) < numberOfColors) && ((second & 0x0f) < numberOfColors),
              InvalidFormat("Color table index out of range")
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

void BMPEncoder::write(const ColorImage* image) throw(IOException) {
  BMPHeader header;

  File file(filename, File::WRITE, File::CREATE);

  unsigned int bytesPerRow = image->getDimension().getWidth() * 3;
  unsigned int bytesPerPaddedRow = (bytesPerRow + 3)/4*4;
  unsigned long long sizeOfFile = sizeof(header) + bytesPerPaddedRow * image->getDimension().getHeight();
  unsigned int zeroPad = bytesPerPaddedRow - bytesPerRow;

  assert(sizeOfFile <= 0xffffffff, IOException("Dimension of image exceeds limit supported by encoder"));

  header.identifier[0] = 'B';
  header.identifier[1] = 'M';
  header.fileSize = sizeOfFile;
  header.reserved = 0;
  header.bitmapDataOffset = 0x36; // only without palette
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

  file.write((const char*)&header, sizeof(header));

  const ColorPixel* sourceElement = image->getElements();
  Allocator<char>* buffer = Thread::getLocalStorage();
  char* beginOfBuffer = buffer->getElements();
  char* endOfBuffer = beginOfBuffer + buffer->getSize();
  char* p = beginOfBuffer;

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

void BMPEncoder::writeGray(const GrayImage* image) throw(IOException) {
  BMPHeader header;

  File file(filename, File::WRITE, File::CREATE);

  unsigned int bytesPerRow = image->getDimension().getWidth();
  unsigned int bytesPerPaddedRow = (bytesPerRow + 3)/4*4;
  unsigned long long sizeOfFile = sizeof(header) + 256*4 + bytesPerPaddedRow * image->getDimension().getHeight();
  unsigned int zeroPad = bytesPerPaddedRow - bytesPerRow;

  assert(sizeOfFile <= 0xffffffff, IOException("Dimension of image exceeds limit supported by encoder"));

  header.identifier[0] = 'B';
  header.identifier[1] = 'M';
  header.fileSize = sizeOfFile;
  header.reserved = 0;
  header.bitmapDataOffset = 0x36 + 256 * 4;
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

  file.write((const char*)&header, sizeof(header));

  Allocator<unsigned char> buffer(maximum(image->getDimension().getWidth() * 16, (unsigned int)256 * 4));

  // make gray palette
  {
    unsigned char* p = buffer.getElements();
    for (unsigned int i = 0; i < 256; ++i) {
      *p++ = i; // blue
      *p++ = i; // green
      *p++ = i; // red
      *p++ = 0; // reserved
    }
    file.write((char*)buffer.getElements(), 256 * 4);
  }

  const GrayPixel* sourceElement = image->getElements();
  unsigned char* beginOfBuffer = buffer.getElements();
  unsigned char* endOfBuffer = beginOfBuffer + buffer.getSize();
  unsigned char* p = beginOfBuffer;

  for (unsigned int row = 0; row < header.height; ++row) {

    const GrayPixel* endOfSourceRow = sourceElement + header.width;
    while (sourceElement < endOfSourceRow) {

      unsigned int pixelCount = endOfBuffer - p;
      if (pixelCount == 0) {
        file.write((char*)beginOfBuffer, p - beginOfBuffer); // empty buffer
        p = beginOfBuffer;
        pixelCount = endOfBuffer - p;
      }

      const GrayPixel* endOfBlock = minimum(sourceElement + pixelCount, endOfSourceRow);
      for ( ; sourceElement < endOfBlock; ++sourceElement) {
        *p++ = *sourceElement;
      }
    }

    if (endOfBuffer - p == 0) {
      file.write((char*)beginOfBuffer, p - beginOfBuffer); // empty buffer
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

  file.write((char*)beginOfBuffer, p - beginOfBuffer); // empty buffer
  file.truncate(sizeOfFile);
}

FormatOutputStream& BMPEncoder::getInfo(FormatOutputStream& stream) throw(IOException) {
  BMPHeader header;

  {
    File file(filename, File::READ, 0);
    file.read((char*)&header, sizeof(header));
  }

  stream << "BMPEncoder (Windows Bitmap File Format):" << EOL
         << "  identifier=" << header.identifier[0] << header.identifier[1] << EOL
         << "  fileSize=" << header.fileSize << EOL
         << "  reserved=" << header.reserved << EOL
         << "  bitmapDataOffset=" << header.bitmapDataOffset << EOL
         << "  bitmapHeaderSize=" << header.bitmapHeaderSize << EOL
         << "  width=" << header.width << EOL
         << "  height=" << header.height << EOL
         << "  planes=" << header.planes << EOL
         << "  bitsPerPixel=" << header.bitsPerPixel << EOL
         << "  compression=" << header.compression << EOL
         << "  bitmapDataSize=" << header.bitmapDataSize << EOL
         << "  horizontalResolution=" << header.horizontalResolution << EOL
         << "  verticalResolution=" << header.verticalResolution << EOL
         << "  colors=" << header.colorsUsed << EOL
         << "  importantColors=" << header.importantColors << EOL;
  return stream;
}

}; // end of namespace
