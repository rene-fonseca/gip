/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/Point.h>

namespace gip {

  FormatOutputStream& operator<<(FormatOutputStream& stream, const Point& value) throw(IOException) {
    FormatOutputStream::PushContext pushContext(stream); // make current context the default context
    return stream << '(' << value.getX() << ',' << value.getY() << ')';
  }

}; // end of gip namespace
