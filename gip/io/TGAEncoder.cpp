/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

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

namespace gip {

  namespace TGAEncoderImpl {

    static const Literal signature = "TRUEVISION-XFILE";
    
    enum {
      TYPE_NO_IMAGE_DATA = 0,
      TYPE_UNCOMPRESSED_COLOR_MAPPED = 1,
      TYPE_UNCOMPRESSED_TRUE_COLOR = 2,
      TYPE_UNCOMPRESSED_BLACK_WHITE = 3,
      TYPE_RUN_LENGTH_COLOR_MAPPED = 9,
      TYPE_RUN_LENGTH_TRUE_COLOR = 10,
      TYPE_RUN_LENGTH_BLACK_WHITE = 11
    };
    
_COM_AZURE_DEV__BASE__PACKED__BEGIN
    struct Header {
      unsigned char sizeOfIdentification;
      unsigned char colorMapType; // 0 ~ no color map - 1 ~ color map present
      unsigned char type;
      struct ColorMap {
        LittleEndian<uint16> origin;
        LittleEndian<uint16> length;
        uint8 bitsPerEntry; // 16, 24, or 32
      } _COM_AZURE_DEV__BASE__PACKED colorMap; // ignored for type 0
      struct Image {
        LittleEndian<uint16> x;
        LittleEndian<uint16> y;
        LittleEndian<uint16> width;
        LittleEndian<uint16> height;
        uint8 pixelDepth; // e.g. 8, 16, 24, or 32
        uint8 attributeBits : 4;
        uint8 origin : 2;
        uint8 reserved : 2; // must be 0
      } _COM_AZURE_DEV__BASE__PACKED image;
    } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END

_COM_AZURE_DEV__BASE__PACKED__BEGIN
    struct Footer {
      LittleEndian<uint32> extensionOffset;
      LittleEndian<uint32> directoryOffset;
      char signature[16];
      char dot; // '.'
      char zero; // '\0'
    } _COM_AZURE_DEV__BASE__PACKED;
_COM_AZURE_DEV__BASE__PACKED__END
  };
  
  TGAEncoder::TGAEncoder() throw() {
  }

  String TGAEncoder::getDescription() const throw() {
    return Literal("Truevision Targa");
  }

  String TGAEncoder::getDefaultExtension() const throw() {
    return Literal("tga");
  }

  Array<String> TGAEncoder::getExtensions() const throw() {
    Array<String> extensions;
    extensions.append("tga");
    extensions.append("tpic");
    return extensions;
  }
  
  bool TGAEncoder::isValid(const String& filename) throw(IOException) {    
    bool newFormat = false;
    TGAEncoderImpl::Header header;
    TGAEncoderImpl::Footer footer;    
    
    File file(filename, File::READ, 0);
    if (file.getSize() >= sizeof(footer)) {
      file.setPosition(-static_cast<MemoryDiff>(sizeof(footer)), File::END);
      file.read(Cast::getAddress(footer), sizeof(footer));
      if ((compare<char>(
             footer.signature,
             TGAEncoderImpl::signature.getValue(),
             TGAEncoderImpl::signature.getLength()) == 0) &&
          (footer.dot == '.') &&
          (footer.zero == '\0')) {
        newFormat = true;
      }
      file.setPosition(0);
    }
    file.read(Cast::getAddress(header), sizeof(header));
    
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
  
  ColorImage* TGAEncoder::read(
    const String& filename) throw(InvalidFormat, IOException) {    
    bool newFormat = false;
    TGAEncoderImpl::Header header;
    TGAEncoderImpl::Footer footer;    
    
    File file(filename, File::READ, 0);
    if (file.getSize() >= sizeof(footer)) {
      file.setPosition(-static_cast<MemoryDiff>(sizeof(footer)), File::END);
      file.read(Cast::getAddress(footer), sizeof(footer));
      if ((compare<char>(
             footer.signature,
             TGAEncoderImpl::signature.getValue(),
             TGAEncoderImpl::signature.getLength()
           ) == 0) &&
          (footer.dot == '.') &&
          (footer.zero == '\0')) {
        newFormat = true;
      }
      file.setPosition(0);
    }
    file.read(Cast::getAddress(header), sizeof(header));

    if (header.type != TGAEncoderImpl::TYPE_UNCOMPRESSED_TRUE_COLOR) {
      return 0;
    }
    
    bassert(
      (header.colorMapType == 0) && (header.image.pixelDepth == 24),
      InvalidFormat(this)
    );
    
    const Dimension dimension(header.image.width, header.image.height);
    ColorImage* image = new ColorImage(dimension);
    
    FileReader reader(file, sizeof(header) + header.sizeOfIdentification);
    // TAG: skip fields...
    
    ColorPixel* dest = image->getElements();

    // TAG: look at origin field in header
    bassert(header.image.origin == 0, NotImplemented(this));
    
    for (unsigned int row = dimension.getHeight(); row > 0; --row) {
      FileReader::ReadIterator src = reader.peek(dimension.getWidth() * 3);
      for (unsigned int column = dimension.getWidth(); column > 0; --column) {
        uint8 blue = *src++;
        uint8 green = *src++;
        uint8 red = *src++;
        *dest++ = makeColorPixel(red, green, blue); // order of args is red, green, and blue
      }
      reader.skip(dimension.getWidth() * 3);
    }
    return image;
  }
  
  void TGAEncoder::write(
    const String& filename,
    const ColorImage* image) throw(ImageException, IOException) {
    bassert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    bassert(
      (dimension.getWidth() <= 0xffff) && (dimension.getHeight() <= 0xffff),
      ImageException(this)
    );

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
    copy<char>(
      footer.signature,
      TGAEncoderImpl::signature.getValue(),
      TGAEncoderImpl::signature.getLength()
    );
    footer.dot = '.';
    footer.zero = '\0';
    
    Allocator<uint8> buffer(BUFFER_SIZE);
    File file(filename, File::WRITE, File::CREATE | File::EXCLUSIVE);
    file.write(Cast::getAddress(header), sizeof(header));
    
    unsigned int count = dimension.getSize(); // max is 0xffff * 0xffff
    const ColorPixel* src = image->getElements();
    while (count > 0) {
      unsigned int elementsToCopy = minimum(BUFFER_SIZE/3, count);
      uint8* dest = buffer.getElements();
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
    
    file.write(Cast::getAddress(footer), sizeof(footer));
    file.truncate(
      sizeof(header) +
      static_cast<unsigned long long>(dimension.getSize()) * 3 + sizeof(footer)
    );
  }

  void TGAEncoder::write(
    const String& filename,
    const ColorAlphaImage* image) throw(ImageException, IOException) {
    bassert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    bassert(
      (dimension.getWidth() <= 0xffff) && (dimension.getHeight() <= 0xffff),
      ImageException(this)
    );
    
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
    copy<char>(
      footer.signature,
      TGAEncoderImpl::signature.getValue(),
      TGAEncoderImpl::signature.getLength()
    );
    footer.dot = '.';
    footer.zero = '\0';
    
    Allocator<uint8> buffer(BUFFER_SIZE);
    File file(filename, File::WRITE, File::CREATE | File::EXCLUSIVE);
    file.write(Cast::getAddress(header), sizeof(header));
    
    unsigned int count = dimension.getSize(); // max is 0xffff * 0xffff
    const ColorAlphaPixel* src = image->getElements();
    while (count > 0) {
      unsigned int elementsToCopy = minimum(BUFFER_SIZE/4, count);
      uint8* dest = buffer.getElements();
      const ColorAlphaPixel* end = src + elementsToCopy;
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

    file.write(Cast::getAddress(footer), sizeof(footer));
    file.truncate(
      sizeof(header) +
      static_cast<unsigned long long>(dimension.getSize()) * 4 + sizeof(footer)
    );
  }

  void TGAEncoder::writeGray(
    const String& filename,
    const GrayImage* image) throw(ImageException, IOException) {
    bassert(image, NullPointer(this));
    Dimension dimension = image->getDimension();
    bassert(
      (dimension.getWidth() <= 0xffff) && (dimension.getHeight() <= 0xffff),
      ImageException(this)
    );

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
    copy<char>(
      footer.signature,
      TGAEncoderImpl::signature.getValue(),
      TGAEncoderImpl::signature.getLength()
    );
    footer.dot = '.';
    footer.zero = '\0';
    
    File file(filename, File::WRITE, File::CREATE | File::EXCLUSIVE);
    
    unsigned int count = dimension.getSize(); // max is 0xffff * 0xffff
    const GrayPixel* src = image->getElements();
    if (sizeof(GrayPixel) == 1) {
      file.write(Cast::getAddress(header), sizeof(header));
      file.write(Cast::pointer<const uint8*>(src), count);
    } else {
      Allocator<uint8> buffer(BUFFER_SIZE);
      file.write(Cast::getAddress(header), sizeof(header));
      while (count > 0) {
        unsigned int bytesToCopy = minimum(BUFFER_SIZE, count);
        uint8* dest = buffer.getElements();
        const GrayPixel* end = src + bytesToCopy;
        while (src < end) {
          *dest++ = *src++;
        }
        file.write(buffer.getElements(), bytesToCopy);
        count -= bytesToCopy;
      }
    }
    
    file.write(Cast::getAddress(footer), sizeof(footer));
    file.truncate(sizeof(header) + dimension.getSize() * 1 + sizeof(footer));
  }

  HashTable<String, AnyValue> TGAEncoder::getInformation(
    const String& filename) throw(IOException) {
    HashTable<String, AnyValue> result;
    static const Literal signature("TRUEVISION-XFILE");
    
    bool newFormat = false;
    TGAEncoderImpl::Header header;
    TGAEncoderImpl::Footer footer;
    
    File file(filename, File::READ, 0);
    if (file.getSize() >= sizeof(footer)) {
      file.setPosition(-static_cast<MemoryDiff>(sizeof(footer)), File::END);
      file.read(Cast::getAddress(footer), sizeof(footer) - 1);
      if ((compare<char>(
             footer.signature,
             signature.getValue(),
             signature.getLength()
           ) == 0) &&
          (footer.dot == '.') &&
          (footer.zero == '\0')) {
        newFormat = true;
      }
      file.setPosition(0, File::BEGIN);
    }
    file.read(Cast::getAddress(header), sizeof(header));
    
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
      throw InvalidFormat(this);
    }
    
    result["encoder"] = Type::getType(*this);
    result["description"] = "Truevision Targa";
    result["x"] = static_cast<unsigned int>(header.image.x);
    result["y"] = static_cast<unsigned int>(header.image.y);
    result["width"] = static_cast<unsigned int>(header.image.width);
    result["height"] = static_cast<unsigned int>(header.image.height);
    result["depth"] = static_cast<unsigned int>(header.image.pixelDepth);
    return result;
  }

}; // end of gip namespace
