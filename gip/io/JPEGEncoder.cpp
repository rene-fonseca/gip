/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/JPEGEncoder.h>
#include <base/io/File.h>
#include <base/mem/Allocator.h>
#include <base/NotImplemented.h>

#include <sys/types.h> // get size_t
#include <stdio.h> // get FILE (but we do not use this)

extern "C" {
#include <jpeglib.h>
};

namespace gip {

  class JPEGEncoderImpl {
  public:

    struct JPEGDestination {
      struct jpeg_destination_mgr pub;
      File* file;
      char* buffer;
    };
    
    static void initializeDestination(j_compress_ptr cinfo) {
      //JPEGDestination* dest = Cast::pointer<JPEGDestination*>(cinfo->dest);
    }

    static boolean empty(j_compress_ptr cinfo) {
      JPEGDestination* dest = Cast::pointer<JPEGDestination*>(cinfo->dest);
      dest->file->write(dest->buffer, JPEGEncoder::BUFFER_SIZE);
      dest->pub.next_output_byte = Cast::pointer<JOCTET*>(dest->buffer);
      dest->pub.free_in_buffer = JPEGEncoder::BUFFER_SIZE;
      return TRUE;
    }
    
    static void flush(j_compress_ptr cinfo) {
      JPEGDestination* dest = Cast::pointer<JPEGDestination*>(cinfo->dest);
      dest->file->write(dest->buffer, JPEGEncoder::BUFFER_SIZE - dest->pub.free_in_buffer);
    }
    
    struct JPEGSource {
      struct jpeg_source_mgr pub;
      File* file;
      char* buffer;
      //bool startOfFile;
    };
    
    static void initialize(j_decompress_ptr cinfo) {
      //JPEGSource* source = (JPEGSource*)cinfo->src;
      //source->startOfFile = true;
    }

    static boolean fill(j_decompress_ptr cinfo) {
      JPEGSource* source = (JPEGSource*)cinfo->src;
      unsigned int bytesRead = source->file->read(source->buffer, JPEGEncoder::BUFFER_SIZE, true); // non-blocking
      if (bytesRead == 0) {
        bytesRead = source->file->read(source->buffer, JPEGEncoder::BUFFER_SIZE, 1); // blocking
      }
      source->pub.next_input_byte = Cast::pointer<JOCTET*>(source->buffer);
      source->pub.bytes_in_buffer = bytesRead;
      //source->startOfFile = false;
      return TRUE;
    }

    static void skip(j_decompress_ptr cinfo, long numberOfBytes) {
      JPEGSource* source = (JPEGSource*)cinfo->src;
      if (numberOfBytes > 0) {
        while (numberOfBytes > static_cast<long>(source->pub.bytes_in_buffer)) {
          numberOfBytes -= static_cast<long>(source->pub.bytes_in_buffer);
          fill(cinfo);
        }
        source->pub.next_input_byte += (size_t)numberOfBytes;
        source->pub.bytes_in_buffer -= (size_t)numberOfBytes;
      }
    }
    
    static void close(j_decompress_ptr cinfo) {
      // does nothing
    }
    
    static void errorHandler(j_common_ptr cinfo) throw(InvalidFormat) {
      throw InvalidFormat();
    }
    
  };

  JPEGEncoder::JPEGEncoder() throw() {
  }

  String JPEGEncoder::getDescription() const throw() {
    return MESSAGE("Joint Photographic Experts Group");
  }
  
  String JPEGEncoder::getDefaultExtension() const throw() {
    return MESSAGE("jpg");
  }

  Array<String> JPEGEncoder::getExtensions() const throw() {
    Array<String> extensions;
    extensions.append(MESSAGE("jpg"));
    extensions.append(MESSAGE("jpeg"));
    return extensions;
  }

  bool JPEGEncoder::isValid(const String& filename) throw(IOException) {
    JPEGEncoderImpl::JPEGSource source;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    Allocator<char> buffer(BUFFER_SIZE);
    File file(filename, File::READ, 0);
    
    source.pub.init_source = JPEGEncoderImpl::initialize;
    source.pub.fill_input_buffer = JPEGEncoderImpl::fill;
    source.pub.skip_input_data = JPEGEncoderImpl::skip;
    source.pub.resync_to_restart = ::jpeg_resync_to_restart;
    source.pub.term_source = JPEGEncoderImpl::close;
    source.pub.bytes_in_buffer = 0;
    source.pub.next_input_byte = 0;
    source.file = &file;
    source.buffer = buffer.getElements();
    
    cinfo.err = ::jpeg_std_error(&jerr);
    jerr.error_exit = JPEGEncoderImpl::errorHandler;
    
    try {
      ::jpeg_create_decompress(&cinfo);
      cinfo.src = Cast::pointer<struct jpeg_source_mgr*>(&source);
      try {
        ::jpeg_read_header(&cinfo, TRUE);
      } catch(...) {
        return false;
      }
      ::jpeg_destroy_decompress(&cinfo);
    } catch(...) {
      return false; // but we really do not know
    }
    return true;
  }

  ColorImage* JPEGEncoder::read(const String& filename) throw(InvalidFormat, IOException) {
    JPEGEncoderImpl::JPEGSource source;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    Allocator<char> buffer(BUFFER_SIZE);
    File file(filename, File::READ, 0);
    
    source.pub.init_source = JPEGEncoderImpl::initialize;
    source.pub.fill_input_buffer = JPEGEncoderImpl::fill;
    source.pub.skip_input_data = JPEGEncoderImpl::skip;
    source.pub.resync_to_restart = ::jpeg_resync_to_restart;
    source.pub.term_source = JPEGEncoderImpl::close;
    source.pub.bytes_in_buffer = 0;
    source.pub.next_input_byte = 0;
    source.file = &file;
    source.buffer = buffer.getElements();
    
    cinfo.err = ::jpeg_std_error(&jerr);
    jerr.error_exit = JPEGEncoderImpl::errorHandler;
    
    ColorImage* image = 0;
    try {
      ::jpeg_create_decompress(&cinfo);
      cinfo.src = Cast::pointer<struct jpeg_source_mgr*>(&source);
      try {
        ::jpeg_read_header(&cinfo, TRUE);
        
        // we want RGB format (possible gray)
        cinfo.out_color_space =  JCS_RGB;
        
        image = new ColorImage(Dimension(cinfo.image_width, cinfo.image_height));
        ColorPixel* dest = image->getElements();
        
        ::jpeg_start_decompress(&cinfo);
        
        JSAMPLE row[cinfo.output_width * cinfo.output_components];
        JSAMPROW prow = Cast::pointer<JSAMPROW>(row);
        
        ASSERT(sizeof(JSAMPLE) == 1);
        
        while (cinfo.output_scanline < cinfo.output_height) {
          ::jpeg_read_scanlines(&cinfo, &prow, 1);
          const JSAMPLE* src = row;
          const ColorPixel* end = dest + cinfo.output_width;
          while (dest < end) {
            ColorPixel element;
            element.red = *src++;
            element.green = *src++;
            element.blue = *src++;
            *dest++ = element;
          }
        }
        
        ::jpeg_finish_decompress(&cinfo);
      } catch(...) {
        image = 0;
      }
      ::jpeg_destroy_decompress(&cinfo);
    } catch(...) {
    }
    
    return image;
  }
  
  void JPEGEncoder::write(const String& filename, const ColorImage* image) throw(ImageException, IOException) {
    struct JPEGEncoderImpl::JPEGDestination dest;
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    //assert(image->getDimension().getWidth() < 0xffff, ImageException(this));
    //assert(image->getDimension().getHeight() < 0xffff, ImageException(this));

    Allocator<char> buffer(BUFFER_SIZE);
    File file(filename, File::WRITE, File::CREATE | File::EXCLUSIVE);

    dest.pub.init_destination = JPEGEncoderImpl::initializeDestination;
    dest.pub.empty_output_buffer = JPEGEncoderImpl::empty;
    dest.pub.term_destination = JPEGEncoderImpl::flush;
    dest.pub.next_output_byte = Cast::pointer<JOCTET*>(buffer.getElements());
    dest.pub.free_in_buffer = BUFFER_SIZE;
    dest.file = &file;
    dest.buffer = buffer.getElements();
    
    cinfo.err = ::jpeg_std_error(&jerr);

    try {
      ::jpeg_create_compress(&cinfo);
      cinfo.dest = Cast::pointer<struct jpeg_destination_mgr*>(&dest);
      
      cinfo.image_width = image->getDimension().getWidth();
      cinfo.image_height = image->getDimension().getHeight();
      cinfo.input_components = 3;
      cinfo.in_color_space = JCS_RGB;
      
      ::jpeg_set_defaults(&cinfo);
    
      //::jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
      try {
        ::jpeg_start_compress(&cinfo, TRUE);
    
        ASSERT(sizeof(JSAMPLE) == 1);
        JSAMPLE row[cinfo.image_width * cinfo.num_components];
        JSAMPROW prow = Cast::pointer<JSAMPROW>(row);
        const ColorPixel* src = image->getElements();
        
        while (cinfo.next_scanline < cinfo.image_height) {
          // TAG: check top-bottom problem
          JSAMPLE* dest = row;
          const ColorPixel* end = src + cinfo.image_width;
          while (src < end) {
            *dest++ = src->red;
            *dest++ = src->green;
            *dest++ = src->blue;
            ++src;
          }
          ::jpeg_write_scanlines(&cinfo, &prow, 1);
        }
        ::jpeg_finish_compress(&cinfo);
      } catch(...) {
      }
      ::jpeg_destroy_compress(&cinfo);
    } catch(...) {
    }
  }

  void JPEGEncoder::writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException) {
    throw NotImplemented();
  }
/*
  class Information : public Object {
  private:
    Dimension dimension;
    enum Type {GRAY, RGB, RGBA};
    enum Component {INTENSITY = 1, RED = 2, GREEN = 4, BLUE = 8, ALPHA = 16};
    unsigned int bitsPerComponent;
  public:

    Dimension getDimension() const throw() {return dimension;}
    
    Type getType() const throw() {return type;}

    unsigned int getPlanes() const throw() {return numberOfComponents;}
    
    unsigned int getComponents() const throw() {return components;}

    bool isCompatibleWith(Type type) const throw() {return false;}

    unsigned int bitsPerComponents() const throw() {return bitsPerComponent;}

    unsigned int bitsPerSample() const throw() {return 0;}
    
    unsigned int getNumberOfImages() const throw() {return numberOfImages;}
  };
  
  Information getGenralInformation() throw() {
  }
  */

  HashTable<String, AnyValue> JPEGEncoder::getInformation(const String& filename) throw(IOException) {
    HashTable<String, AnyValue> result;
    
    JPEGEncoderImpl::JPEGSource source;
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    Allocator<char> buffer(BUFFER_SIZE);
    File file(filename, File::READ, 0);
    
    source.pub.init_source = JPEGEncoderImpl::initialize;
    source.pub.fill_input_buffer = JPEGEncoderImpl::fill;
    source.pub.skip_input_data = JPEGEncoderImpl::skip;
    source.pub.resync_to_restart = ::jpeg_resync_to_restart;
    source.pub.term_source = JPEGEncoderImpl::close;
    source.pub.bytes_in_buffer = 0;
    source.pub.next_input_byte = 0;
    source.file = &file;
    source.buffer = buffer.getElements();
    
    cinfo.err = ::jpeg_std_error(&jerr);
    jerr.error_exit = JPEGEncoderImpl::errorHandler;

    try {
      ::jpeg_create_decompress(&cinfo);
      cinfo.src = Cast::pointer<struct jpeg_source_mgr*>(&source);
      try {
        ::jpeg_read_header(&cinfo, TRUE);
      } catch (...) {
        throw InvalidFormat(this);
      }
      ::jpeg_destroy_decompress(&cinfo);
    } catch (...) {
      throw InvalidFormat(this);
    }
    
    result[MESSAGE("encoder")] = Type::getType(*this);
    result[MESSAGE("description")] = MESSAGE("Joint Photographic Experts Group File Format");
    result[MESSAGE("width")] = cinfo.image_width;
    result[MESSAGE("height")] = cinfo.image_height;
    result[MESSAGE("components")] = cinfo.num_components;
    return result;
  }

}; // end of gip namespace
