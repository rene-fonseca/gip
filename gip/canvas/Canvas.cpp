/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/canvas/Canvas.h>
#include <base/math/Math.h>

namespace gip {

  Canvas::Canvas(ColorImage* _canvas) noexcept
     : canvas(_canvas),
       dimension(_canvas->getDimension()),
       rows(_canvas->getRows())
  {
  }

  Dimension Canvas::getDimension() const noexcept
  {
    return dimension;
  }

  bool Canvas::clip(Point& p1, Point& p2) const noexcept
  {
    // Cohen-Sutherland line clipping
    enum {BOTTOM = 1, TOP = 2, LEFT = 4, RIGHT = 8};

    const Point delta = p2 - p1;
    Point clipped1 = p1;
    Point clipped2 = p2;

    unsigned int clip1 = 0;
    if (clipped1.getX() < 0) {
      clip1 |= LEFT;
    } else if (static_cast<unsigned int>(clipped1.getX()) >= dimension.getWidth()) {
      clip1 |= RIGHT;
    }
    if (clipped1.getY() < 0) {
      clip1 |= BOTTOM;
    } else if (static_cast<unsigned int>(clipped1.getY()) >= dimension.getHeight()) {
      clip1 |= TOP;
    }

    unsigned int clip2 = 0;
    if (clipped2.getX() < 0) {
      clip2 |= LEFT;
    } else if (static_cast<unsigned int>(clipped2.getX()) >= dimension.getWidth()) {
      clip2 |= RIGHT;
    }
    if (clipped2.getY() < 0) {
      clip2 |= BOTTOM;
    } else if (static_cast<unsigned int>(clipped2.getY()) >= dimension.getHeight()) {
      clip2 |= TOP;
    }

    while ((clip1 | clip2) != 0) { // continue until trivial accept
      if ((clip1 & clip2) != 0) { // reject line 'cause completely on one side
        return false;
      }

      if ((clip1 & (TOP | BOTTOM)) != 0) {
        int y = ((clip1 & BOTTOM) != 0) ? 0 : (dimension.getHeight() - 1);
        clipped1 = Point(delta.getX() * (y - p1.getY())/delta.getY() + p1.getX(), y);
        if (clipped1.getX() < 0) {
          clip1 = LEFT;
        } else if (static_cast<unsigned int>(clipped1.getX()) >= dimension.getWidth()) {
          clip1 = RIGHT;
        } else {
          clip1 = 0;
        }
      } else if ((clip1 & (LEFT | RIGHT)) != 0) {
        int x = ((clip1 & LEFT) != 0) ? 0 : (dimension.getWidth() - 1);
        clipped1 = Point(x, delta.getY() * (x - p1.getX())/delta.getX() + p1.getY());
        if (clipped1.getY() < 0) {
          clip1 = BOTTOM;
        } else if (static_cast<unsigned int>(clipped1.getY()) >= dimension.getHeight()) {
          clip1 = TOP;
        } else {
          clip1 = 0;
        }
      } else if ((clip2 & (TOP | BOTTOM)) != 0) {
        int y = ((clip2 & BOTTOM) != 0) ? 0 : (dimension.getHeight() - 1);
        clipped2 = Point(delta.getX() * (y - p1.getY())/delta.getY() + p1.getX(), y);
        if (clipped2.getX() < 0) {
          clip2 = LEFT;
        } else if (static_cast<unsigned int>(clipped2.getX()) >= dimension.getWidth()) {
          clip2 = RIGHT;
        } else {
          clip2 = 0;
        }
      } else { // if ((clip2 & (LEFT | RIGHT)) != 0) { // LEFT or RIGHT must be set
        int x = ((clip2 & LEFT) != 0) ? 0 : (dimension.getWidth() - 1);
        clipped2 = Point(x, delta.getY() * (x - p1.getX())/delta.getX() + p1.getY());
        if (clipped2.getY() < 0) {
          clip2 = BOTTOM;
        } else if (static_cast<unsigned int>(clipped2.getY()) >= dimension.getHeight()) {
          clip2 = TOP;
        } else {
          clip2 = 0;
        }
      }
    }
    p1 = clipped1;
    p2 = clipped2;
    return true;
  }

  void Canvas::image(const Point& offset, const ColorImage& image) noexcept {
    Dimension srcDimension = image.getDimension();

    if (
      ((offset.getX() >= 0) && (static_cast<unsigned int>(offset.getX()) >= dimension.getWidth())) ||
      ((offset.getX() + srcDimension.getWidth()) < 0) ||
      ((offset.getY() >= 0) && (static_cast<unsigned int>(offset.getY()) >= dimension.getHeight())) ||
      ((offset.getY() + srcDimension.getHeight()) < 0)
    ) {
      return;
    }
    
    Point srcOffset(0, 0);
    Point destOffset = offset;
    if (destOffset.getX() < 0) {
      srcOffset.setX(-destOffset.getX());
      destOffset.setX(0);
    }
    if (destOffset.getY() < 0) {
      srcOffset.setY(-destOffset.getY());
      destOffset.setY(0);
    }

    Point srcEndOffset(srcDimension.getWidth(), srcDimension.getHeight());
    if (offset.getX() + srcDimension.getWidth() >= dimension.getWidth()) {
      srcEndOffset.setX(dimension.getWidth() - offset.getX());
    }
    if (offset.getY() + srcDimension.getHeight() >= dimension.getHeight()) {
      srcEndOffset.setY(dimension.getHeight() - offset.getY());
    }
    Point delta = srcEndOffset - srcOffset; // dimension of common area

    ColorImage::ReadableRows::RowIterator srcRow = image.getRows()[srcOffset.getY()];
    ColorImage::Rows::RowIterator row = rows[destOffset.getY()];
    const ColorImage::Rows::RowIterator endRow = rows[destOffset.getY() + delta.getY()];
    while (row != endRow) {
//      copy(row[destOffset.getX()], row[destOffset.getX() + delta.getX()], srcRow[srcOffset.getX()]);
      ColorImage::ReadableRows::RowIterator::ElementIterator src = srcRow[srcOffset.getX()];
      ColorImage::Rows::RowIterator::ElementIterator dest = row[destOffset.getX()];
      const ColorImage::Rows::RowIterator::ElementIterator endDest = row[destOffset.getX() + delta.getX()];
      while (dest != endDest) {
        *dest++ = *src++;
      }
      ++row;
      ++srcRow;
    }
  }

  void Canvas::circle(const Point& center, unsigned int radius, Pixel color, unsigned int options) noexcept {
    int d = 3 - 2 * radius;
    int x = 0;
    int y = radius;

    if (options & ANTIALIASING) {
      unsigned int yExactSquared = radius * radius; // calculated incrementally: radius * radius - x * x
      // int otherY = 0; // selects inner or outer pixel

      if (options & FILL) { // anti aliased filled circle
        int yLast = Math::iSqrt(radius * radius/2); // TAG: rounding problem
        while (x < y) {
          unsigned int yExact = Math::iSqrt8Round(yExactSquared); // scaled with 256 // TAG: could be optimized
          unsigned int error = yExact & 0xff; // use fraction - error = y - y' <= 0.5 - scaled with 256
          int yBlend = (yExact < static_cast<unsigned int>(y * 256)) ? y /*blend with inner*/ : (y + 1) /*blend with outer*/;

          pixelInternal(
            Point(x, yBlend) + center,
            blend(getPixelInternal(Point(x, yBlend) + center), color, 255, error)
          );
          pixelInternal(
            Point(x, -yBlend) + center,
            blend(getPixelInternal(Point(x, -yBlend) + center), color, 255, error)
          );
          pixelInternal(
            Point(-x, -yBlend) + center,
            blend(getPixelInternal(Point(-x, -yBlend) + center), color, 255, error)
          );
          pixelInternal(
            Point(-x, yBlend) + center,
            blend(getPixelInternal(Point(-x, yBlend) + center), color, 255, error)
          );

          pixelInternal(
            Point(yBlend, x) + center,
            blend(getPixelInternal(Point(yBlend, x) + center), color, 255, error)
          );
          pixelInternal(
            Point(yBlend, -x) + center,
            blend(getPixelInternal(Point(yBlend, -x) + center), color, 255, error)
          );
          pixelInternal(
            Point(-yBlend, -x) + center,
            blend(getPixelInternal(Point(-yBlend, -x) + center), color, 255, error)
          );
          pixelInternal(
            Point(-yBlend, x) + center,
            blend(getPixelInternal(Point(-yBlend, x) + center), color, 255, error)
          );
          
          line(Point(x, yBlend - 1) + center, Point(x, yLast) + center, color);
          line(Point(-x, yBlend - 1) + center, Point(-x, yLast) + center, color);
          line(Point(x, -yBlend + 1) + center, Point(x, -yLast) + center, color);
          line(Point(-x, -yBlend + 1) + center, Point(-x, -yLast) + center, color);
          line(Point(yBlend - 1, x) + center, Point(-yBlend + 1, x) + center, color);
          line(Point(yBlend - 1, -x) + center, Point(-yBlend + 1, -x) + center, color);

          if (d < 0) {
            d += 4 * x + 6;
          } else {
            --y;
            d += 4 * (x - y) + 6;
          }

          yExactSquared -= 2 * x + 1; // (a+1)^2 - a^2 = 2a + 1
          ++x;
        }
        if (x == y) {
          pixelInternal(Point(x, x) + center, color);
          pixelInternal(Point(x, -x) + center, color);
          pixelInternal(Point(-x, -x) + center, color);
          pixelInternal(Point(-x, x) + center, color);
        }
      } else { // anti aliased circle
        while (x < y) {
          unsigned int error = 0;
          unsigned int otherError = 0;
          int otherY = 0;
          unsigned int yExact = Math::iSqrt8Round(yExactSquared); // scaled with 256 // TAG: could be optimized
          if (yExact < static_cast<unsigned int>(y * 256)) {
            error = yExact & 0xff; // use fraction
            otherError = 255 - error;
            otherY = y - 1; // blend with inner pixel
          } else {
            otherError = yExact & 0xff; // use fraction
            error = 255 - otherError;
            otherY = y + 1; // blend with the outer pixel
          }

          pixelInternal(
            Point(x, y) + center,
            blend(getPixelInternal(Point(x, y) + center), color, 255, error)
          );
          pixelInternal(
            Point(x, otherY) + center,
            blend(getPixelInternal(Point(x, otherY) + center), color, 255, otherError)
          );
          pixelInternal(
            Point(x, -y) + center,
            blend(getPixelInternal(Point(x, -y) + center), color, 255, error)
          );
          pixelInternal(
            Point(x, -otherY) + center,
            blend(getPixelInternal(Point(x, -otherY) + center), color, 255, otherError)
          );
          pixelInternal(
            Point(-x, -y) + center,
            blend(getPixelInternal(Point(-x, -y ) + center), color, 255, error)
          );
          pixelInternal(
            Point(-x, -otherY) + center,
            blend(getPixelInternal(Point(-x, -otherY) + center), color, 255, otherError)
          );
          pixelInternal(
            Point(-x, y) + center,
            blend(getPixelInternal(Point(-x, y) + center), color, 255, error)
          );
          pixelInternal(
            Point(-x, otherY) + center,
            blend(getPixelInternal(Point(-x, otherY) + center), color, 255, otherError)
          );

          pixelInternal(
            Point(y, x) + center,
            blend(getPixelInternal(Point(y, x) + center), color, 255, error)
          );
          pixelInternal(
            Point(otherY, x) + center,
            blend(getPixelInternal(Point(otherY, x) + center), color, 255, otherError)
          );
          pixelInternal(
            Point(y, -x) + center,
            blend(getPixelInternal(Point(y, -x) + center), color, 255, error)
          );
          pixelInternal(
            Point(otherY, -x) + center,
            blend(getPixelInternal(Point(otherY, -x) + center), color, 255, otherError)
          );
          pixelInternal(
            Point(-y, -x) + center,
            blend(getPixelInternal(Point(-y, -x) + center), color, 255, error)
          );
          pixelInternal(
            Point(-otherY, -x) + center,
            blend(getPixelInternal(Point(-otherY, -x) + center), color, 255, otherError)
          );
          pixelInternal(
            Point(-y, x) + center,
            blend(getPixelInternal(Point(-y, x) + center), color, 255, error)
          );
          pixelInternal(
            Point(-otherY, x) + center,
            blend(getPixelInternal(Point(-otherY, x) + center), color, 255, otherError)
          );
          
          if (d < 0) {
            d += 4 * x + 6;
          } else {
            --y;
            d += 4 * (x - y) + 6;
          }

          yExactSquared -= 2 * x + 1; // (a+1)^2 - a^2 = 2a + 1
          ++x;
        }
        if (x == y) {
          pixelInternal(Point(x, x) + center, color);
          pixelInternal(Point(x, -x) + center, color);
          pixelInternal(Point(-x, -x) + center, color);
          pixelInternal(Point(-x, x) + center, color);
        }
      }
    } else {
      if (options & FILL) { // filled circle
        while (x < y) {
          line(Point(x, y) + center, Point(-x, y) + center, color);
          line(Point(x, -y) + center, Point(-x, -y) + center, color);
          line(Point(y, x) + center, Point(-y, x) + center, color);
          line(Point(y, -x) + center, Point(-y, -x) + center, color);
          if (d < 0) {
            d += 4 * x + 6;
          } else {
            --y;
            d += 4 * (x - y) + 6;
          }
          ++x;
        }
        if (x == y) {
          line(Point(-x, x) + center, Point(x, x) + center, color);
          line(Point(-x, -x) + center, Point(x, -x) + center, color);
        }
        return;
      } else { // normal circle
        while (x < y) {
          pixelInternal(Point(x, y) + center, color);
          pixelInternal(Point(x, -y) + center, color);
          pixelInternal(Point(-x, -y) + center, color);
          pixelInternal(Point(-x, y) + center, color);
          pixelInternal(Point(y, x) + center, color);
          pixelInternal(Point(y, -x) + center, color);
          pixelInternal(Point(-y, -x) + center, color);
          pixelInternal(Point(-y, x) + center, color);
          if (d < 0) {
            d += 4 * x + 6;
          } else {
            --y;
            d += 4 * (x - y) + 6;
          }
          ++x;
        }
        if (x == y) {
          pixelInternal(Point(x, x) + center, color);
          pixelInternal(Point(x, -x) + center, color);
          pixelInternal(Point(-x, -x) + center, color);
          pixelInternal(Point(-x, x) + center, color);
        }
      }
    }
  }

  void Canvas::ring(const Point& center, unsigned int innerRadius, unsigned int outerRadius, Pixel color, unsigned int options) noexcept
  {
    unsigned int radiusInner = minimum(innerRadius, outerRadius);
    unsigned int radiusOuter = maximum(innerRadius, outerRadius);

    int dInner = 3 - 2 * radiusInner;
    int yInner = radiusInner;
    int dOuter = 3 - 2 * radiusOuter;
    int yOuter = radiusOuter;
    int x = 0;

    if (options & ANTIALIASING) {
      // case 1: draw vertical lines between inner and outer circles (i.e. for first octant)
      while (x < yInner) {
        int error;

        unsigned int yOuterExact = Math::iSqrt8Round(radiusOuter*radiusOuter - x*x); // scaled with 256 // TAG: could be optimized
        error = yOuterExact & 0xff; // use fraction
        int yOuterBlend =
          (yOuterExact < static_cast<unsigned int>(yOuter * 256)) ? yOuter /*blend with inner*/ : (yOuter + 1) /*blend with outer*/;
        pixelInternal(
          Point(x, yOuterBlend) + center,
          blend(getPixelInternal(Point(x, yOuterBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(x, -yOuterBlend) + center,
          blend(getPixelInternal(Point(x, -yOuterBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, -yOuterBlend) + center,
          blend(getPixelInternal(Point(-x, -yOuterBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, yOuterBlend) + center,
          blend(getPixelInternal(Point(-x, yOuterBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(yOuterBlend, x) + center,
          blend(getPixelInternal(Point(yOuterBlend, x) + center), color, 255, error)
        );
        pixelInternal(
          Point(yOuterBlend, -x) + center,
          blend(getPixelInternal(Point(yOuterBlend, -x) + center), color, 255, error)
        );
        pixelInternal(
          Point(-yOuterBlend, -x) + center,
          blend(getPixelInternal(Point(-yOuterBlend, -x) + center), color, 255, error)
        );
        pixelInternal(
          Point(-yOuterBlend, x) + center,
          blend(getPixelInternal(Point(-yOuterBlend, x) + center), color, 255, error)
        );

        unsigned int yInnerExact = Math::iSqrt8Round(radiusInner*radiusInner - x*x); // scaled with 256 // TAG: could be optimized
        error = 255 - yInnerExact & 0xff; // use fraction
        int yInnerBlend =
          (yInnerExact < static_cast<unsigned int>(yInner * 256)) ? (yInner - 1) /*blend with outer*/ : yInner /*blend with inner*/;
        pixelInternal(
          Point(x, yInnerBlend) + center,
          blend(getPixelInternal(Point(x, yInnerBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(x, -yInnerBlend) + center,
          blend(getPixelInternal(Point(x, -yInnerBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, -yInnerBlend) + center,
          blend(getPixelInternal(Point(-x, -yInnerBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, yInnerBlend) + center,
          blend(getPixelInternal(Point(-x, yInnerBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(yInnerBlend, x) + center,
          blend(getPixelInternal(Point(yInnerBlend, x) + center), color, 255, error)
        );
        pixelInternal(
          Point(yInnerBlend, -x) + center,
          blend(getPixelInternal(Point(yInnerBlend, -x) + center), color, 255, error)
        );
        pixelInternal(
          Point(-yInnerBlend, -x) + center,
          blend(getPixelInternal(Point(-yInnerBlend, -x) + center), color, 255, error)
        );
        pixelInternal(
          Point(-yInnerBlend, x) + center,
          blend(getPixelInternal(Point(-yInnerBlend, x) + center), color, 255, error)
        );

        line(Point(x, yInnerBlend + 1) + center, Point(x, yOuterBlend - 1) + center, color); // right top
        line(Point(-x, yInnerBlend + 1) + center, Point(-x, yOuterBlend - 1) + center, color); // left top
        line(Point(x, -yInnerBlend - 1) + center, Point(x, -yOuterBlend + 1) + center, color); // right bottom
        line(Point(-x, -yInnerBlend - 1) + center, Point(-x, -yOuterBlend + 1) + center, color); // left bottom

        line(Point(yInnerBlend + 1, x) + center, Point(yOuterBlend - 1, x) + center, color); // right upper middle
        line(Point(-yInnerBlend - 1, x) + center, Point(-yOuterBlend + 1, x) + center, color); // left upper middle
        line(Point(-yInnerBlend - 1, -x) + center, Point(-yOuterBlend + 1, -x) + center, color); // left lower middle
        line(Point(yInnerBlend + 1, -x) + center, Point(yOuterBlend - 1, -x) + center, color); // right lower middle

        if (dInner < 0) {
          dInner += 4 * x + 6;
        } else {
          --yInner;
          dInner += 4 * (x - yInner) + 6;
        }

        if (dOuter < 0) {
          dOuter += 4 * x + 6;
        } else {
          --yOuter;
          dOuter += 4 * (x - yOuter) + 6;
        }
        ++x;
      }

      // case 2: draw horizontal lines between outer circle and last x of inner circle (i.e. for first octant)
      int xLast = x;
      while (x < yOuter) {
        unsigned int yOuterExact = Math::iSqrt8Round(radiusOuter*radiusOuter - x*x); // scaled with 256 // TAG: could be optimized
        int error = yOuterExact & 0xff; // use fraction
        int yOuterBlend =
          (yOuterExact < static_cast<unsigned int>(yOuter * 256)) ? yOuter /*blend with inner*/ : (yOuter + 1) /*blend with outer*/;
        pixelInternal(
          Point(x, yOuterBlend) + center,
          blend(getPixelInternal(Point(x, yOuterBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(x, -yOuterBlend) + center,
          blend(getPixelInternal(Point(x, -yOuterBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, -yOuterBlend) + center,
          blend(getPixelInternal(Point(-x, -yOuterBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, yOuterBlend) + center,
          blend(getPixelInternal(Point(-x, yOuterBlend) + center), color, 255, error)
        );
        pixelInternal(
          Point(yOuterBlend, x) + center,
          blend(getPixelInternal(Point(yOuterBlend, x) + center), color, 255, error)
        );
        pixelInternal(
          Point(yOuterBlend, -x) + center,
          blend(getPixelInternal(Point(yOuterBlend, -x) + center), color, 255, error)
        );
        pixelInternal(
          Point(-yOuterBlend, -x) + center,
          blend(getPixelInternal(Point(-yOuterBlend, -x) + center), color, 255, error)
        );
        pixelInternal(
          Point(-yOuterBlend, x) + center,
          blend(getPixelInternal(Point(-yOuterBlend, x) + center), color, 255, error)
        );

        line(Point(xLast, yOuterBlend - 1) + center, Point(x, yOuterBlend - 1) + center, color);
        line(Point(xLast, x) + center, Point(yOuterBlend - 1, x) + center, color);
        line(Point(xLast, -yOuterBlend + 1) + center, Point(x, -yOuterBlend + 1) + center, color);
        line(Point(xLast, -x) + center, Point(yOuterBlend - 1, -x) + center, color);
        line(Point(-xLast, -yOuterBlend + 1) + center, Point(-x, -yOuterBlend + 1) + center, color);
        line(Point(-xLast, -x) + center, Point(-yOuterBlend + 1, -x) + center, color);
        line(Point(-xLast, yOuterBlend - 1) + center, Point(-x, yOuterBlend - 1) + center, color);
        line(Point(-xLast, x) + center, Point(-yOuterBlend + 1, x) + center, color);

        if (dOuter < 0) {
          dOuter += 4 * x + 6;
        } else {
          --yOuter;
          dOuter += 4 * (x - yOuter) + 6;
        }
        ++x;
      }

      // TAG: fix inner cirle around x==y
      if (x == yOuter) {
        line(Point(xLast, yOuter) + center, Point(x - 1, yOuter) + center, color);
        line(Point(xLast, -yOuter) + center, Point(x - 1, -yOuter) + center, color);
        line(Point(-xLast, yOuter) + center, Point(-x + 1, yOuter) + center, color);
        line(Point(-xLast, -yOuter) + center, Point(-x + 1, -yOuter) + center, color);

        // TAG: case 1: only one blend - case 2: need to blend with vertical and horizontal
        unsigned int yOuterExact = Math::iSqrt8Round(radiusOuter*radiusOuter - x*x); // scaled with 256 // TAG: could be optimized
        unsigned int error = yOuterExact & 0xff; // use fraction
        pixelInternal(
          Point(x, yOuter) + center,
          blend(getPixelInternal(Point(x, yOuter) + center), color, 255, error)
        );
        pixelInternal(
          Point(x, -yOuter) + center,
          blend(getPixelInternal(Point(x, -yOuter) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, -yOuter) + center,
          blend(getPixelInternal(Point(-x, -yOuter) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, yOuter) + center,
          blend(getPixelInternal(Point(-x, yOuter) + center), color, 255, error)
        );
      } else {
        unsigned int yOuterExact = Math::iSqrt8Round(radiusOuter*radiusOuter - x*x); // scaled with 256 // TAG: could be optimized
        unsigned int error = yOuterExact & 0xff; // use fraction
        ++yOuter;
        pixelInternal(
          Point(x, yOuter) + center,
          blend(getPixelInternal(Point(x, yOuter) + center), color, 255, error)
        );
        pixelInternal(
          Point(x, -yOuter) + center,
          blend(getPixelInternal(Point(x, -yOuter) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, -yOuter) + center,
          blend(getPixelInternal(Point(-x, -yOuter) + center), color, 255, error)
        );
        pixelInternal(
          Point(-x, yOuter) + center,
          blend(getPixelInternal(Point(-x, yOuter) + center), color, 255, error)
        );
      }
    } else { // normal ring
      // case 1: draw vertical lines between inner and outer circles (i.e. for first octant)
      while (x < yInner) {
        line(Point(x, yInner) + center, Point(x, yOuter) + center, color);
        line(Point(x, -yInner) + center, Point(x, -yOuter) + center, color);
        line(Point(-x, -yInner) + center, Point(-x, -yOuter) + center, color);
        line(Point(-x, yInner) + center, Point(-x, yOuter) + center, color);
        line(Point(yInner, x) + center, Point(yOuter, x) + center, color);
        line(Point(-yInner, x) + center, Point(-yOuter, x) + center, color);
        line(Point(-yInner, -x) + center, Point(-yOuter, -x) + center, color);
        line(Point(yInner, -x) + center, Point(yOuter, -x) + center, color);

        if (dInner < 0) {
          dInner += 4 * x + 6;
        } else {
          --yInner;
          dInner += 4 * (x - yInner) + 6;
        }

        if (dOuter < 0) {
          dOuter += 4 * x + 6;
        } else {
          --yOuter;
          dOuter += 4 * (x - yOuter) + 6;
        }
        ++x;
      }

      // case 2: draw horizontal lines between outer circle and last x of inner circle (i.e. for first octant)
      int xLast = x;
      while (x < yOuter) {
        line(Point(xLast, yOuter) + center, Point(x, yOuter) + center, color);
        line(Point(xLast, x) + center, Point(yOuter, x) + center, color);
        line(Point(xLast, -yOuter) + center, Point(x, -yOuter) + center, color);
        line(Point(xLast, -x) + center, Point(yOuter, -x) + center, color);
        line(Point(-xLast, -yOuter) + center, Point(-x, -yOuter) + center, color);
        line(Point(-xLast, -x) + center, Point(-yOuter, -x) + center, color);
        line(Point(-xLast, yOuter) + center, Point(-x, yOuter) + center, color);
        line(Point(-xLast, x) + center, Point(-yOuter, x) + center, color);
        if (dOuter < 0) {
          dOuter += 4 * x + 6;
        } else {
          --yOuter;
          dOuter += 4 * (x - yOuter) + 6;
        }
        ++x;
      }

      if (x == yOuter) {
        line(Point(xLast, yOuter) + center, Point(x, yOuter) + center, color);
        line(Point(xLast, -yOuter) + center, Point(x, -yOuter) + center, color);
        line(Point(-xLast, yOuter) + center, Point(-x, yOuter) + center, color);
        line(Point(-xLast, -yOuter) + center, Point(-x, -yOuter) + center, color);
      }
    }
  }

  void Canvas::ellipse(const Point& center, const Dimension& dimension, Pixel color, unsigned int options) noexcept {
//    if ((point.getX() >= 0) && (point.getY() >= 0) && (point.getX() < dimension.getWidth()) && (point.getY() < dimension.getHeight())) {
//      rows[point.getY()][point.getX()] = color;
//    }
    int doubleSquareA = 2 * 2 * dimension.getWidth() * dimension.getWidth();
    int doubleSquareB = 2 * 2 * dimension.getHeight() * dimension.getHeight();
    int xChange = 2 * dimension.getHeight() * dimension.getHeight() * (1 - 2 * dimension.getWidth());
    int yChange = 2 * dimension.getWidth() * dimension.getWidth();
    int error = xChange/2;
    int xStopping = doubleSquareB * dimension.getWidth();
    int yStopping = 0;
    int x = dimension.getWidth();
    int y = 0;

    if (options & ANTIALIASING) {
      if (options & FILL) {
        while (xStopping >= yStopping) {
          unsigned int opacity;
          unsigned int otherOpacity;
          int otherX;
          int xExact = dimension.getWidth() * Math::iSqrt8(dimension.getHeight()*dimension.getHeight() - y * y)/dimension.getHeight(); // TAG: rounding problem
          if (xExact < 256 * x) {
            otherOpacity = 256 * x - xExact; // use fraction
            opacity = 255 - otherOpacity;
            otherX = x - 1; // blend with inner pixel
            pixelInternal(
              Point(x, y) + center,
              blend(getPixelInternal(Point(x, y) + center), color, 255, opacity)
            );
            pixelInternal(
              Point(x, -y) + center,
              blend(getPixelInternal(Point(x, -y) + center), color, 255, opacity)
            );
            pixelInternal(
              Point(-x, -y) + center,
              blend(getPixelInternal(Point(-x, -y ) + center), color, 255, opacity)
            );
            pixelInternal(
              Point(-x, y) + center,
              blend(getPixelInternal(Point(-x, y) + center), color, 255, opacity)
            );
            line(Point(-otherX, y) + center, Point(otherX, y) + center, color);
            line(Point(-otherX, -y) + center, Point(otherX, -y) + center, color);
          } else {
            otherOpacity = xExact - 256 * x; // use fraction
            opacity = 255 - otherOpacity;
            otherX = x + 1; // blend with the outer pixel
            pixelInternal(
              Point(otherX, y) + center,
              blend(getPixelInternal(Point(otherX, y) + center), color, 255, otherOpacity)
            );
            pixelInternal(
              Point(otherX, -y) + center,
              blend(getPixelInternal(Point(otherX, -y) + center), color, 255, otherOpacity)
            );
            pixelInternal(
              Point(-otherX, -y) + center,
              blend(getPixelInternal(Point(-otherX, -y) + center), color, 255, otherOpacity)
            );
            pixelInternal(
              Point(-otherX, y) + center,
              blend(getPixelInternal(Point(-otherX, y) + center), color, 255, otherOpacity)
            );
            line(Point(-x, y) + center, Point(x, y) + center, color);
            line(Point(-x, -y) + center, Point(x, -y) + center, color);
          }

          ++y;
          yStopping += doubleSquareA;
          error += yChange;
          yChange += doubleSquareA;
          if (error > 0) {
            --x;
            xStopping -= doubleSquareB;
            error += xChange;
            xChange += doubleSquareB;
          }
        }

        int lastY = y - 1;
        xChange = 2 * dimension.getHeight() * dimension.getHeight();
        yChange = 2 * dimension.getWidth() * dimension.getWidth() * (1 - 2 * dimension.getHeight());
        error = yChange/2;
        xStopping = 0;
        yStopping = doubleSquareA * dimension.getHeight();
        x = 0;
        y = dimension.getHeight();

        while (xStopping <= yStopping) {
          unsigned int opacity;
          unsigned int otherOpacity;
          int otherY;
          int yExact = dimension.getHeight() * Math::iSqrt8(dimension.getWidth()*dimension.getWidth() - x * x)/dimension.getWidth(); // TAG: rounding problem
          if (yExact < y * 256) {
            otherOpacity = 256 * y - yExact;
            opacity = 255 - otherOpacity;
            otherY = y - 1; // blend with inner pixel
            pixelInternal(
              Point(x, y) + center,
              blend(getPixelInternal(Point(x, y) + center), color, 255, opacity)
            );
            pixelInternal(
              Point(x, -y) + center,
              blend(getPixelInternal(Point(x, -y) + center), color, 255, opacity)
            );
            pixelInternal(
              Point(-x, -y) + center,
              blend(getPixelInternal(Point(-x, -y ) + center), color, 255, opacity)
            );
            pixelInternal(
              Point(-x, y) + center,
              blend(getPixelInternal(Point(-x, y) + center), color, 255, opacity)
            );
            line(Point(x, otherY) + center, Point(x, lastY) + center, color);
            line(Point(-x, otherY) + center, Point(-x, lastY) + center, color);
            line(Point(x, -otherY) + center, Point(x, -lastY) + center, color);
            line(Point(-x, -otherY) + center, Point(-x, -lastY) + center, color);
          } else {
            otherOpacity = yExact - 256 * y; // use fraction
            opacity = 255 - otherOpacity;
            otherY = y + 1; // blend with the outer pixel
            pixelInternal(
              Point(x, otherY) + center,
              blend(getPixelInternal(Point(x, otherY) + center), color, 255, otherOpacity)
            );
            pixelInternal(
              Point(x, -otherY) + center,
              blend(getPixelInternal(Point(x, -otherY) + center), color, 255, otherOpacity)
            );
            pixelInternal(
              Point(-x, -otherY) + center,
              blend(getPixelInternal(Point(-x, -otherY) + center), color, 255, otherOpacity)
            );
            pixelInternal(
              Point(-x, otherY) + center,
              blend(getPixelInternal(Point(-x, otherY) + center), color, 255, otherOpacity)
            );
            line(Point(x, y) + center, Point(x, lastY) + center, color);
            line(Point(-x, y) + center, Point(-x, lastY) + center, color);
            line(Point(x, -y) + center, Point(x, -lastY) + center, color);
            line(Point(-x, -y) + center, Point(-x, -lastY) + center, color);
          }

          ++x;
          xStopping += doubleSquareB;
          error += xChange;
          xChange += doubleSquareB;
          if (error > 0) {
            --y;
            yStopping -= doubleSquareA;
            error += yChange;
            yChange += doubleSquareA;
          }
        }
      } else { // anti aliased ellipsed
        while (xStopping >= yStopping) {
          unsigned int opacity;
          unsigned int otherOpacity;
          int otherX;
          int xExact = dimension.getWidth() * Math::iSqrt8(dimension.getHeight()*dimension.getHeight() - y * y)/dimension.getHeight(); // TAG: rounding problem
          if (xExact < 256 * x) {
            otherOpacity = 256 * x - xExact; // use fraction
            otherX = x - 1; // blend with inner pixel
          } else {
            otherOpacity = xExact - 256 * x; // use fraction
            otherX = x + 1; // blend with the outer pixel
          }
          opacity = 255 - otherOpacity;

          pixelInternal(
            Point(x, y) + center,
            blend(getPixelInternal(Point(x, y) + center), color, 255, opacity)
          );
          pixelInternal(
            Point(otherX, y) + center,
            blend(getPixelInternal(Point(otherX, y) + center), color, 255, otherOpacity)
          );
          pixelInternal(
            Point(x, -y) + center,
            blend(getPixelInternal(Point(x, -y) + center), color, 255, opacity)
          );
          pixelInternal(
            Point(otherX, -y) + center,
            blend(getPixelInternal(Point(otherX, -y) + center), color, 255, otherOpacity)
          );
          pixelInternal(
            Point(-x, -y) + center,
            blend(getPixelInternal(Point(-x, -y ) + center), color, 255, opacity)
          );
          pixelInternal(
            Point(-otherX, -y) + center,
            blend(getPixelInternal(Point(-otherX, -y) + center), color, 255, otherOpacity)
          );
          pixelInternal(
            Point(-x, y) + center,
            blend(getPixelInternal(Point(-x, y) + center), color, 255, opacity)
          );
          pixelInternal(
            Point(-otherX, y) + center,
            blend(getPixelInternal(Point(-otherX, y) + center), color, 255, otherOpacity)
          );

          ++y;
          yStopping += doubleSquareA;
          error += yChange;
          yChange += doubleSquareA;
          if (error > 0) {
            --x;
            xStopping -= doubleSquareB;
            error += xChange;
            xChange += doubleSquareB;
          }
        }

        xChange = 2 * dimension.getHeight() * dimension.getHeight();
        yChange = 2 * dimension.getWidth() * dimension.getWidth() * (1 - 2 * dimension.getHeight());
        error = yChange/2;
        xStopping = 0;
        yStopping = doubleSquareA * dimension.getHeight();
        x = 0;
        y = dimension.getHeight();

        while (xStopping <= yStopping) {
          unsigned int opacity;
          unsigned int otherOpacity;
          int otherY;
          int yExact = dimension.getHeight() * Math::iSqrt8(dimension.getWidth()*dimension.getWidth() - x * x)/dimension.getWidth(); // TAG: rounding problem
          if (yExact < y * 256) {
            otherOpacity = 256 * y - yExact; // use fraction
            otherY = y - 1; // blend with inner pixel
          } else {
            otherOpacity = yExact - 256 * y; // use fraction
            otherY = y + 1; // blend with the outer pixel
          }
          opacity = 255 - otherOpacity;

          pixelInternal(
            Point(x, y) + center,
            blend(getPixelInternal(Point(x, y) + center), color, 255, opacity)
          );
          pixelInternal(
            Point(x, otherY) + center,
            blend(getPixelInternal(Point(x, otherY) + center), color, 255, otherOpacity)
          );
          pixelInternal(
            Point(x, -y) + center,
            blend(getPixelInternal(Point(x, -y) + center), color, 255, opacity)
          );
          pixelInternal(
            Point(x, -otherY) + center,
            blend(getPixelInternal(Point(x, -otherY) + center), color, 255, otherOpacity)
          );
          pixelInternal(
            Point(-x, -y) + center,
            blend(getPixelInternal(Point(-x, -y ) + center), color, 255, opacity)
          );
          pixelInternal(
            Point(-x, -otherY) + center,
            blend(getPixelInternal(Point(-x, -otherY) + center), color, 255, otherOpacity)
          );
          pixelInternal(
            Point(-x, y) + center,
            blend(getPixelInternal(Point(-x, y) + center), color, 255, opacity)
          );
          pixelInternal(
            Point(-x, otherY) + center,
            blend(getPixelInternal(Point(-x, otherY) + center), color, 255, otherOpacity)
          );

          ++x;
          xStopping += doubleSquareB;
          error += xChange;
          xChange += doubleSquareB;
          if (error > 0) {
            --y;
            yStopping -= doubleSquareA;
            error += yChange;
            yChange += doubleSquareA;
          }
        }
      }
    } else { // normal ellipse
      if (options & FILL) {
        while (xStopping >= yStopping) {
          line(Point(-x, y) + center, Point(x, y) + center, color);
          line(Point(-x, -y) + center, Point(x, -y) + center, color);
          ++y;
          yStopping += doubleSquareA;
          error += yChange;
          yChange += doubleSquareA;
          if (error > 0) {
            --x;
            xStopping -= doubleSquareB;
            error += xChange;
            xChange += doubleSquareB;
          }
        }

        int lastY = y;
        xChange = 2 * dimension.getHeight() * dimension.getHeight();
        yChange = 2 * dimension.getWidth() * dimension.getWidth() * (1 - 2 * dimension.getHeight());
        error = yChange/2;
        xStopping = 0;
        yStopping = doubleSquareA * dimension.getHeight();
        x = 0;
        y = dimension.getHeight();

        while (xStopping <= yStopping) {
          line(Point(x, y) + center, Point(x, lastY) + center, color);
          line(Point(-x, y) + center, Point(-x, lastY) + center, color);
          line(Point(x, -y) + center, Point(x, -lastY) + center, color);
          line(Point(-x, -y) + center, Point(-x, -lastY) + center, color);
          ++x;
          xStopping += doubleSquareB;
          error += xChange;
          xChange += doubleSquareB;
          if (error > 0) {
            --y;
            yStopping -= doubleSquareA;
            error += yChange;
            yChange += doubleSquareA;
          }
        }
      } else { // normal ellipse
        while (xStopping >= yStopping) {
          pixelInternal(Point(x, y) + center, color);
          pixelInternal(Point(-x, y) + center, color);
          pixelInternal(Point(-x, -y) + center, color);
          pixelInternal(Point(x, -y) + center, color);
          ++y;
          yStopping += doubleSquareA;
          error += yChange;
          yChange += doubleSquareA;
          if (error > 0) {
            --x;
            xStopping -= doubleSquareB;
            error += xChange;
            xChange += doubleSquareB;
          }
        }

        xChange = 2 * dimension.getHeight() * dimension.getHeight();
        yChange = 2 * dimension.getWidth() * dimension.getWidth() * (1 - 2 * dimension.getHeight());
        error = yChange/2;
        xStopping = 0;
        yStopping = doubleSquareA * dimension.getHeight();
        x = 0;
        y = dimension.getHeight();

        while (xStopping <= yStopping) {
          pixelInternal(Point(x, y) + center, color);
          pixelInternal(Point(-x, y) + center, color);
          pixelInternal(Point(-x, -y) + center, color);
          pixelInternal(Point(x, -y) + center, color);
          ++x;
          xStopping += doubleSquareB;
          error += xChange;
          xChange += doubleSquareB;
          if (error > 0) {
            --y;
            yStopping -= doubleSquareA;
            error += yChange;
            yChange += doubleSquareA;
          }
        }

      }
    }
  }

  void Canvas::rectangle(const Point& p1, const Point& p2, Pixel color, unsigned int options) noexcept
  {
    if (options & FILL) {
      Point clipped1 = p1;
      Point clipped2 = p2;
      clip(clipped1, clipped2);

      Point first(minimum(clipped1.getX(), clipped2.getX()), minimum(clipped1.getY(), clipped2.getY()));
      Point last(maximum(clipped1.getX(), clipped2.getX()), maximum(clipped1.getY(), clipped2.getY()));

      ColorImage::Rows::RowIterator row = rows[first.getY()];
      const ColorImage::Rows::RowIterator endRow = rows[last.getY() + 1];
      while (row != endRow) {
        ColorImage::Rows::RowIterator::ElementIterator column = row[first.getX()];
        const ColorImage::Rows::RowIterator::ElementIterator endColumn = row[last.getX() + 1];
        while (column != endColumn) {
          *column = color;
          ++column;
        }
        ++row;
      }
    } else {
      line(Point(p1.getX(), p1.getY()), Point(p2.getX(), p1.getY()), color);
      line(Point(p1.getX(), p2.getY()), Point(p2.getX(), p2.getY()), color);
      line(Point(p1.getX(), p1.getY()), Point(p1.getX(), p2.getY()), color);
      line(Point(p2.getX(), p1.getY()), Point(p2.getX(), p2.getY()), color);
    }
  }

  void Canvas::pixel(const Point& point, Pixel color) noexcept
  {
    if (isPointInsideDimension(point, dimension)) {
      rows[point.getY()][point.getX()] = color;
    }
  }

  Canvas::Pixel Canvas::getPixel(const Point& point) const noexcept
  {
    Canvas::Pixel result;
    if (isPointInsideDimension(point, dimension)) {
      return rows[point.getY()][point.getX()];
    }
    result.rgb = 0;
    return result;
  }

  void Canvas::lineClipped(const Point& p1, const Point& p2, Pixel color) noexcept
  {
    // TAG: need initial error (when clipping)
    // the basic formula: y = dy/dx * (x - x1) + y1 where dy = y2-y1 and dx = x2-x1
    Point first = p1;
    Point last = p2;
    Point delta = last - first;

    if (absolute(delta.getX()) >= absolute(delta.getY())) {
      if (first.getX() > last.getX()) {
        first = p2;
        last = p1;
        delta = last - first; // dx >= 0
      }

      if (delta.getY() == 0) { // horizontal line
        ColorImage::Rows::RowIterator::ElementIterator column = rows[first.getY()][first.getX()];
        const ColorImage::Rows::RowIterator::ElementIterator endColumn = rows[first.getY()][last.getX() + 1];
        while (column != endColumn) {
          *column = color;
          ++column;
        }
        return;
      }

      Point doubleDelta(2 * delta.getX(), 2 * delta.getY());
      delta.setY(absolute(delta.getY()));
      doubleDelta.setY(absolute(doubleDelta.getY()));
      int incrementRow = doubleDelta.getY() - doubleDelta.getX();
      int numerator = doubleDelta.getY() - delta.getX(); // round to nearest

      ColorImage::Rows::RowIterator row = rows[first.getY()];
      ColorImage::Rows::RowIterator::ElementIterator column = row[first.getX()];
      unsigned int count = last.getX() - first.getX() + 1;
      if (last.getY() >= first.getY()) {
        while (count--) {
          *column = color;
          ++column;
          if (numerator >= 0) {
            numerator += incrementRow;
            ColorImage::Rows::RowIterator::ElementIterator firstColumn = row[0];
            ++row;
            column = row[column - firstColumn];
          } else {
            numerator += doubleDelta.getY();
          }
        }
      } else {
        while (count--) {
          *column = color;
          ++column;
          if (numerator >= 0) {
            numerator += incrementRow;
            ColorImage::Rows::RowIterator::ElementIterator firstColumn = row[0];
            --row;
            column = row[column - firstColumn];
          } else {
            numerator += doubleDelta.getY();
          }
        }
      }
    } else { // |dy| > |dx|

      if (p1.getY() <= p2.getY()) {
        first = p1;
        last = p2;
      } else {
        first = p2;
        last = p1;
      }
      delta = last - first; // dy >= 0

      if (delta.getX() == 0) { // vertical line
        ColorImage::Rows::RowIterator row = rows[first.getY()];
        const ColorImage::Rows::RowIterator endRow = rows[last.getY() + 1];
        while (row != endRow) {
          row[first.getX()] = color;
          ++row;
        }
        return;
      }

      Point doubleDelta(2 * delta.getX(), 2 * delta.getY());
      delta.setX(absolute(delta.getX()));
      doubleDelta.setX(absolute(doubleDelta.getX()));
      int incrementColumn = doubleDelta.getX() - doubleDelta.getY();
      int numerator = doubleDelta.getX() - delta.getY(); // round to nearest

      ColorImage::Rows::RowIterator row = rows[first.getY()];
      const ColorImage::Rows::RowIterator endRow = rows[last.getY() + 1];
      unsigned int x = first.getX();
      if (last.getX() >= first.getX()) {
        while (row != endRow) {
          row[x] = color;
          ++row;
          if (numerator >= 0) {
            numerator += incrementColumn;
            ++x;
          } else {
            numerator += doubleDelta.getX();
          }
        }
      } else {
        while (row != endRow) {
          row[x] = color;
          ++row;
          if (numerator >= 0) {
            numerator += incrementColumn;
            --x;
          } else {
            numerator += doubleDelta.getX();
          }
        }
      }

    }
  }

  void Canvas::lineClippedAntiAliased(const Point& p1, const Point& p2, Pixel color) noexcept
  {
    // TAG: need initial error (when clipping)
    // the basic formula: y = dy/dx * (x - x1) + y1 where dy = y2-y1 and dx = x2-x1
    Point first = p1;
    Point last = p2;
    Point delta = last - first;

    if (absolute(delta.getX()) >= absolute(delta.getY())) {
      if (first.getX() > last.getX()) {
        first = p2;
        last = p1;
        delta = last - first; // dx >= 0
      }

      if (delta.getY() == 0) { // horizontal line
        ColorImage::Rows::RowIterator::ElementIterator column = rows[first.getY()][first.getX()];
        const ColorImage::Rows::RowIterator::ElementIterator endColumn = rows[first.getY()][last.getX() + 1];
        while (column != endColumn) {
          *column = color;
          ++column;
        }
        return;
      }

      delta.setY(absolute(delta.getY()));
      int incrementRow = delta.getX() - delta.getY(); // >= 0
      int numerator = 0; // round down

      ColorImage::Rows::RowIterator row = rows[first.getY()];
      ColorImage::Rows::RowIterator::ElementIterator column = row[first.getX()];
      unsigned int count = last.getX() - first.getX() + 1;
      if (last.getY() >= first.getY()) {
        ColorImage::Rows::RowIterator nextRow = rows[first.getY() + 1];
        ColorImage::Rows::RowIterator::ElementIterator nextColumn = nextRow[first.getX()];
        while (count--) {
          *column = blend(*column, color, delta.getX(), delta.getX() - numerator);
          ++column;
          *nextColumn = blend(*nextColumn, color, delta.getX(), numerator);
          ++nextColumn;
          if (numerator >= incrementRow) {
            numerator -= incrementRow;
            row = nextRow;
            column = nextColumn;
            ColorImage::Rows::RowIterator::ElementIterator firstColumn = nextRow[0];
            ++nextRow;
            nextColumn = nextRow[nextColumn - firstColumn];
          } else {
            numerator += delta.getY();
          }
        }
      } else {
        ColorImage::Rows::RowIterator previousRow = rows[first.getY() - 1];
        ColorImage::Rows::RowIterator::ElementIterator previousColumn = previousRow[first.getX()];
        while (count--) {
          *column = blend(*column, color, delta.getX(), delta.getX() - numerator);
          ++column;
          *previousColumn = blend(*previousColumn, color, delta.getX(), numerator);
          ++previousColumn;
          if (numerator >= incrementRow) {
            numerator -= incrementRow;
            row = previousRow;
            column = previousColumn;
            ColorImage::Rows::RowIterator::ElementIterator firstColumn = previousRow[0];
            --previousRow;
            previousColumn = previousRow[previousColumn - firstColumn];
          } else {
            numerator += delta.getY();
          }
        }
      }
    } else { // |dy| > |dx|

      if (p1.getY() <= p2.getY()) {
        first = p1;
        last = p2;
      } else {
        first = p2;
        last = p1;
      }
      delta = last - first; // dy >= 0

      if (delta.getX() == 0) { // vertical line
        ColorImage::Rows::RowIterator row = rows[first.getY()];
        const ColorImage::Rows::RowIterator endRow = rows[last.getY() + 1];
        while (row != endRow) {
          row[first.getX()] = color;
          ++row;
        }
        return;
      }

      delta.setX(absolute(delta.getX()));
      int incrementColumn = delta.getY() - delta.getX(); // >= 0
      int numerator = 0; // round down

      ColorImage::Rows::RowIterator row = rows[first.getY()];
      int x = first.getX();
      unsigned int count = last.getY() - first.getY() + 1; // use end iterator?
      if (last.getX() >= first.getX()) {
        while (count--) {
          ColorImage::Rows::RowIterator::ElementIterator column = row[x];
          *column = blend(*column, color, delta.getY(), delta.getY() - numerator);
          ++column; // next column
          *column = blend(*column, color, delta.getY(), numerator);
          ++row; // next row
          if (numerator >= incrementColumn) {
            numerator -= incrementColumn;
            ++x; // next column
          } else {
            numerator += delta.getX();
          }
        }
      } else {
        while (count--) {
          ColorImage::Rows::RowIterator::ElementIterator column = row[x];
          *column = blend(*column, color, delta.getY(), delta.getY() - numerator);
          --column; // previous column
          *column = blend(*column, color, delta.getY(), numerator);
          ++row; // next row
          if (numerator >= incrementColumn) {
            numerator -= incrementColumn;
            --x; // previous column
          } else {
            numerator += delta.getX();
          }
        }
      }
    }
  }

  void Canvas::line(const Point& p1, const Point& p2, Pixel color, unsigned int options) noexcept
  {
    Point clipped1 = p1;
    Point clipped2 = p2;
    if (!clip(clipped1, clipped2)) {
      return; // nothing to do
    }
    if (options & ANTIALIASING) {
      lineClippedAntiAliased(clipped1, clipped2, color);
    } else {
      lineClipped(clipped1, clipped2, color);
    }
  }

  void Canvas::line(const Point& p1, const Point& p2, ColorAlphaPixel color, unsigned int options) noexcept
  {
    Point clipped1 = p1;
    Point clipped2 = p2;
    if (!clip(clipped1, clipped2)) {
      return; // nothing to do
    }
    if (options & ANTIALIASING) {
//      lineClippedAntiAliased(clipped1, clipped2, color);
    } else {
//      lineClipped(clipped1, clipped2, color);
    }
  }

  void Canvas::setFont(const String& name) noexcept {
    fontName = name;
  }

  String Canvas::getFont() const noexcept {
    return fontName;
  }

  Dimension Canvas::getDimensionOfText(const String& message) const noexcept
  {
    return Dimension(message.getLength() * 8, 16);
  }

  // width, height, bytesPerLine, bytesPerCharacter

  void Canvas::write(const Point& position, const String& message) noexcept
  {
    const unsigned char* fontBitmap = nullptr; // fill me
    // const unsigned int bytesPerRow = 1;
    const unsigned int bytesPerCharacter = 16;
    const unsigned int characterWidth = 8;
    const unsigned int characterHeight = 16;

    int x = position.getX();
    String::ReadEnumerator enu = message.getReadEnumerator();
    while (enu.hasNext()) {
      const unsigned char* characterBitmap = fontBitmap + *enu.next() * bytesPerCharacter;
      for (unsigned int row = characterHeight; row > 0; --row) {
        unsigned int rowBitmap = *characterBitmap;
        for (unsigned int column = characterWidth; column > 0; --column) {
          if (rowBitmap & 1) {
            pixel(Point(x + column, position.getY() - row), makeColorPixel(127, 255, 63));
          }
          rowBitmap >>= 1;
        }
        x += characterWidth;
      }
    }
  }

}; // end of gip namespace
