/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/RASEncoder.h>
#include <base/mem/Allocator.h>
#include <base/io/File.h>
#include <base/io/FileReader.h>
#include <base/Primitives.h>
#include <base/ByteOrder.h>
#include <base/string/String.h>
#include <base/NotImplemented.h>

namespace gip {

  namespace RASEncoderImpl {

    enum {
       MAGIC = 0x59a66a95
    };

    enum {
      TYPE_OLD = 0,
      TYPE_STANDARD = 1,
      TYPE_BYTE_ENCODED = 2,
      TYPE_RGB = 3,
      TYPE_TIFF = 4,
      TYPE_IFF = 5,
      TYPE_EXPERIMENTAL = 0xffff,

      MAP_TYPE_NONE = 0,
      MAP_TYPE_RGB = 1,
      MAP_TYPE_RAW = 2
    };

    struct Header {
      BigEndian<uint32> magic;
      BigEndian<int32> width;
      BigEndian<int32> height;
      BigEndian<int32> depth;
      BigEndian<int32> length;
      BigEndian<int32> type;
      BigEndian<int32> mapType;
      BigEndian<int32> mapLength;
    } _DK_SDU_MIP__BASE__PACKED;
  };
  
  RASEncoder::RASEncoder() throw() {
  }

  String RASEncoder::getDescription() const throw() {
    return MESSAGE("Sun Rasterfile Format");
  }

  String RASEncoder::getDefaultExtension() const throw() {
    return MESSAGE("ras");
  }

  Array<String> RASEncoder::getExtensions() const throw() {
    Array<String> extensions;
    extensions.append(MESSAGE("ras"));
    extensions.append(MESSAGE("sun"));
    return extensions;
  }

  bool RASEncoder::isValid(const String& filename) throw(IOException) {
    RASEncoderImpl::Header header;

    File file(filename, File::READ, 0);
    if (file.getSize() < sizeof(header)) {
      return false;
    }
    file.read(Cast::getCharAddress(header), sizeof(header));

    if (header.magic != RASEncoderImpl::MAGIC) {
      return false;
    }

    switch (header.type) {
    case RASEncoderImpl::TYPE_OLD:
    case RASEncoderImpl::TYPE_STANDARD:
    case RASEncoderImpl::TYPE_BYTE_ENCODED:
    case RASEncoderImpl::TYPE_RGB:
    case RASEncoderImpl::TYPE_TIFF:
    case RASEncoderImpl::TYPE_IFF:
      break;
    case RASEncoderImpl::TYPE_EXPERIMENTAL: // not supported
    default:
      return false;
    }

    switch (header.mapType) {
    case RASEncoderImpl::MAP_TYPE_NONE:
      break;
    case RASEncoderImpl::MAP_TYPE_RAW:
      break;
    case RASEncoderImpl::MAP_TYPE_RGB:
      if (header.mapLength % 3 == 0) {
        break;
      }
    default:
      return false;
    }

    if (!((header.depth == 1) || (header.depth == 8) || (header.depth == 24) || (header.depth == 32))) {
      return false;
    }

    return true;
  }
  
  ColorImage* RASEncoder::read(const String& filename) throw(InvalidFormat, IOException) {
    RASEncoderImpl::Header header;

    File file(filename, File::READ, 0);
    file.read(Cast::getCharAddress(header), sizeof(header));

    assert((header.width >= 0) && (header.height >= 0), InvalidFormat(this));
    const Dimension dimension(header.width, header.height);
    ColorImage image(dimension);
    
    FileReader reader(file, sizeof(header));

    bool useColorMap = false;
    unsigned int colorMapEntries = 0;
    ColorPixel colorMap[256];
    switch (header.mapType) {
    case RASEncoderImpl::MAP_TYPE_NONE:
      break;
    case RASEncoderImpl::MAP_TYPE_RGB:
      if (header.mapLength % 3 == 0) {
        assert(header.mapLength >= 0, InvalidFormat(this));
        const unsigned int bytesInVectors = minimum<int>(header.mapLength, 256*3); // we only want the first 256 vectors of each color map
        colorMapEntries = bytesInVectors/3;
        FileReader::ReadIterator vectorRed = reader.peek(bytesInVectors);
        for (unsigned int i = 0; i < colorMapEntries; ++i) {
          colorMap[i].red = *vectorRed++;
        }
        reader.skip(header.mapLength/3); // skip red color map
        FileReader::ReadIterator vectorGreen = reader.peek(bytesInVectors);
        for (unsigned int i = 0; i < colorMapEntries; ++i) {
          colorMap[i].green = *vectorGreen++;
        }
        reader.skip(header.mapLength/3); // skip green color map
        FileReader::ReadIterator vectorBlue = reader.peek(bytesInVectors);
        for (unsigned int i = 0; i < colorMapEntries; ++i) {
          colorMap[i].blue = *vectorBlue++;
        }
        reader.skip(header.mapLength/3); // skip blue color map
        useColorMap = true;
        break; // accept color map
      }
    case RASEncoderImpl::MAP_TYPE_RAW:
      // TAG: fixme
    default:
      throw InvalidFormat(this);
    }

    ColorImage::Rows::RowIterator row = image.getRows().getEnd();
    const ColorImage::Rows::RowIterator endRow = image.getRows().getFirst();
    ColorPixel* dest = image.getElements();
    unsigned int bytesPerLine = ((dimension.getWidth() * (header.depth >> 3) + 1)/2)*2; // aligned to 16 bit boundary

    switch (header.type) {
    case RASEncoderImpl::TYPE_OLD:
      if (header.length == 0) {
        header.length = header.width * header.height * (header.depth >> 3);
      }
    case RASEncoderImpl::TYPE_STANDARD: // read bgr
      switch (header.depth) {
      case 8:
        if (useColorMap) {
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              *column++ = colorMap[*src++];
            }
            reader.skip(bytesPerLine);
          }
        } else { // no color map
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              *column++ = makeColorPixel(*src, *src, *src);
              ++src;
            }
            reader.skip(bytesPerLine);
          }
        }
        break;
      case 24:
        if (useColorMap) {
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              ColorPixel result;
              result.blue = colorMap[*src++].blue;
              result.green = colorMap[*src++].green;
              result.red = colorMap[*src++].red;
              *column++ = result;
            }
            reader.skip(bytesPerLine);
          }
        } else { // no color map
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            const ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              ColorPixel result;
              result.blue = *src++;
              result.green = *src++;
              result.red = *src++;
              *column++ = result;
            }
            reader.skip(bytesPerLine);
          }
        }
        break;
      case 32:
        if (useColorMap) {
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              ++src; // skip padding - 32 bit alignment
              ColorPixel result;
              result.blue = colorMap[*src++].blue;
              result.green = colorMap[*src++].green;
              result.red = colorMap[*src++].red;
              *column++ = result;
            }
            reader.skip(bytesPerLine);
          }
        } else { // no color map
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              ++src; // skip padding - 32 bit alignment
              ColorPixel result;
              result.blue = *src++;
              result.green = *src++;
              result.red = *src++;
              *column++ = result;
            }
            reader.skip(bytesPerLine);
          }
        }
        break;
      default:
        throw InvalidFormat(this);
      }
      break;
    case RASEncoderImpl::TYPE_BYTE_ENCODED:
      {
        // TAG: if not color map then read blue, green, and red bytes
        assert(header.length > 0, InvalidFormat(this));
        unsigned int pixelsToWrite = dimension.getSize();
        unsigned int bytesToRead = header.length;
        while (pixelsToWrite > 0) {
          FileReader::ReadIterator src = reader.peek(header.length); // entire image data
          unsigned char value = *src++;
          if (value == 0x80) {
            value = *src++;
            if (value != 0) {
              unsigned int count = value + 1; // a run may cross the end of line
              assert(count <= pixelsToWrite, InvalidFormat(this));
              pixelsToWrite -= count;
              ColorPixel result = colorMap[*src++];
              for (unsigned int i = count; i > 0; --i) {
                *dest++ = result;
              }
            } else {
              *dest++ = colorMap[0x80];
              --pixelsToWrite;
            }
          } else {
            *dest++ = colorMap[value];
            --pixelsToWrite;
          }
          if (bytesToRead == bytesPerLine) {
            if (dimension.getWidth() & 1) {
              ++src; // skip padding - 16 bit alignment
              bytesToRead = 0;
            }
          } else if (bytesToRead > bytesPerLine) { // wrap
            bytesToRead %= bytesPerLine; // several lines may be written by one scan
          }
        }
      }
      break;
    case RASEncoderImpl::TYPE_RGB:
      switch (header.depth) {
      case 8:
        if (useColorMap) {
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              *column++ = colorMap[*src++];
            }
            reader.skip(bytesPerLine);
          }
        } else { // no color map
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              *column++ = makeColorPixel(*src, *src, *src);
              ++src;
            }
            reader.skip(bytesPerLine);
          }
        }
        break;
      case 24:
        if (useColorMap) {
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              ColorPixel result;
              result.red = colorMap[*src++].red;
              result.green = colorMap[*src++].green;
              result.blue = colorMap[*src++].blue;
              *column++ = result;
            }
            reader.skip(bytesPerLine);
          }
        } else { // no color map
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              ColorPixel result;
              result.red = *src++;
              result.green = *src++;
              result.blue = *src++;
              *column++ = result;
            }
            reader.skip(bytesPerLine);
          }
        }
        break;
      case 32:
        if (useColorMap) {
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              ++src; // skip padding - 32 bit alignment
              ColorPixel result;
              result.red = colorMap[*src++].red;
              result.green = colorMap[*src++].green;
              result.blue = colorMap[*src++].blue;
              *column++ = result;
            }
            reader.skip(bytesPerLine);
          }
        } else { // no color map
          while (row != endRow) {
            --row;
            FileReader::ReadIterator src = reader.peek(bytesPerLine);
            ColorImage::Rows::RowIterator::ElementIterator column = row.getFirst();
            ColorImage::Rows::RowIterator::ElementIterator endColumn = row.getEnd();
            while (column != endColumn) {
              ++src; // skip padding - 32 bit alignment
              ColorPixel result;
              result.red = *src++;
              result.green = *src++;
              result.blue = *src++;
              *column++ = result;
            }
            reader.skip(bytesPerLine);
          }
        }
        break;
      default:
        throw InvalidFormat(this);
      }
      break;
    case RASEncoderImpl::TYPE_TIFF:
    case RASEncoderImpl::TYPE_IFF:
    default:
      throw InvalidFormat(this);
    }

    return new ColorImage(image);
  }
  
  void RASEncoder::write(const String& filename, const ColorImage* image) throw(ImageException, IOException) {
    assert(image, NullPointer(this));
    const Dimension dimension = image->getDimension();
    assert(
      dimension.getSize() * 3 <= static_cast<unsigned int>(PrimitiveTraits<int>::MAXIMUM),
      ImageException(this)
    ); // make sure length fits in header.length

    RASEncoderImpl::Header header;
    clear(header);
    header.magic = RASEncoderImpl::MAGIC;
    header.width = dimension.getWidth();
    header.height = dimension.getHeight();
    header.depth = 24;
    header.length = dimension.getSize() * 3;
    header.type = RASEncoderImpl::TYPE_STANDARD;
    header.mapType = RASEncoderImpl::MAP_TYPE_NONE;
//    header.mapLength = 0;

    const unsigned int bytesPerLine = ((dimension.getWidth() * 3 + 1)/2)*2; // 16 bit alignment
    Allocator<char> buffer(
      (BUFFER_SIZE >= bytesPerLine) ? (BUFFER_SIZE/bytesPerLine) * bytesPerLine : bytesPerLine
    );
    
    File file(filename, File::WRITE, File::CREATE);
    file.write(Cast::getCharAddress(header), sizeof(header));
    
    const ColorImage::ReadableRows::RowIterator endRow = image->getRows().getFirst();
    ColorImage::ReadableRows::RowIterator row = image->getRows().getEnd();
    const Allocator<char>::Iterator endDest = buffer.getEndIterator();
    Allocator<char>::Iterator dest = buffer.getBeginIterator();
    
    while (row != endRow) {
      --row;
      const ColorImage::ReadableRows::RowIterator::ElementIterator endColumn = row.getEnd();
      ColorImage::ReadableRows::RowIterator::ElementIterator column = row.getFirst();
      while (column != endColumn) {
        *dest++ = column->blue;
        *dest++ = column->green;
        *dest++ = column->red;
        ++column;
      }
      if ((dimension.getWidth() * 3) & 1) { // do we need to pad
        *dest++ = 0; // pad to 16 bit boundary
      }
      if (dest == endDest) {
        file.write(buffer.getElements(), buffer.getSize()); // write entire buffer
        dest = buffer.getBeginIterator();
      }
    }
    if (dest != buffer.getBeginIterator()) {
      file.write(buffer.getElements(), dest - buffer.getBeginIterator()); // write entire buffer
    }
    file.truncate(sizeof(header) + static_cast<unsigned long long>(bytesPerLine) * dimension.getHeight());
  }

  void RASEncoder::writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException) {
    assert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    assert(
      dimension.getSize() <= static_cast<unsigned int>(PrimitiveTraits<int>::MAXIMUM),
      ImageException(this)
    ); // make sure length fits in header.length

    RASEncoderImpl::Header header;
    clear(header);
    header.magic = RASEncoderImpl::MAGIC;
    header.width = dimension.getWidth();
    header.height = dimension.getHeight();
    header.depth = 8;
    header.length = dimension.getSize() * 1;
    header.type = RASEncoderImpl::TYPE_STANDARD;
    header.mapType = RASEncoderImpl::MAP_TYPE_NONE;
//    header.mapLength = 0;

    const unsigned int bytesPerLine = ((dimension.getWidth() + 1)/2)*2; // 16 bit alignment
    Allocator<char> buffer(
      (BUFFER_SIZE >= bytesPerLine) ? (BUFFER_SIZE/bytesPerLine)*bytesPerLine : bytesPerLine
    );

    File file(filename, File::WRITE, File::CREATE);
    file.write(Cast::getCharAddress(header), sizeof(header));

    const GrayImage::ReadableRows::RowIterator endRow = image->getRows().getFirst();
    GrayImage::ReadableRows::RowIterator row = image->getRows().getEnd();
    const Allocator<char>::Iterator endDest = buffer.getEndIterator();
    Allocator<char>::Iterator dest = buffer.getBeginIterator();

    while (row != endRow) {
      --row;
      const GrayImage::ReadableRows::RowIterator::ElementIterator endColumn = row.getEnd();
      GrayImage::ReadableRows::RowIterator::ElementIterator column = row.getFirst();
      while (column != endColumn) {
        *dest++ = *column++;
      }
      if (dimension.getWidth() & 1) { // do we need to pad
        *dest++ = 0; // pad to 16 bit boundary
      }
      if (dest == endDest) {
        file.write(buffer.getElements(), buffer.getSize()); // write entire buffer
        dest = buffer.getBeginIterator();
      }
    }
    if (dest != buffer.getBeginIterator()) {
      file.write(buffer.getElements(), dest - buffer.getBeginIterator()); // write entire buffer
    }
    file.truncate(sizeof(header) + static_cast<unsigned long long>(bytesPerLine) * dimension.getHeight());
  }

  HashTable<String, AnyValue> RASEncoder::getInformation(const String& filename) throw(IOException) {
    HashTable<String, AnyValue> result;
    RASEncoderImpl::Header header;
    {
      File file(filename, File::READ, 0);
      file.read(Cast::getCharAddress(header), sizeof(header));
    }
    result[MESSAGE("encoder")] = Type::getType(*this);
    result[MESSAGE("description")] = MESSAGE("Sun Rasterfile Format");
    result[MESSAGE("width")] = static_cast<unsigned int>(header.width);
    result[MESSAGE("height")] = static_cast<unsigned int>(header.height);
    result[MESSAGE("depth")] = static_cast<unsigned int>(header.depth);
    return result;
  }

}; // end of gip namespace
