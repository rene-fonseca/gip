/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__PPM_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__PPM_ENCODER_H

#include <gip/gip.h>
#include <gip/io/ImageEncoder.h>

namespace gip {
  
  /**
    Portable pixmap (PPM) file format encoder/decoder.
    
    @short Portable pixmap (PPM) file format encoder/decoder.
    @ingroup imageEncoders
    @author Rene Moeller Fonseca
    @version 1.0
  */
  
  class PPMEncoder : public ImageEncoder {
  public:
    
    /**
      Initializes the encoder.
    */
    PPMEncoder() throw();
    
    String getDescription() const throw();
    
    String getDefaultExtension() const throw();
    
    bool isValid(const String& filename) throw(IOException);
    
    ColorImage* read(const String& filename) throw(InvalidFormat, IOException);
    
    void write(const String& filename, const ColorImage* image) throw(ImageException, IOException);
    
    void write(const String& filename, const ArrayImage<RGBAPixel>* image) throw(ImageException, IOException);
    
    void writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException);
    
    FormatOutputStream& getInfo(FormatOutputStream& stream, const String& filename) throw(IOException);
  };
  
}; // end of namespace

#endif