/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Ren� M�ller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__PCX_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__PCX_ENCODER_H

#include <gip/gip.h>
#include <gip/io/ImageEncoder.h>

namespace gip {

/**

  @short PCX format encoder/decoder.
  @author Ren� M�ller Fonseca
*/

class PCXEncoder : public ImageEncoder {
private:

  const String filename;
public:

  /**
    Initializes Windows Bitmap encoder.
  */
  PCXEncoder(const String& filename) throw();

  String getDescription() const throw();

  String getDefaultExtension() const throw();

  bool isValid() throw(IOException);

  ColorImage* read() throw(IOException);

  void write(const ColorImage* image) throw(IOException);

  void writeGray(const GrayImage* image) throw(IOException);

  FormatOutputStream& getInfo(FormatOutputStream& stream) throw(IOException);
};

}; // end of namespace

#endif