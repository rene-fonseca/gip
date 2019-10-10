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
    Tag Image File Format (TIFF) encoder/decoder.
     
    @short Tag Image File Format (TIFF) encoder
    @ingroup imageEncoders
    @version 1.1
  */
  
  class _COM_AZURE_DEV__BASE__API TIFFEncoder : public ImageEncoder {
  public:
    
    /**
      Initializes the encoder.
    */
    TIFFEncoder() throw();

    /**
      Returns a description of the encoder.
    */
    String getDescription() const throw();
    
    /**
      Returns the default extension.
    */
    String getDefaultExtension() const throw();
    
    /**
      Returns an array of extensions.
    */
    Array<String> getExtensions() const throw();
  };
  
}; // end of gip namespace
