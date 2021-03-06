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

#include <base/Object.h>
#include <base/AnyValue.h>
#include <base/io/InputStream.h>
#include <base/io/OutputStream.h>
#include <base/collection/Array.h>
#include <base/collection/ArrayMap.h>
#include <base/string/InvalidFormat.h>
#include <gip/ArrayImage.h>
#include <gip/ImageException.h>

namespace gip {

  /**
    @defgroup imageEncoders Image Encoders
  */

  /**
    Image encoding and decoding interface.
  
    @short Interface implemented by image encoders/decoders
    @ingroup imageEncoders
    @version 1.1
  */
  
  class _COM_AZURE_DEV__GIP__API ImageEncoder : public Object {
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
    virtual String getDescription() const noexcept = 0;
    
    /**
      Returns the default extension.
    */
    virtual String getDefaultExtension() const noexcept = 0;
    
    /**
      Returns an array of extensions. The default is to return the default
      extension.
    */
    virtual Array<String> getExtensions() const noexcept;
    
    /**
      Returns true if the file seems to be a valid.
      
      @param filename The path of the file.
    */
    virtual bool isValid(const String& filename) = 0;
    
    /**
      Reads a color image from the specified file.
      
      @param filename The path of the file.
    */
    virtual ArrayImage<ColorPixel>* read(const String& filename) = 0;
    
    /**
      Writes the specified image to the specified file.

      @param filename The path of the file.
      @param image The image to be written.
    */
    virtual void write(const String& filename, const ArrayImage<ColorPixel>* image) = 0;
    
    /**
      Returns a description of the object.

      @param stream The stream to write the information to.
      @param filename The path of the file.
    */
    virtual ArrayMap<String, AnyValue> getInformation(const String& filename) = 0;
  };
  
}; // end of gip namespace
