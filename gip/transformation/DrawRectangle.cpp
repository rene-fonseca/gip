/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/DrawRectangle.h>

namespace gip {

DrawRectangle::DrawRectangle(DestinationImage* image, const Region& r, ColorPixel c) throw() :
  UnaryTransformation<DestinationImage>(image), region(r), color(c) {
}

void DrawRectangle::operator()() throw() {
  ColorPixel* elements = destination->getElements();

  int left = region.getOffset().getColumn();
  int top = region.getOffset().getRow();
  unsigned int width = region.getDimension().getWidth();
  unsigned int height = region.getDimension().getHeight();
//  int right = left + width - 1;
//  int bottom = top + height - 1;

  unsigned int columns = destination->getDimension().getWidth();
  unsigned int rows = destination->getDimension().getHeight();

  for (unsigned int r = top; r < (height + top); ++r) {
    for (unsigned int c = left; c < (width + left); ++c) {
      if ((r < rows) && (c < columns)) {
        elements[r * columns + c] = color;
      }
    }
  }

//  if ((left >= columns) || (top >= rows)) {
//    return;
//  }
//
//  // draw horizontal top
//  if ((top < rows) && (top > 0) && (left < columns)) {
////    ColorPixel* p = elements + top * rows + maximum(left, 0);
////    const ColorPixel* q = p + minimum(width, columns - left);
////    while (p < q) {
////      *p++ = color;
////    }
//  }
//
//  // draw horizontal bottom
//  if ((bottom > 0) && (bottom < rows)) {
//  }
//
//  // draw vertical left
//  if ((left > 0) && (left < columns)) {
//  }
//
//  // draw vertical right
//  if ((right > 0) && (right < columns)) {
//  }
}

}; // end of namespace
