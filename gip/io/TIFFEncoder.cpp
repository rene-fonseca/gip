/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/TIFFEncoder.h>

namespace gip {

  TIFFEncoder::TIFFEncoder() throw() {
  }
  
  String TIFFEncoder::getDescription() const throw() {
    return MESSAGE("Tag Image File Format");
  }
  
  String TIFFEncoder::getDefaultExtension() const throw() {
    return MESSAGE("tiff");
  }

  Array<String> TIFFEncoder::getExtensions() const throw() {
    Array<String> extensions;
    extensions.append(MESSAGE("tif"));
    extensions.append(MESSAGE("tiff"));
    return extensions;
  }

}; // end of namespace
