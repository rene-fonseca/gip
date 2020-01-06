/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/io/ImageEncoder.h>

namespace gip {

  /**
    Joint Photographic Experts Group (JPEG) format encoder/decoder.
    
    @short Joint Photographic Experts Group (JPEG) encoder/decoder
    @ingroup imageEncoders
    @version 1.1
  */

  class _COM_AZURE_DEV__GIP__API JPEGEncoder : public ImageEncoder {
    friend class JPEGEncoderImpl;
  private:

    /** Internal buffer size in bytes (used for both reading and writing). */
    static const unsigned int BUFFER_SIZE = 4096;
  public:

    /**
      Initializes encoder.
    */
    JPEGEncoder() noexcept;

    /**
      Returns a description of the encoder.
    */
    String getDescription() const noexcept;
    
    /**
      Returns the default extension.
    */
    String getDefaultExtension() const noexcept;
    
    /**
      Reads a color image from the specified file.
      
      @param filename The path of the file.
    */
    Array<String> getExtensions() const noexcept;

    /**
      Returns true if the file seems to be a valid.
      
      @param filename The path of the file.
    */
    bool isValid(const String& filename);
    
    /**
      Reads a color image from the specified file.
      
      @param filename The path of the file.
    */
    ColorImage* read(const String& filename);
    
    /**
      Writes the specified image to the specified file.
      
      @param filename The path of the file.
      @param image The image to be written.
    */
    void write(const String& filename, const ColorImage* image);

    /**
      Writes the specified image to the specified file.
      
      @param filename The path of the file.
      @param image The image to be written.
    */
    void writeGray(const String& filename, const GrayImage* image);

    /**
      Returns information about the specified image.
    */
    HashTable<String, AnyValue> getInformation(const String& filename);
  };

}; // end of gip namespace
