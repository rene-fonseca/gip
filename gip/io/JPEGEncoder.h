/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__JPEG_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__JPEG_ENCODER_H

#include <gip/io/ImageEncoder.h>

namespace gip {

  /**
    Joint Photographic Experts Group (JPEG) format encoder/decoder.
    
    @short JPEG format encoder/decoder.
    @ingroup imageEncoders
    @author Rene Moeller Fonseca
    @version 1.1
  */

  class JPEGEncoder : public ImageEncoder {
    friend class JPEGEncoderImpl;
  private:

    /** Internal buffer size in bytes (used for both reading and writing). */
    static const unsigned int BUFFER_SIZE = 4096;
  public:
	
    JPEGEncoder() throw();
    
    String getDescription() const throw();
    
    String getDefaultExtension() const throw();

    Array<String> getExtensions() const throw();

    bool isValid(const String& filename) throw(IOException);
    
    ColorImage* read(const String& filename) throw(InvalidFormat, IOException);
    
    void write(const String& filename, const ColorImage* image) throw(ImageException, IOException);

    void writeGray(const String& filename, const GrayImage* image) throw(ImageException, IOException);

    FormatOutputStream& JPEGEncoder::getInfo(FormatOutputStream& stream, const String& filename) throw(IOException);
    
  };

}; // end of namespace

#endif
