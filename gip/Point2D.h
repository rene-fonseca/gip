/***************************************************************************
    begin                : Thu Apr 26 2001
    copyright            : (C) 2001 by René Møller Fonseca
    email                : fonseca@mip.sdu.dk
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__POINT_2D_H
#define _DK_SDU_MIP__GIP__POINT_2D_H

#include <base/Dimension.h>

using namespace base;

namespace gip {

/**
  This class is normally used to specify an element within an image.

  @short Discrete two-dimensional point.
  @author René Møller Fonseca
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
  Point2D() throw();

  /**
    Initializes point.

    @param row The desired row of the point.
    @param column The desired column of the point.
  */
  Point2D(unsigned int row, unsigned int column) throw();

  /**
    Initializes point from other point.
  */
  Point2D(const Point2D& copy) throw();

  /**
    Assignment of point by point.
  */
  Point2D& operator=(const Point2D& eq) throw();

  /**
    Returns true if the point is contained within the specified dimension.
  */
  bool isWithin(const Dimension& dimension) const throw();

  /**
    Returns the column of the point.
  */
  unsigned int getColumn() const throw();

  /**
    Returns the row of the point.
  */
  unsigned int getRow() const throw();

  /**
    Sets the column of the point.
  */
  void setColumn(unsigned int column) throw();

  /**
    Sets the row of the point.
  */
  void setRow(unsigned int row) throw();
};

inline Point2D::Point2D() throw() : row(0), column(0) {}

inline Point2D::Point2D(unsigned int r, unsigned int c) throw() : row(r), column(c) {}

inline Point2D::Point2D(const Point2D& copy) throw() : row(copy.row), column(copy.column) {}

inline Point2D& Point2D::operator=(const Point2D& eq) throw() {
  // no need to protect against self-assignment
  row = eq.row;
  column = eq.column;
  return *this;
}

inline bool Point2D::isWithin(const Dimension& dimension) const throw() {
  return (column < dimension.getWidth()) && (row < dimension.getHeight());
}

inline unsigned int Point2D::getColumn() const throw() {return column;}

inline unsigned int Point2D::getRow() const throw() {return row;}

inline void Point2D::setColumn(unsigned int column) throw() {this->column = column;}

inline void Point2D::setRow(unsigned int row) throw() {this->row = row;}

template<> inline bool isRelocateable<Point2D>() throw() {return isRelocateable<unsigned int>();}

}; // end of namespace

#endif
