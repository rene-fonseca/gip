/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Test.h>

namespace gip {

  Test2::Test2(DestinationImage* destination) throw() : UnaryTransformation<DestinationImage>(destination) {
  }

  void Test2::operator()() throw() {
    ColorPixel* element = destination->getElements();

    for (unsigned int row = 0; row < destination->getDimension().getHeight(); ++row) {
      for (unsigned int column = 0; column < destination->getDimension().getWidth(); ++column) {
        ColorPixel temp;
        temp.blue = (column % 0xff);
        temp.green = (row % 0xff);
        temp.red = ((column + row) % 0xff);
        *element++ = temp;
      }
    }
  }

}; // end of namespace
