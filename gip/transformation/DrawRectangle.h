/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__DRAW_RECTANGLE_H
#define _DK_SDU_MIP__GIP__DRAW_RECTANGLE_H

#include <gip/transformation/UnaryTransformation.h>
#include <gip/Region.h>
#include <gip/Image.h>

namespace gip {

/**
  Draw a rectangle on the specified image.

  @short Draw rectagle
  @author René Møller Fonseca
*/

class DrawRectangle : public UnaryTransformation<ColorImage> {
protected:

  /** Specification of boundaries of rectangle */
  const Region region;
  /** The color of the . */
  ColorPixel color;
public:

  /**
    Initializes the transformation.
  */
  DrawRectangle(DestinationImage* image, const Region& region, ColorPixel color) throw();

  /**
    Draws the rectangle on the image.
  */
  void operator()() throw();
};

}; // end of namespace

#endif
