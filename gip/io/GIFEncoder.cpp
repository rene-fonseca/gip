/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/GIFEncoder.h>
#include <base/mem/Allocator.h>
#include <base/io/File.h>
#include <base/concurrency/Thread.h>
#include <base/Primitives.h>
#include <base/ByteOrder.h>

using namespace base;

namespace gip {

  namespace GIFImpl {

    struct Header {
      char signature[3]; // 'GIF'
      char version[3]; // '87a' or '89a'
    } __attribute__ ((packed));

    struct DataSubBlock {
      byte size; // size of the block in bytes
      byte data[255]; // the data
    } __attribute__ ((packed));

    const byte TERMINATOR = 0x00; // terminates stream of data blocks
    const byte TRAILER = 0x3b; // indicates end of data stream
    const byte IMAGESEPARATOR = 0x2c; // indicates beginning of image

    struct LogicalScreenDescriptor {
      LittleEndian::UnsignedShort width; // logical screen width
      LittleEndian::UnsignedShort height; // logical screen height
      unsigned int entriesOfColorTable : 3; // number of entries is 2^(value + 1)
      bool sortedColorTable: 1; // true if the colors are sorted
      unsigned int colorResolution : 3; //
      bool colorTable : 1; // true if table is present
      byte backGroundColorIndex; // index to global color table
      byte aspectRatio; // actual ratio = (aspectRatio + 15) / 64
    } __attribute__ ((packed));

    struct ImageDescriptor {
      byte separator; // fixed value of ImageSeperator
      LittleEndian::UnsignedShort left; // column in pixels in respect to left edge of logical screen
      LittleEndian::UnsignedShort top; // row in pixels in respect to top of logical screen
      LittleEndian::UnsignedShort width; // width of image in pixels
      LittleEndian::UnsignedShort height; // height of image in pixels
      unsigned int entriesOfColorTable : 3; // number of entries is 2^(value + 1)
      unsigned int reserved : 2; // guess what
      bool sortedColorTable : 1; // true if the colors are sorted after importance
      bool interlaced : 1; // true if image is interlaced
      bool colorTable : 1; // true if color table is present
    } __attribute__ ((packed));

    struct ColorEntry {
      byte red;
      byte green;
      byte blue;
    } __attribute__ ((packed));



    class ReadGIF {
    private:

      File file;
      unsigned int currentCodeSize;
      unsigned int bitsAvailable;
      unsigned int unreadBits;
      unsigned int dataBlockIndex;
      byte dataBlockSize;
      byte dataBlockData[255];
    public:

      inline unsigned int getNextCode() throw() {
        static const unsigned int mask[13] = {0x000, 0x001, 0x003, 0x007, 0x00f, 0x01f, 0x03f, 0x07f, 0x0ff, 0x1ff, 0x3ff, 0x7ff, 0xfff};

        while (bitsAvailable < currentCodeSize) {
          while (dataBlockIndex == dataBlockSize) { // data block is allowed to be empty
            file.read(getCharAddress(dataBlockSize), sizeof(dataBlockSize)); // get size of data block
            file.read(getCharAddress(dataBlockData), dataBlockSize);
            dataBlockIndex = 0;
          }
          byte temp = dataBlockData[dataBlockIndex++];
          unreadBits |= static_cast<unsigned int>(temp) << bitsAvailable;
          bitsAvailable += 8;
        }
        unsigned int code = unreadBits & mask[currentCodeSize]; // only copy the right number of bits
        unreadBits >>= currentCodeSize; // remove code bits from buffer
        bitsAvailable -= currentCodeSize;
        return code;
      }

      void readImage(ColorImage& image, const ColorPixel* colorTable, bool interlaced) throw(InvalidFormat, IOException) {
        static const unsigned int lookupRowIndex[4] = {0, 4, 2, 1};
        static const unsigned int lookupRowStep[4] = {8, 8, 4, 2};
        unsigned int width = image.getWidth();
        unsigned int height = image.getHeight();

        unsigned int columnIndex = 0;
        unsigned int rowIndex = interlaced ? lookupRowIndex[0] : 0;
        unsigned int rowStep = interlaced ? lookupRowStep[0] : 1;
        unsigned int nextPass = 1;
        ColorPixel* elements = image.getElements();
        ColorPixel* row = elements + rowIndex * width;
        bool done = false;

        byte LZWCodeSize;
        file.read(getCharAddress(LZWCodeSize), sizeof(LZWCodeSize)); // get LZW minimum code size
        assert((LZWCodeSize >= 2) && (LZWCodeSize <= 9), InvalidFormat("Invalid GIF format", this));

        const unsigned int clearCode = 1 << LZWCodeSize; // the clear code
        const unsigned int highCode = clearCode - 1; // set the highest code not needing decoding
        const unsigned int endCode = clearCode + 1; // set the ending code
        const unsigned int firstSlot = clearCode + 2; //
        const unsigned int initialCodeSize = LZWCodeSize + 1;

        byte decodeStack[4096]; // stack for decoded codes
        unsigned int prefix[4096]; // code prefixes
        unsigned int suffix[4096]; // code suffixes
        unsigned int stackIndex = 0;

        currentCodeSize = initialCodeSize; // set the initial code size
        dataBlockSize = 0;
        dataBlockIndex = 0;
        bitsAvailable = 0;
        unreadBits = 0;
        bool limitReached = false;

        unsigned int code;
        unsigned int oldCode;
        unsigned int slot;
        unsigned int topSlot;

        while (!done) {
          code = getNextCode();
          assert(!limitReached || (code == clearCode), InvalidFormat("Invalid GIF format", this));
          limitReached = false;
          if (code == endCode) { // stop on end code
            break;
          } else if (code == clearCode) {
            currentCodeSize = initialCodeSize; // reset code size
            slot = firstSlot; // reset slot
            topSlot = 1 << currentCodeSize; // set max slot number
            do { // remove all clear codes
              code = getNextCode();
            } while (code == clearCode);
            assert(code != endCode, InvalidFormat("Invalid GIF format", this)); // ending code after a clear code
//        if (code >= slot) { // if beyond preset codes then set to zero
//          code = 0;
//        }
            oldCode = code;
            decodeStack[stackIndex++] = code; // output code to decoded stack
          } else { // code is data
            unsigned int C = code;
            if (C < slot) { // is the code in the table
              while (C > highCode) { // decode the code
                decodeStack[stackIndex++] = suffix[C];
                C = prefix[C];
              }
              decodeStack[stackIndex++] = C;
              if (slot < topSlot) {
                suffix[slot] = C;
                prefix[slot] = oldCode;
                ++slot;
                oldCode = code;
              }
            } else { // the code is not in the table
              assert(C == slot, InvalidFormat("Invalid GIF format", this));
              unsigned int tempCode = oldCode;
              while (oldCode > highCode) { // translate the old code
                decodeStack[stackIndex] = suffix[oldCode];
                oldCode = prefix[oldCode];
              }
              decodeStack[stackIndex] = oldCode;
              if (slot < topSlot) {
                suffix[slot] = oldCode;
                prefix[slot] = tempCode;
                ++slot;
              }
              while (C > highCode) { // decode the code
                decodeStack[stackIndex++] = suffix[C];
                C = prefix[C];
              }
              decodeStack[stackIndex++] = C;
              oldCode = code;
            }
            if (slot >= topSlot) {
              if (currentCodeSize < 12) {
                topSlot <<= 1;
                ++currentCodeSize;
              } else {
                limitReached = true;
              }
            }
          }

          while (stackIndex > 0) { // pop the decoded data of the stack
            row[columnIndex++] = colorTable[decodeStack[--stackIndex]];
            if (columnIndex >= width) { // has the entire row been read
              columnIndex = 0;
              rowIndex += rowStep;
              if (rowIndex >= height) {
                rowIndex = lookupRowIndex[nextPass];
                rowStep = lookupRowStep[nextPass];
                ++nextPass;
                done = interlaced ? (nextPass > 4) : true;
                if (done) {
                  break;
                }
              }
              row = elements + rowIndex * width;
            }
          }
        }
      }

      ReadGIF(File f, ColorImage& image, const ColorPixel* colorTable, bool interlaced) : file(f) {
        readImage(image, colorTable, interlaced);
      }
    };

  };



  GIFEncoder::GIFEncoder() throw() {
    ASSERT(sizeof(GIFImpl::Header) == 6);
    ASSERT(sizeof(GIFImpl::LogicalScreenDescriptor) == 7);
  }

  String GIFEncoder::getDescription() const throw() {
    return MESSAGE("Graphics Interchange Format");
  }

  String GIFEncoder::getDefaultExtension() const throw() {
    return MESSAGE("gif");
  }

  bool GIFEncoder::isValid(const String& filename) throw(IOException) {
    GIFImpl::Header header;
    unsigned int size;

    {
      File file(filename, File::READ, 0);
      file.read(getCharAddress(header), sizeof(header));
      size = file.getSize();
    }

    return (header.signature[0] == 'G') && (header.signature[1] == 'I') && (header.signature[2] == 'F') &&
      ((header.version[0] == '8') && (header.version[1] == '7') && (header.version[2] == 'a') ||
       (header.version[0] == '8') && (header.version[1] == '9') && (header.version[2] == 'a'));
  }

  ColorImage* GIFEncoder::read(const String& filename) throw(InvalidFormat, IOException) {

    File file(filename, File::READ, 0);

    GIFImpl::Header header;
    file.read(getCharAddress(header), sizeof(header));

    bool value = (header.signature[0] == 'G') && (header.signature[1] == 'I') && (header.signature[2] == 'F') &&
      ((header.version[0] == '8') && (header.version[1] == '7') && (header.version[2] == 'a') ||
       (header.version[0] == '8') && (header.version[1] == '9') && (header.version[2] == 'a'));
    assert(value, InvalidFormat("Invalid GIF format", this));

    GIFImpl::LogicalScreenDescriptor globalDescriptor;
    file.read(getCharAddress(globalDescriptor), sizeof(globalDescriptor));

    ColorPixel globalColorTable[256];
    unsigned int globalColors = 1 << (globalDescriptor.entriesOfColorTable + 1);
    if (globalDescriptor.colorTable) {
      GIFImpl::ColorEntry tempTable[256]; // number of entries cannot exceed 256
      file.read(getCharAddress(tempTable), sizeof(GIFImpl::ColorEntry) * globalColors);
      for (unsigned int i = 0; i < globalColors; ++i) {
        GIFImpl::ColorEntry src = tempTable[i];
        ColorPixel dest;
        dest.red = src.red;
        dest.green = src.green;
        dest.blue = src.blue;
        globalColorTable[i] = dest;
      }
    }

    GIFImpl::ImageDescriptor imageDescriptor;
    file.read(getCharAddress(imageDescriptor), sizeof(imageDescriptor));
    assert(imageDescriptor.separator == GIFImpl::IMAGESEPARATOR, InvalidFormat("Invalid GIF format", this));

    ColorPixel localColorTable[256];
    unsigned int localColors = 1 << (imageDescriptor.entriesOfColorTable + 1);
    if (imageDescriptor.colorTable) {
      GIFImpl::ColorEntry tempTable[256]; // number of entries cannot exceed 256
      file.read(getCharAddress(tempTable), sizeof(GIFImpl::ColorEntry) * localColors);
      for (unsigned int i = 0; i < localColors; ++i) {
        GIFImpl::ColorEntry src = tempTable[i];
        ColorPixel dest;
        dest.red = src.red;
        dest.green = src.green;
        dest.blue = src.blue;
        localColorTable[i] = dest;
      }
    }

    ColorPixel* colorTable = (imageDescriptor.colorTable) ? &localColorTable[0] : &globalColorTable[0];
    ColorImage image(Dimension(imageDescriptor.width, imageDescriptor.height));

    GIFImpl::ReadGIF read(file, image, colorTable, imageDescriptor.interlaced);

    byte terminator;
    file.read(getCharAddress(terminator), sizeof(terminator)); // check terminator
    assert(terminator == GIFImpl::TERMINATOR, InvalidFormat("Invalid GIF format", this));

    byte trailer;
    file.read(getCharAddress(trailer), sizeof(trailer)); // check trailer
    assert(trailer == GIFImpl::TRAILER, InvalidFormat("Invalid GIF format", this));

    return new ColorImage(image);
  }

  void GIFEncoder::write(const String& filename, const ColorImage* image) throw(IOException) {
    GIFImpl::Header header;

    File file(filename, File::WRITE, File::CREATE);

    header.signature[0] = 'G';
    header.signature[1] = 'I';
    header.signature[2] = 'F';
    header.version[0] = '8';
    header.version[1] = '7';
    header.version[2] = 'a';

    file.write(getCharAddress(header), sizeof(header));
    file.truncate(sizeof(header));
  }

  FormatOutputStream& GIFEncoder::getInfo(FormatOutputStream& stream, const String& filename) throw(IOException) {
    GIFImpl::Header header;

    {
      File file(filename, File::READ, 0);
      file.read(getCharAddress(header), sizeof(header));
    }

    stream << MESSAGE("GIFEncoder (Graphics Interchange Format):") << EOL
           << MESSAGE("  signature=") << header.signature[0] << header.signature[1] << header.signature[2] << EOL
           << MESSAGE("  version=") << header.version[0] << header.version[1] << header.version[2] << EOL;
    return stream;
  }

}; // end of namespace
