/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

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
  Graphics Interchange Format (GIF) encoder/decoder.
  The Graphics Interchange Format(c) is the Copyright property of CompuServe
  Incorporated. GIF(sm) is a Service Mark property of CompuServe Incorporated.

  @short GIF encoder/decoder.
  @author René Møller Fonseca
*/

class GIFEncoder : public ImageEncoder {
private:

  const String filename;
public:

  /**
    Initializes Windows Bitmap encoder.
  */
  GIFEncoder(const String& filename) throw();

  String getDescription() const throw();

  String getDefaultExtension() const throw();

  bool isValid() throw(IOException);

  ColorImage* read() throw(InvalidFormat, IOException);

  void write(const ColorImage* image) throw(IOException);

  FormatOutputStream& getInfo(FormatOutputStream& stream) throw(IOException);
};

}; // end of namespace

#endif
