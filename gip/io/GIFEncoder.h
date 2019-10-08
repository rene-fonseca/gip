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
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.1
  */

  class GIFEncoder : public ImageEncoder {
  public:

    /**
      Initializes the encoder.
    */
    GIFEncoder() throw();

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
    void write(const String& filename, const ColorImage* image) throw(IOException);

    /**
      Returns information about the specified image.
    */
    HashTable<String, AnyValue> getInformation(const String& filename) throw(IOException);
  };

}; // end of gip namespace
