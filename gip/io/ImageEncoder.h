/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__IMAGE_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__IMAGE_ENCODER_H

#include <base/Object.h>
#include <base/string/String.h>
#include <base/io/InputStream.h>
#include <base/io/OutputStream.h>
#include <base/collection/Array.h>
#include <gip/ArrayImage.h>
#include <gip/ImageException.h>
#include <base/string/InvalidFormat.h>

namespace gip {

  /**
    @defgroup imageEncoders Image Encoders
  */

  /**
    Image encoding and decoding interface.
  
    @short Interface implemented by image encoders/decoders
    @ingroup imageEncoders
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.1
  */
  
  class ImageEncoder : public Object {
  public:

    /** Exception cause. */
    enum ExceptionCause {
      DIMENSION_NOT_SUPPORTED = 1,
      INVALID_COLOR_TABLE,
      INVALID_COLOR
    };
    
    /**
      Returns a description of the encoder.
    */
    virtual String getDescription() const throw() = 0;
    
    /**
      Returns the default extension.
    */
    virtual String getDefaultExtension() const throw() = 0;
    
    /**
      Returns an array of extensions. The default is to return the default
      extension.
    */
    virtual Array<String> getExtensions() const throw();
    
    /**
      Returns true if the file seems to be a valid.
      
      @param filename The path of the file.
    */
    virtual bool isValid(const String& filename) throw(IOException) = 0;
    
    /**
      Reads a color image from the specified file.
      
      @param filename The path of the file.
    */
    virtual ArrayImage<ColorPixel>* read(const String& filename) throw(InvalidFormat, IOException) = 0;
    
    /**
      Writes the specified image to the specified file.

      @param filename The path of the file.
      @param image The image to be written.
    */
    virtual void write(const String& filename, const ArrayImage<ColorPixel>* image) throw(ImageException, IOException) = 0;
    
    /**
      Returns a description of the object.

      @param stream The stream to write the information to.
      @param filename The path of the file.
    */
    virtual FormatOutputStream& getInfo(FormatOutputStream& stream, const String& filename) throw(IOException) = 0;
  };
  
}; // end of namespace

#endif
