/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__JPEG_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__JPEG_ENCODER_H

#include <gip/io/ImageEncoder.h>

namespace gip {

  /**
    Joint Photographic Experts Group (JPEG) format encoder/decoder.
    
    @short Joint Photographic Experts Group (JPEG) encoder/decoder
    @ingroup imageEncoders
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.1
  */

  class JPEGEncoder : public ImageEncoder {
    friend class JPEGEncoderImpl;
  private:

    /** Internal buffer size in bytes (used for both reading and writing). */
    static const unsigned int BUFFER_SIZE = 4096;
  public:

    /**
      Initializes encoder.
    */
    JPEGEncoder() throw();

    /**
      Returns a description of the encoder.
    */
    String getDescription() const throw();
    
    /**
      Returns the default extension.
    */
    String getDefaultExtension() const throw();
    
    /**
      Reads a color image from the specified file.
      
      @param filename The path of the file.
    */
    Array<String> getExtensions() const throw();

    /**
      Returns true if the file seems to be a valid.
      
      @param filename The path of the file.
    */
    bool isValid(const String& filename) throw(IOException);
    
    /**
      Reads a color image from the specified file.
      
      @param filename The path of the file.
    */
    ColorImage* read(const String& filename) throw(InvalidFormat, IOException);
    
    /**
      Writes the specified image to the specified file.
      
      @param filename The path of the file.
      @param image The image to be written.
    */
    void write(const String& filename, const ColorImage* image) throw(ImageException, IOException);

    /**
      Writes the specified image to the specified file.
      
      @param filename The path of the file.
      @param image The image to be written.
    */
    void writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException);

    /**
      Returns information about the specified image.
    */
    HashTable<String, AnyValue> getInformation(const String& filename) throw(IOException);
  };

}; // end of gip namespace

#endif
