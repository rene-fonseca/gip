/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__RAS_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__RAS_ENCODER_H

#include <gip/gip.h>
#include <gip/io/ImageEncoder.h>

namespace gip {
  
  /**
    Sun rasterfile (RAS) format encoder/decoder.
    
    @short Sun rasterfile (RAS) format encoder
    @ingroup imageEncoders
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  
  class RASEncoder : public ImageEncoder {
  public:
    
    /** Specifies the size of the internal buffer use for reading and writing. */
    static const unsigned int BUFFER_SIZE = 4096 * 4;
    
    /**
      Initializes the encoder.
    */
    RASEncoder() throw();
    
    String getDescription() const throw();
    
    String getDefaultExtension() const throw();
    
    Array<String> getExtensions() const throw();
    
    bool isValid(const String& filename) throw(IOException);
    
    ColorImage* read(const String& filename) throw(InvalidFormat, IOException);
    
    void write(const String& filename, const ColorImage* image) throw(ImageException, IOException);
    
    void writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException);
    
    FormatOutputStream& getInfo(FormatOutputStream& stream, const String& filename) throw(IOException);
  };
  
}; // end of gip namespace

#endif
