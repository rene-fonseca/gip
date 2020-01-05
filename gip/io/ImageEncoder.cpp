/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/ImageEncoder.h>

namespace gip {

  Array<String> ImageEncoder::getExtensions() const noexcept {
    Array<String> extensions;
    extensions.append(getDefaultExtension());
    return extensions;
  }
  
};
