/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/draw/DrawLine.h>

namespace gip {

  Canvas::Canvas(ColorImage* _canvas) throw()
     : canvas(_canvas),
       dimension(_canvas->getDimension()),
       rows(_canvas->getRows()) {
  }

  void Canvas::clip(Point& p1, Point& p2) const throw() {
    // Cohen-Sutherland line clipping
    enum {BOTTOM = 1, TOP = 2, LEFT = 4, RIGTH = 8};

    Point delta = p2 - p1;
    Point clipped1 = p1;
    Point clipped2 = p2;

    unsigned int clip1 = 0;
    if (clipped1.getX() < 0) {
      clip1 |= LEFT;
    } else if (clipped1.getX() >= dimension.getWidth()) {
      clip1 |= RIGHT;
    }
    if (clipped1.getY() < 0) {
      clip1 |= BOTTOM;
    } else if (clipped1.getY() >= dimension.getHeight()) {
      clip1 |= TOP;
    }

    unsigned int clip2 = 0;
    if (clipped2.getX() < 0) {
      clip2 |= LEFT;
    } else if (clipped2.getX() >= dimension.getWidth()) {
      clip2 |= RIGHT;
    }
    if (clipped2.getY() < 0) {
      clip2 |= BOTTOM;
    } else if (clipped2.getY() >= dimension.getHeight()) {
      clip2 |= TOP;
    }

    while ((clip1 | clip2) != 0) { // continue until trivial accept
      if ((clip1 & clip2) != 0) { // reject line 'cause completely on one side
        return;
      }
      if ((clip1 & (TOP | BOTTOM)) != 0) {
        int y = ((clip1 & BOTTOM) != 0) ? 0 : (dimension.getHeight() - 1);
        clipped1 = Point(delta.getX() * (y - p1.getY())/delta.getY() + p1.getX(), y);
        if (clipped1.getX() < 0) {
          clip1 = LEFT;
        } else if (clipped1.getX() >= dimension.getWidth()) {
          clip1 = RIGHT;
        } else {
          clip1 = 0;
        }
      } else if ((clip1 & (LEFT | RIGHT)) != 0) {
        int x = ((clip1 & LEFT) != 0) ? 0 : (dimension.getWidth() - 1);
        clipped1 = Point(x, delta.getY() * (x - p1.getX())/delta.getX() + p1.getY());
        if (clipped1.getY() < 0) {
          clip1 = BOTTOM;
        } else if (clipped1.getY() >= dimension.getHeight()) {
          clip1 = TOP;
        } else {
          clip1 = 0;
        }
      } else if ((clip2 & (TOP | BOTTOM)) != 0) {
        int y = ((clip2 & BOTTOM) != 0) ? 0 : (dimension.getHeight() - 1);
        clipped2 = Point(delta.getX() * (y - p1.getY())/delta.getY() + p1.getX(), y);
        if (clipped2.getX() < 0) {
          clip2 = LEFT;
        } else if (clipped2.getX() >= dimension.getWidth()) {
          clip2 = RIGHT;
        } else {
          clip2 = 0;
        }
      } else { // if ((clip2 & (LEFT | RIGHT)) != 0) { // LEFT or RIGHT must be set
        int x = ((clip2 & LEFT) != 0) ? 0 : (dimension.getWidth() - 1);
        clipped2 = Point(x, delta.getY() * (x - p1.getX())/delta.getX() + p1.getY());
        if (clipped2.getY() < 0) {
          clip2 = BOTTOM;
        } else if (clipped2.getY() >= dimension.getHeight()) {
          clip2 = TOP;
        } else {
          clip2 = 0;
        }
      }
    }
    p1 = clipped1;
    p2 = clipped2;

    ASSERT((clipped1.getX() >= 0) && (clipped1.getX() < dimension.getWidth()));
    ASSERT((clipped1.getY() >= 0) && (clipped1.getY() < dimension.getHeight()));
    ASSERT((clipped2.getX() >= 0) && (clipped2.getX() < dimension.getWidth()));
    ASSERT((clipped2.getY() >= 0) && (clipped2.getY() < dimension.getHeight()));
  }

  void Canvas::image(const Point& offset, const ColorImage& image) throw() {
  }

  void Canvas::circle(const Point& center, unsigned int radius, Pixel color) throw() {
     int d = 3 - (2 * radius);
     int x = 0;
     int y = radius;

     while (x < y) {
       // draw 8 pixels
//     pixel(center.getX() + x, center.getY() + y);
//     pixel(center.getX() + x , center.getY() - y);
//     pixel(center.getX() - x, center.getY() + y);
//     pixel(center.getX() - x, center.getY() - y);
//     pixel(center.getX() + y, center.getY() + x);
//     pixel(center.getX() + y, center.getY() - x);
//     pixel(center.getX() - y, center.getY() + x);
//     pixel(center.getX() - y, center.getY() - x);
       if (d < 0) {
         d += 4 * x + 6;
       } else {
         d += 4 * (x - y) + 10;
         --y;
       }
       ++x;
     }
     if (x == y) {
       // draw pixels
     }
  }

  void Canvas::ellipse(const Point& center, int a, int b, Pixel color) throw() {
    int x = 0;
    int y = b;
    int a2 = a * a;
    int b2 = b * b;
    int S = a2 * (1 - 2 * b) + 2 * b2;
    int T = b2 - 2 * a2 * (2 * b - 1);
//    pixel(x, y);
//    pixel(-x, y);
//    pixel(-x, -y);
//    pixel(x, -y);
    do {
      if (S<0) {
        S += 2 * b2 * (2 * x + 3);
        T += 4 * b2 * (x + 1);
        ++x;
      } else if (T < 0) {
        S += 2 * b2 * (2 * x + 3) - 4 * a2 * (y - 1);
        T += 4 * b2 * (x + 1) - 2 * a2 * (2 * y - 3);
        ++x;
        --y;
      } else {
        S -= 4 * a2 * (y - 1);
        T -= 2 * a2 * (2 * y - 3);
        --y;
      }
//        pixel(x, y);
//        pixel(-x, y);
//        pixel(-x, -y);
//        pixel(x, -y);
    } while (y > 0);
  }

  void Canvas::fill(const Point& p1, const Point& p2, ColorImage::Pixel color) throw() {
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
  }

  void Canvas::rectangle(const Point& p1, const Point& p2, Pixel color) throw() {
    line(Point(p1.getX(), p1.getY()), Point(p2.getX(), p1.getY()), color);
    line(Point(p1.getX(), p2.getY()), Point(p2.getX(), p2.getY()), color);
    line(Point(p1.getX(), p1.getY()), Point(p1.getX(), p2.getY()), color);
    line(Point(p2.getX(), p1.getY()), Point(p2.getX(), p2.getY()), color);
  }

  void Canvas::pixel(const Point& point, ColorImage::Pixel color) throw() {
    if ((point.getX() >= 0) && (point.getY() >= 0) && (point.getX() < dimension.getWidth()) && (point.getY() < dimension.getHeight())) {
      rows[point.getY()][point.getX()] = color;
    }
  }

  void Canvas::lineClipped(const Point& p1, const Point& p2, ColorPixel color) throw() {
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

  void Canvas::line(const Point& p1, const Point& p2, ColorPixel color) throw() {
    Point clipped1 = p1;
    Point clipped2 = p2;
    clip(clipped1, clipped2);
    lineClipped(clipped1, clipped2, color);
  }

}; // end of gip namespace
