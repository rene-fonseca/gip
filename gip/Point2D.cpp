/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/Point2D.h>

namespace gip {

  FormatOutputStream& operator<<(FormatOutputStream& stream, const Point2D& value) {
    FormatOutputStream::PushContext pushContext(stream); // make current context the default context
    return stream << '(' << value.getColumn() << ',' << value.getRow() << ')';
  }

}; // end of gip namespace
