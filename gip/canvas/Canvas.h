/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_DRAW__CANVAS_H
#define _DK_SDU_MIP__GIP_DRAW__CANVAS_H

#include <gip/ArrayImage.h>
#include <gip/Point.h>

namespace gip {

  /**
    Canvas.

    @short Canvas
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  class Canvas {
  private:

    typedef ColorImage::Pixel Pixel;

    ColorImage* canvas;
    Dimension dimension;
    ColorImage::Rows rows;

    void lineClipped(const Point& p1, const Point& p2, ColorPixel color) throw();
  public:

    Canvas(ColorImage* canvas) throw();

    void clip(Point& p1, Point& p2) const throw();

    void image(const Point& offset, const ColorImage& image) throw();

    void circle(const Point& p1, unsigned int radius, ColorImage::Pixel color) throw();

    void ellipse(const Point& p1, int a, int b, ColorImage::Pixel color) throw();

    void fill(const Point& p1, const Point& p2, ColorImage::Pixel color) throw();

    void rectangle(const Point& p1, const Point& p2, ColorImage::Pixel color) throw();

    void pixel(const Point& point, ColorImage::Pixel color) throw();

    void line(const Point& p1, const Point& p2, ColorImage::Pixel color) throw();
  };

}; // end of gip namespace

#endif
