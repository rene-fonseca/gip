/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Test.h>
#include <gip/canvas/Canvas.h>

namespace gip {

  Test::Test(DestinationImage* destination) throw() : UnaryTransformation<DestinationImage>(destination) {
  }

  void Test::operator()() throw() {
    ColorPixel* element = destination->getElements();

    Canvas canvas(destination);
    canvas.fill(Point(0, 0), Point(destination->getDimension().getWidth() - 1, destination->getDimension().getHeight() - 1), makeColorPixel(48, 32, 128));

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

}; // end of gip namespace
