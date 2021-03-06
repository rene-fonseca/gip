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
    @version 1.0
  */

  class _COM_AZURE_DEV__GIP__API Point {
  private:

    /** Specifies the Y coordinate. */
    int x = 0;
    /** Specifies the X coordinate. */
    int y = 0;
  public:

    /**
      Initializes point as origin (0, 0).
    */
    inline Point() noexcept {
    }

    /**
      Initializes point.
      
      @param x The desired x coordinate.
      @param y The desired y coordinate.
    */
    inline Point(int _x, int _y) noexcept : x(_x), y(_y) {
    }

    /**
      Initializes point from other point.
    */
    inline Point(const Point& copy) noexcept : x(copy.x), y(copy.y) {
    }
    
    /**
      Assignment of point by point.
    */
    inline Point& operator=(const Point& eq) noexcept {
      x = eq.x; // no need to protect against self-assignment
      y = eq.y;
      return *this;
    }

    /**
      Returns the x coordinate of the point.
    */
    inline int getX() const noexcept {
      return x;
    }

    /**
      Returns the y coordinate of the point.
    */
    inline int getY() const noexcept {
      return y;
    }

    /**
      Sets the x coordinate.
    */
    inline void setX(int x) noexcept {
      this->x = x;
    }

    /**
      Sets the y coordinate.
    */
    inline void setY(int y) noexcept {
      this->y = y;
    }
  };
  
  inline Point operator+(const Point& left, const Point& right) noexcept {
    return Point(left.getX() + right.getX(), left.getY() + right.getY());
  }
  
  inline Point operator-(const Point& left, const Point& right) noexcept {
    return Point(left.getX() - right.getX(), left.getY() - right.getY());
  }

  /**
    Writes the specified point to the stream (e.g. "(x,y)").
  */
  _COM_AZURE_DEV__GIP__API FormatOutputStream& operator<<(FormatOutputStream& stream, const Point& value);

}; // end of gip namespace

namespace base {
  
  template<>
  class Relocateable<gip::Point> {
  public:
    static const bool IS_RELOCATEABLE = Relocateable<int>::IS_RELOCATEABLE;
  };

}; // end of base namespace
