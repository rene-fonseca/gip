/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__IMAGE_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__IMAGE_ENCODER_H

#include <base/Object.h>
#include <base/string/String.h>
#include <base/io/InputStream.h>
#include <base/io/OutputStream.h>
#include <gip/ArrayImage.h>
#include <gip/io/InvalidFormat.h>

namespace base {
  // TAG: temporary solution
  extern __declspec(dllimport) FormatOutputStream fout;
  extern __declspec(dllimport) FormatOutputStream ferr;
};

using namespace base;

namespace gip {

/**
  @short Interface implemented by image encoders/decoders.
  @author René Møller Fonseca
*/

class ImageEncoder : public Object {
public:

  /**
    Returns a description of the encoder.
  */
  virtual String getDescription() const throw() = 0;

  /**
    Returns the default extension.
  */
  virtual String getDefaultExtension() const throw() = 0;

  /**
    Returns true if the format is valid.
  */
  virtual bool isValid() throw(IOException) = 0;

  /**
    Reads a color image from the stream.

    @param stream The input stream.
  */
  virtual ArrayImage<ColorPixel>* read() throw(IOException) = 0;

  /**
    Writes the specified image to the stream.

    @param stream The output stream.
    @param image The image to be written.
  */
  virtual void write(const ArrayImage<ColorPixel>* image) throw(IOException) = 0;

  /**
    Returns a description of the object.
  */
  virtual FormatOutputStream& getInfo(FormatOutputStream& stream) throw(IOException) = 0;
};

}; // end of namespace

#endif
