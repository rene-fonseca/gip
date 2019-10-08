/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/features.h>
#include <base/string/FormatOutputStream.h>

namespace gip {

  /**
    This class is normally used to specify an element within an image.

    @short Discrete two-dimensional point.
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  class Point {
  private:

    /** Specifies the Y coordinate. */
    int x;
    /** Specifies the X coordinate. */
    int y;
  public:

    /**
      Initializes point as origin (0, 0).
    */
    inline Point() throw() : x(0), y(0) {
    }

    /**
      Initializes point.
      
      @param x The desired x coordinate.
      @param y The desired y coordinate.
    */
    Point(int x, int y) throw();

    /**
      Initializes point from other point.
    */
    inline Point(const Point& copy) throw() : x(copy.x), y(copy.y) {
    }
    
    /**
      Assignment of point by point.
    */
    inline Point& operator=(const Point& eq) throw() {
      x = eq.x; // no need to protect against self-assignment
      y = eq.y;
      return *this;
    }

    /**
      Returns the x coordinate of the point.
    */
    inline int getX() const throw() {
      return x;
    }

    /**
      Returns the y coordinate of the point.
    */
    inline int getY() const throw() {
      return y;
    }

    /**
      Sets the x coordinate.
    */
    inline void setX(int x) throw() {
      this->x = x;
    }

    /**
      Sets the y coordinate.
    */
    inline void setY(int y) throw() {
      this->y = y;
    }
  };

  inline Point::Point(int _x, int _y) throw() : x(_x), y(_y) {
  }
  
  inline Point operator+(const Point& left, const Point& right) throw() {
    return Point(left.getX() + right.getX(), left.getY() + right.getY());
  }
  
  inline Point operator-(const Point& left, const Point& right) throw() {
    return Point(left.getX() - right.getX(), left.getY() - right.getY());
  }

  /**
    Writes the specified point to the stream (e.g. "(x,y)").
  */
  FormatOutputStream& operator<<(FormatOutputStream& stream, const Point& value) throw(IOException);

}; // end of gip namespace

namespace base {
  
  template<>
  class Relocateable<gip::Point> {
  public:
    static const bool IS_RELOCATEABLE = Relocateable<int>::IS_RELOCATEABLE;
  };

}; // end of base namespace
