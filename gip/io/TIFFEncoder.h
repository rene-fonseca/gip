/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__TIFF_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__TIFF_ENCODER_H

#include <gip/io/ImageEncoder.h>

namespace gip {

  /**
    Tag Image File Format (TIFF) encoder/decoder.
     
    @short Tag Image File Format (TIFF) encoder
    @ingroup imageEncoders
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.1
  */
  
  class TIFFEncoder : public ImageEncoder {
  public:
    
    TIFFEncoder() throw();

    /**
      Returns a description of the encoder.
    */
    virtual String getDescription() const throw() = 0;
    
    /**
      Returns the default extension.
    */
    virtual String getDefaultExtension() const throw() = 0;
    
    /**
      Returns an array of extensions.
    */
    virtual Array<String> getExtensions() const throw();    
  };
  
}; // end of gip namespace

#endif
