/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__GIF_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__GIF_ENCODER_H

#include <gip/gip.h>
#include <gip/io/ImageEncoder.h>

namespace gip {

  /**
    Graphics Interchange Format (GIF) encoder/decoder. The Graphics Interchange
    Format(c) is the Copyright property of CompuServe Incorporated. GIF(sm) is
    a Service Mark property of CompuServe Incorporated.
     
    @short Graphics Interchange Format (GIF) encoder/decoder.
    @ingroup imageEncoders
    @author Rene Moeller Fonseca
    @version 1.1
  */

  class GIFEncoder : public ImageEncoder {
  public:

    /**
      Initializes the encoder.
    */
    GIFEncoder() throw();

    String getDescription() const throw();

    String getDefaultExtension() const throw();

    bool isValid(const String& filename) throw(IOException);

    ColorImage* read(const String& filename) throw(InvalidFormat, IOException);

    void write(const String& filename, const ColorImage* image) throw(IOException);

    FormatOutputStream& getInfo(FormatOutputStream& stream, const String& filename) throw(IOException);
  };

}; // end of namespace

#endif
