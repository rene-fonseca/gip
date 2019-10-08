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

#include <gip/gip.h>
#include <gip/io/ImageEncoder.h>

namespace gip {

  /**
    Windows Bitmap (BMP) format encoder/decoder.
    
    @short Windows Bitmap (BMP) format encoder
    @ingroup imageEncoders
    @version 1.2
  */

  class BMPEncoder : public ImageEncoder {
  public:

    /** Compression. */
    enum Compression {
      RGB = 0, /**< Uncompressed image. */
      RLE8 = 1, /**< Run-length encoded (RLE) 8-bit format. */
      RLE4 = 2 /**< Run-length encoded (RLE) 4-bit format. */
    };
  
    /**
      Initializes Windows Bitmap encoder.
    */
    BMPEncoder() throw();

    /**
      Returns a description of the encoder.
    */
    String getDescription() const throw();
    
    /**
      Returns the default extension.
    */
    String getDefaultExtension() const throw();
    
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
    void write(const String& filename, const ArrayImage<ColorPixel>* image) throw(ImageException, IOException);
    
    /**
      Returns a description of the object.

      @param stream The stream to write the information to.
      @param filename The path of the file.
    */
    void writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException);    
    
    /**
      Returns information about the specified image.
    */
    HashTable<String, AnyValue> getInformation(const String& filename) throw(IOException);
  };

}; // end of gip namespace
