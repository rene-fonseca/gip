/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/TGAEncoder.h>
#include <base/mem/Allocator.h>
#include <base/io/File.h>
#include <base/io/FileReader.h>
#include <base/concurrency/Thread.h>
#include <base/Primitives.h>
#include <base/ByteOrder.h>
#include <base/string/String.h>
#include <base/NotImplemented.h>

using namespace base;

namespace gip {

  namespace TGAEncoderImpl {

    static const StringLiteral signature = MESSAGE("TRUEVISION-XFILE");
    
    enum {
      TYPE_NO_IMAGE_DATA = 0,
      TYPE_UNCOMPRESSED_COLOR_MAPPED = 1,
      TYPE_UNCOMPRESSED_TRUE_COLOR = 2,
      TYPE_UNCOMPRESSED_BLACK_WHITE = 3,
      TYPE_RUN_LENGTH_COLOR_MAPPED = 9,
      TYPE_RUN_LENGTH_TRUE_COLOR = 10,
      TYPE_RUN_LENGTH_BLACK_WHITE = 11
    };
    
    typedef struct {
      unsigned char sizeOfIdentification;
      unsigned char colorMapType; // 0 ~ no color map - 1 ~ color map present
      unsigned char type;
      struct ColorMap {
        LittleEndian::UnsignedShort origin;
        LittleEndian::UnsignedShort length;
        unsigned char bitsPerEntry; // 16, 24, or 32
      } __attribute__ ((packed)) colorMap; // ignored for type 0
      struct Image {
        LittleEndian::UnsignedShort x;
        LittleEndian::UnsignedShort y;
        LittleEndian::UnsignedShort width;
        LittleEndian::UnsignedShort height;
        unsigned char pixelDepth; // e.g. 8, 16, 24, or 32
        unsigned char attributeBits : 4;
        unsigned char origin : 2;
        unsigned char reserved : 2; // must be 0
      } __attribute__ ((packed)) image;
    } __attribute__ ((packed)) Header;
    
    typedef struct {
      LittleEndian::UnsignedInt extensionOffset;
      LittleEndian::UnsignedInt directoryOffset;
      char signature[16];
      char dot; // '.'
      char zero; // '\0'
    } __attribute__ ((packed)) Footer;
  };
  
  TGAEncoder::TGAEncoder() throw() {
  }

  String TGAEncoder::getDescription() const throw() {
    return MESSAGE("Truevision Targa");
  }

  String TGAEncoder::getDefaultExtension() const throw() {
    return MESSAGE("tga");
  }

  Array<String> TGAEncoder::getExtensions() const throw() {
    Array<String> extensions;
    extensions.append(MESSAGE("tga"));
    extensions.append(MESSAGE("tpic"));
    return extensions;
  }
  
  bool TGAEncoder::isValid(const String& filename) throw(IOException) {    
    bool newFormat = false;
    TGAEncoderImpl::Header header;
    TGAEncoderImpl::Footer footer;    
    
    File file(filename, File::READ, 0);
    if (file.getSize() >= sizeof(footer)) {
      file.setPosition(-sizeof(footer), File::END);
      file.read(getCharAddress(footer), sizeof(footer));
      if ((compare<char>(footer.signature, TGAEncoderImpl::signature, TGAEncoderImpl::signature.getLength()) == 0) &&
          (footer.dot == '.') &&
          (footer.zero == '\0')) {
        newFormat = true;
      }
      file.setPosition(0);
    }
    file.read(getCharAddress(header), sizeof(header));
    
    switch (header.type) {
    case TGAEncoderImpl::TYPE_UNCOMPRESSED_COLOR_MAPPED:
    case TGAEncoderImpl::TYPE_RUN_LENGTH_COLOR_MAPPED:
      return header.colorMapType == 1;
    case TGAEncoderImpl::TYPE_UNCOMPRESSED_TRUE_COLOR:
    case TGAEncoderImpl::TYPE_UNCOMPRESSED_BLACK_WHITE:
    case TGAEncoderImpl::TYPE_RUN_LENGTH_TRUE_COLOR:
    case TGAEncoderImpl::TYPE_RUN_LENGTH_BLACK_WHITE:
      return header.colorMapType == 0;
    case TGAEncoderImpl::TYPE_NO_IMAGE_DATA:
    default:
      return false;
    }
  }
  
  ColorImage* TGAEncoder::read(const String& filename) throw(InvalidFormat, IOException) {    
    bool newFormat = false;
    TGAEncoderImpl::Header header;
    TGAEncoderImpl::Footer footer;    
    
    File file(filename, File::READ, 0);
    if (file.getSize() >= sizeof(footer)) {
      file.setPosition(-sizeof(footer), File::END);
      file.read(getCharAddress(footer), sizeof(footer));
      if ((compare<char>(footer.signature, TGAEncoderImpl::signature, TGAEncoderImpl::signature.getLength()) == 0) &&
          (footer.dot == '.') &&
          (footer.zero == '\0')) {
        newFormat = true;
      }
      file.setPosition(0);
    }
    file.read(getCharAddress(header), sizeof(header));

    if (header.type != TGAEncoderImpl::TYPE_UNCOMPRESSED_TRUE_COLOR) {
      return 0;
    }
    
    assert((header.colorMapType == 0) && (header.image.pixelDepth == 24), InvalidFormat(this));
    
    const Dimension dimension(header.image.width, header.image.height);
    ColorImage* image = new ColorImage(dimension);
    
    FileReader reader(file, sizeof(header) + header.sizeOfIdentification);
    // TAG: skip fields...
    
    ColorPixel* dest = image->getElements();

    // TAG: look at origin field in header
    assert(header.image.origin == 0, NotImplemented(this));
    
    for (unsigned int row = dimension.getHeight(); row > 0; --row) {
      FileReader::ReadIterator src = reader.peek(dimension.getWidth() * 3);
      for (unsigned int column = dimension.getWidth(); column > 0; --column) {
        Intensity blue = *src++;
        Intensity green = *src++;
        Intensity red = *src++;
        *dest++ = makeRGBPixel(red, green, blue); // order of args is red, green, and blue
      }
      reader.skip(dimension.getWidth() * 3);
    }
    return image;
  }
  
  void TGAEncoder::write(const String& filename, const ColorImage* image) throw(ImageException, IOException) {
    assert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    assert((dimension.getWidth() <= 0xffff) && (dimension.getHeight() <= 0xffff), ImageException(this));

    TGAEncoderImpl::Header header;
    clear(header);
    header.type = TGAEncoderImpl::TYPE_UNCOMPRESSED_TRUE_COLOR; // TGAEncoderImpl::TYPE_RUN_LENGTH_TRUE_COLOR;
    header.image.width = dimension.getWidth();
    header.image.height = dimension.getHeight();
    header.image.pixelDepth = 24;
    header.image.origin = 0; // bottom and left
    
    TGAEncoderImpl::Footer footer;
    footer.extensionOffset = 0;
    footer.directoryOffset = 0;
    copy<char>(footer.signature, TGAEncoderImpl::signature, TGAEncoderImpl::signature.getLength());
    footer.dot = '.';
    footer.zero = '\0';
    
    Allocator<char> buffer(BUFFER_SIZE);
    File file(filename, File::WRITE, File::CREATE | File::EXCLUSIVE);
    file.write(getCharAddress(header), sizeof(header));
    
    unsigned int count = dimension.getSize(); // max is 0xffff * 0xffff
    const ColorPixel* src = image->getElements();
    while (count > 0) {
      unsigned int elementsToCopy = minimum(BUFFER_SIZE/3, count);
      char* dest = buffer.getElements();
      const ColorPixel* end = src + elementsToCopy;
      while (src < end) {
        *dest++ = src->blue;
        *dest++ = src->green;
        *dest++ = src->red;
        ++src;
      }
      file.write(buffer.getElements(), elementsToCopy * 3);
      count -= elementsToCopy;
    }
    
    file.write(getCharAddress(footer), sizeof(footer));
    file.truncate(sizeof(header) + static_cast<unsigned long long>(dimension.getSize()) * 3 + sizeof(footer));
  }

  void TGAEncoder::write(const String& filename, const ArrayImage<RGBAPixel>* image) throw(ImageException, IOException) {
    assert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    assert((dimension.getWidth() <= 0xffff) && (dimension.getHeight() <= 0xffff), ImageException(this));
    
    TGAEncoderImpl::Header header;
    clear(header);
    header.type = TGAEncoderImpl::TYPE_UNCOMPRESSED_TRUE_COLOR; // TGAEncoderImpl::TYPE_RUN_LENGTH_TRUE_COLOR;
    header.image.width = dimension.getWidth();
    header.image.height = dimension.getHeight();
    header.image.pixelDepth = 32;
    header.image.attributeBits = 8;
    header.image.origin = 2; // bottom and left
    
    TGAEncoderImpl::Footer footer;
    footer.extensionOffset = 0;
    footer.directoryOffset = 0;
    copy<char>(footer.signature, TGAEncoderImpl::signature, TGAEncoderImpl::signature.getLength());
    footer.dot = '.';
    footer.zero = '\0';
    
    Allocator<char> buffer(BUFFER_SIZE);
    File file(filename, File::WRITE, File::CREATE | File::EXCLUSIVE);
    file.write(getCharAddress(header), sizeof(header));
    
    unsigned int count = dimension.getSize(); // max is 0xffff * 0xffff
    const RGBAPixel* src = image->getElements();
    while (count > 0) {
      unsigned int elementsToCopy = minimum(BUFFER_SIZE/4, count);
      char* dest = buffer.getElements();
      const RGBAPixel* end = src + elementsToCopy;
      while (src < end) {
        *dest++ = src->blue;
        *dest++ = src->green;
        *dest++ = src->red;
        *dest++ = src->alpha;
        ++src;
      }
      file.write(buffer.getElements(), elementsToCopy * 4);
      count -= elementsToCopy;
    }

    file.write(getCharAddress(footer), sizeof(footer));
    file.truncate(sizeof(header) + static_cast<unsigned long long>(dimension.getSize()) * 4 + sizeof(footer));
  }

  void TGAEncoder::writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException) {
    assert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    assert((dimension.getWidth() <= 0xffff) && (dimension.getHeight() <= 0xffff), ImageException(this));

    TGAEncoderImpl::Header header;
    clear(header);
    header.type = TGAEncoderImpl::TYPE_UNCOMPRESSED_BLACK_WHITE; // TGAEncoderImpl::TYPE_RUN_LENGTH_BLACK_WHITE;
    header.image.width = dimension.getWidth();
    header.image.height = dimension.getHeight();
    header.image.pixelDepth = 8;
    header.image.origin = 0; // bottom and left
    
    TGAEncoderImpl::Footer footer;
    footer.extensionOffset = 0;
    footer.directoryOffset = 0;
    copy<char>(footer.signature, TGAEncoderImpl::signature, TGAEncoderImpl::signature.getLength());
    footer.dot = '.';
    footer.zero = '\0';
    
    File file(filename, File::WRITE, File::CREATE | File::EXCLUSIVE);
    
    unsigned int count = dimension.getSize(); // max is 0xffff * 0xffff
    const GrayPixel* src = image->getElements();
    if (sizeof(GrayPixel) == 1) {
      file.write(getCharAddress(header), sizeof(header));
      file.write(pointer_cast<const char*>(src), count);
    } else {
      Allocator<char> buffer(BUFFER_SIZE);
      file.write(getCharAddress(header), sizeof(header));
      while (count > 0) {
        unsigned int bytesToCopy = minimum(BUFFER_SIZE, count);
        char* dest = buffer.getElements();
        const GrayPixel* end = src + bytesToCopy;
        while (src < end) {
          *dest++ = *src++;
        }
        file.write(buffer.getElements(), bytesToCopy);
        count -= bytesToCopy;
      }
    }
    
    file.write(getCharAddress(footer), sizeof(footer));
    file.truncate(sizeof(header) + dimension.getSize() * 1 + sizeof(footer));
  }

  FormatOutputStream& TGAEncoder::getInfo(FormatOutputStream& stream, const String& filename) throw(IOException) {
    static const StringLiteral signature = MESSAGE("TRUEVISION-XFILE");
    
    bool newFormat = false;
    TGAEncoderImpl::Header header;
    TGAEncoderImpl::Footer footer;
    
    fout << "sizeof(header)=" << sizeof(header) << ENDL;
    fout << "sizeof(footer)=" << sizeof(footer) << ENDL;
    
    File file(filename, File::READ, 0);
    if (file.getSize() >= sizeof(footer)) {
      file.setPosition(-sizeof(footer), File::END);
      fout << "size=" << file.getSize() << ENDL;
      fout << "position=" << file.getPosition() << ENDL;
      file.read(getCharAddress(footer), sizeof(footer) - 1);
      if ((compare<char>(footer.signature, signature, signature.getLength()) == 0) &&
          (footer.dot == '.') &&
          (footer.zero == '\0')) {
        newFormat = true;
      }
      file.setPosition(0, File::BEGIN);
      fout << "position=" << file.getPosition() << ENDL;
    }
    file.read(getCharAddress(header), sizeof(header));

    switch (header.type) {
    case TGAEncoderImpl::TYPE_NO_IMAGE_DATA:
    case TGAEncoderImpl::TYPE_UNCOMPRESSED_COLOR_MAPPED:
    case TGAEncoderImpl::TYPE_UNCOMPRESSED_TRUE_COLOR:
    case TGAEncoderImpl::TYPE_UNCOMPRESSED_BLACK_WHITE:
    case TGAEncoderImpl::TYPE_RUN_LENGTH_COLOR_MAPPED:
    case TGAEncoderImpl::TYPE_RUN_LENGTH_TRUE_COLOR:
    case TGAEncoderImpl::TYPE_RUN_LENGTH_BLACK_WHITE:
      break;
    default:
      return stream;
    }

    stream << MESSAGE("TGAEncoder (Truevision Targa):") << EOL
           << MESSAGE("  new format=") << newFormat << EOL
           << MESSAGE("  color map type=") << header.colorMapType << EOL
           << MESSAGE("  type=") << header.type << EOL
           << MESSAGE("  x=") << header.image.x << EOL
           << MESSAGE("  y=") << header.image.y << EOL
           << MESSAGE("  width=") << header.image.width << EOL
           << MESSAGE("  height=") << header.image.height << EOL
           << MESSAGE("  pixel depth=") << header.image.pixelDepth << EOL;
    return stream;
  }

}; // end of namespace