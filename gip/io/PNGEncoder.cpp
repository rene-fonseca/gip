/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/PNGEncoder.h>
#include <base/io/File.h>
#include <base/NotImplemented.h>

#include <png.h>

namespace gip {

  class PNGEncoderImpl {
  public:

    static void read(png_structp context, png_bytep buffer, png_size_t size) {
      File* file = (File*)::png_get_io_ptr(context);
      file->read(
        Cast::pointer<uint8*>(buffer),
        static_cast<unsigned int>(size)
      ); // could raise exception
    }

    static void write(png_structp context, png_bytep buffer, png_size_t size) {
      File* file = (File*)::png_get_io_ptr(context);
      file->write(
        Cast::pointer<const uint8*>(buffer),
        static_cast<unsigned int>(size)
      ); // could raise exception
    }

    static void flush(png_structp context) throw() {
      // file.flush(); // TAG: flushed when file is closed
    }

    static void errorHandler(
      png_structp context, png_const_charp message) throw(InvalidFormat) {
      throw InvalidFormat();
    }
    
  };
  
  PNGEncoder::PNGEncoder() throw() {
  }
  
  String PNGEncoder::getDescription() const throw() {
    return Literal("Portable Network Graphics");
  }
  
  String PNGEncoder::getDefaultExtension() const throw() {
    return Literal("png");
  }

  bool PNGEncoder::isValid(const String& filename) throw(IOException) {
    uint8 signature[8]; // size of signature is 8
    File file(filename, File::READ, 0);
    unsigned int result = file.read(signature, sizeof(signature));
    return (result == 8) && !::png_sig_cmp((png_byte*)signature, 0, result);
  }

  ColorImage* PNGEncoder::read(
    const String& filename) throw(InvalidFormat, IOException) {
    File file(filename, File::READ, 0);

    png_structp context = ::png_create_read_struct(
      PNG_LIBPNG_VER_STRING,
      0,
      PNGEncoderImpl::errorHandler,
      PNGEncoderImpl::errorHandler
    );
    bassert(context, MemoryException(this));
    
    png_infop information = ::png_create_info_struct(context);
    if (!information) {
      ::png_destroy_read_struct(&context, 0, 0);
      throw MemoryException(this);
    }

    png_infop endInformation = ::png_create_info_struct(context);
    if (!endInformation) {
      ::png_destroy_read_struct(&context, &information, 0);
      throw MemoryException(this);
    }

    // ::png_set_invert_alpha(context);

    try {
      ::png_set_read_fn(context, &file, PNGEncoderImpl::read);
      ::png_read_info(context, information);
      
      png_uint_32 width;
      png_uint_32 height;
      int bitDepth;
      int colorType;
      int interlaceType;
      int compressionType;
      int filterType;      
      ::png_get_IHDR(context, information, &width, &height, &bitDepth, &colorType, &interlaceType, &compressionType, &filterType);

      //if ((colorType == PNG_COLOR_TYPE_RGB) || (colorType == PNG_COLOR_TYPE_RGB_ALPHA)) {
      //  ::png_set_bgr(context);
      //}

      if ((colorType == PNG_COLOR_TYPE_GRAY) || (colorType == PNG_COLOR_TYPE_GRAY_ALPHA)) {
        ::png_set_gray_to_rgb(context);
      }

      // expand into 32 bits per pixel
      if ((bitDepth == 8) && (colorType == PNG_COLOR_TYPE_RGB)) {
        ::png_set_filler(context, 0xff /*ALPHA_OPAQUE_INTENSITY*/, PNG_FILLER_AFTER);
      }
      
      //if (colorType == PNG_COLOR_TYPE_RGB_ALPHA) {
      //  ::png_set_swap_alpha(context); // select ARGB
      //}

      int numberOfPasses = ::png_set_interlace_handling(context);
      ::png_read_update_info(context, information);
      ::png_get_IHDR(context, information, &width, &height, &bitDepth, &colorType, &interlaceType, &compressionType, &filterType);
      
      if (!((bitDepth == 8) && (colorType == PNG_COLOR_TYPE_RGB))) {
        ::png_destroy_read_struct(&context, &information, &endInformation);
        return 0;
      }
      
      ColorImage* image = new ColorImage(Dimension(width, height));

      unsigned int count = height;
      ColorPixel* rows[1024];
      ColorPixel* src = image->getElements() + width * height;
      while (count > 0) {
        unsigned int blockHeight = minimum(count, 1024U);
        count -= blockHeight;
        ColorPixel** row = rows;
        ColorPixel* end = src - width * blockHeight;
        while (src > end) {
          src -= width;
          *row++ = src;
        }
        ::png_read_rows(context, (png_bytepp)rows, 0, blockHeight); // TAG: fix for bad prototype in PNG API
//        ::png_read_image(context, Cast::pointer<png_bytepp>(rows));
      }
      
      ::png_read_end(context, 0);
      ::png_destroy_read_struct(&context, &information, &endInformation);
      return image;
    } catch(...) {
      ::png_destroy_read_struct(&context, &information, &endInformation);
      return 0;
    }
  }

  void PNGEncoder::write(
    const String& filename,
    const ColorImage* image) throw(ImageException, IOException) {
    unsigned int width = image->getDimension().getWidth();
    unsigned int height = image->getDimension().getHeight();
    bassert(
      (width <= (1 << 31)) && (height <= (1 << 31)),
      ImageException(this)
    );

    File file(filename, File::WRITE, File::CREATE | File::EXCLUSIVE);
    
    png_structp context = ::png_create_write_struct(
      PNG_LIBPNG_VER_STRING,
      0,
      PNGEncoderImpl::errorHandler,
      PNGEncoderImpl::errorHandler
    );
    bassert(context, MemoryException(this));
    
    png_infop information = ::png_create_info_struct(context);
    if (!information) {
      ::png_destroy_write_struct(&context, 0);
      throw MemoryException(this);
    }

    try {
      ::png_set_write_fn(context, &file, PNGEncoderImpl::write, PNGEncoderImpl::flush);
      
      ::png_set_IHDR(
        context,
        information,
        width,
        height,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_DEFAULT,
        PNG_FILTER_TYPE_DEFAULT
      );
      
      //::png_set_invert_alpha(context);
      ::png_set_compression_level(context, Z_BEST_COMPRESSION);
      
      ::png_write_info(context, information);
      
      ::png_set_filler(context, 0, PNG_FILLER_AFTER);
      //::png_set_bgr(context);
      
      unsigned int count = height;
      const ColorPixel* rows[1024];
      const ColorPixel* src = image->getElements() + width * height;
      while (count > 0) {
        unsigned int blockHeight = minimum(count, 1024U);
        count -= blockHeight;
        const ColorPixel** row = rows;
        const ColorPixel* end = src - width * blockHeight;
        while (src > end) {
          src -= width;
          *row++ = src;
        }
        ::png_write_rows(context, (png_bytepp)rows, blockHeight); // TAG: fix for bad prototype in PNG API
      }
      
      ::png_write_end(context, 0); // no comments or other information
      
      ::png_destroy_write_struct(&context, &information);
    } catch(...) {
      ::png_destroy_write_struct(&context, &information);
      throw;
    }
  }
  
  /*
    need support for encoder specific parameters
    quality, compression, and more
    
    void hint(enum {SLOWEST, DEFAULT, FASTEST}) throw();
    
    bool isLossless() const throw() - not static
  */
  
  void PNGEncoder::writeGray(
    const String& filename,
    const GrayImage* image) throw(ImageException, IOException) {
    throw NotImplemented(this);
  }

  HashTable<String, AnyValue> PNGEncoder::getInformation(
    const String& filename) throw(IOException) {
    HashTable<String, AnyValue> result;
    File file(filename, File::READ, 0);
    
    png_structp context = ::png_create_read_struct(
      PNG_LIBPNG_VER_STRING,
      0,
      PNGEncoderImpl::errorHandler,
      PNGEncoderImpl::errorHandler
    );
    bassert(context, MemoryException(this));
    
    png_infop information = ::png_create_info_struct(context);
    if (!information) {
      ::png_destroy_read_struct(&context, 0, 0);
      throw MemoryException(this);
    }
    
    try {
      ::png_set_read_fn(context, &file, PNGEncoderImpl::read);
      ::png_read_info(context, information);
      
      png_uint_32 width;
      png_uint_32 height;
      int bitDepth;
      int colorType;
      int interlaceType;
      int compressionType;
      int filterType;      
      ::png_get_IHDR(
        context,
        information,
        &width,
        &height,
        &bitDepth,
        &colorType,
        &interlaceType,
        &compressionType,
        &filterType
      );

      result["encoder"] = Type::getType(*this);
      result["description"] = "Portable Network Graphics";
      result["width"] = width;
      result["height"] = height;
      result["bit depth"] = bitDepth;
      result["color type"] = colorType;
      result["interlaced type"] = interlaceType;
      result["compression type"] = compressionType;
      result["filter type"] = filterType;
      
      ::png_destroy_read_struct(&context, &information, 0);
    } catch(IOException&) {
      ::png_destroy_read_struct(&context, &information, 0);
    }
    return result;
  }

}; // end of gip namespace
