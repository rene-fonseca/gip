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
    Graphics Interchange Format (GIF) encoder/decoder. The Graphics Interchange
    Format(c) is the Copyright property of CompuServe Incorporated. GIF(sm) is
    a Service Mark property of CompuServe Incorporated.
     
    @short Graphics Interchange Format (GIF) encoder.
    @ingroup imageEncoders
    @version 1.1
  */

  class _COM_AZURE_DEV__GIP__API GIFEncoder : public ImageEncoder {
  public:

    /**
      Initializes the encoder.
    */
    GIFEncoder() noexcept;

    /**
      Returns a description of the encoder.
    */
    String getDescription() const noexcept;
    
    /**
      Returns the default extension.
    */
    String getDefaultExtension() const noexcept;
    
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
      Returns information about the specified image.
    */
    ArrayMap<String, AnyValue> getInformation(const String& filename);
  };

}; // end of gip namespace
