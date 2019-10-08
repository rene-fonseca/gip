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
#include <base/Dimension.h>

namespace gip {

  /**
    This class is normally used to specify an element within an image.

    @short Discrete two-dimensional point.
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  class Point2D {
  private:

    /** Specifies the Y coordinate. */
    unsigned int row;
    /** Specifies the X coordinate. */
    unsigned int column;
  public:

    /**
      Initializes point as origin (0, 0).
    */
    inline Point2D() throw() : row(0), column(0) {
    }

    /**
      Initializes point.
      
      @param row The desired row of the point.
      @param column The desired column of the point.
    */
    Point2D(unsigned int row, unsigned int column) throw();

    /**
      Initializes point from other point.
    */
    inline Point2D(const Point2D& copy) throw()
      : row(copy.row), column(copy.column) {
    }
    
    /**
      Assignment of point by point.
    */
    inline Point2D& operator=(const Point2D& eq) throw() {
      row = eq.row; // no need to protect against self-assignment
      column = eq.column;
      return *this;
    }

    /**
      Returns true if the point is contained within the specified dimension.
    */
    inline bool isWithin(const Dimension& dimension) const throw() {
      return (column < dimension.getWidth()) && (row < dimension.getHeight());
    }

    /**
      Returns the column of the point.
    */
    inline unsigned int getColumn() const throw() {
      return column;
    }

    /**
      Returns the row of the point.
    */
    inline unsigned int getRow() const throw() {
      return row;
    }

    /**
      Sets the column of the point.
    */
    inline void setColumn(unsigned int column) throw() {
      this->column = column;
    }

    /**
      Sets the row of the point.
    */
    inline void setRow(unsigned int row) throw() {
      this->row = row;
    }
  };

  inline Point2D::Point2D(unsigned int _row, unsigned int _column) throw()
    : row(_row), column(_column) {
  }

  /**
    Writes the specified point to the stream (e.g. "(row, column)").
  */
  FormatOutputStream& operator<<(FormatOutputStream& stream, const Point2D& value) throw(IOException);

}; // end of gip namespace

namespace base {
  
  template<>
  class Relocateable<gip::Point2D> {
  public:
    static const bool IS_RELOCATEABLE = Relocateable<unsigned int>::IS_RELOCATEABLE;
  };

}; // end of base namespace
