/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__YUV_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__YUV_ENCODER_H

#include <gip/gip.h>
#include <gip/io/ImageEncoder.h>

namespace gip {
  
  /**
    Abekas YUV format encoder/decoder. All images are 720 by 486 pixels.
    
    @short Abekas YUV format encoder
    @ingroup imageEncoders
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  
  class YUVEncoder : public ImageEncoder {
  public:

    /** The width of images. */
    static const unsigned int WIDTH = 720;
    /** The height of images. */
    static const unsigned int HEIGHT = 486;

    /**
      Initializes the encoder.
    */
    YUVEncoder() throw();
    
    String getDescription() const throw();
    
    String getDefaultExtension() const throw();
    
    bool isValid(const String& filename) throw(IOException);
    
    ColorImage* read(const String& filename) throw(InvalidFormat, IOException);
    
    void write(const String& filename, const ColorImage* image) throw(ImageException, IOException);
    
    void writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException);
    
    FormatOutputStream& getInfo(FormatOutputStream& stream, const String& filename) throw(IOException);
  };
  
}; // end of gip namespace

#endif
