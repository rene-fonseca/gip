/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__BMP_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__BMP_ENCODER_H

#include <gip/gip.h>
#include <gip/io/ImageEncoder.h>

namespace gip {

/**
  Windows Bitmap (BMP) format encoder/decoder.

  @short BMP format encoder/decoder.
  @author René Møller Fonseca
*/

class BMPEncoder : public ImageEncoder {
private:

  const String filename;
public:

  /** Compression. */
  enum Compression {
    RGB = 0, /**< Uncompressed image. */
    RLE8 = 1, /**< Run-length encoded (RLE) 8-bit format. */
    RLE4 = 2 /**< Run-length encoded (RLE) 4-bit format. */
  };
  
  /**
    Initializes Windows Bitmap encoder.
  */
  BMPEncoder(const String& filename) throw();

  String getDescription() const throw();

  String getDefaultExtension() const throw();

  bool isValid() throw(IOException);

  ColorImage* read() throw(InvalidFormat, IOException);

  void write(const ColorImage* image) throw(IOException);

  void writeGray(const GrayImage* image) throw(IOException);

  FormatOutputStream& getInfo(FormatOutputStream& stream) throw(IOException);
};

}; // end of namespace

#endif
