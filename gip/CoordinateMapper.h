/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__COORDINATE_MAPPER_H
#define _DK_SDU_MIP__GIP__COORDINATE_MAPPER_H

#include <base/Object.h>
#include <base/Dimension.h>
#include <base/OutOfRange.h>
#include <gip/Point2D.h>

using namespace base;

namespace gip {

/**
  Class responsible for mapping coordinates (spanned by a specified dimension)
  into one-dimensional indices in a consistent manner. Offen all the
  coordinates have a corresponding index but this is not required.

  @short Mapper of coordinates into one-dimensional indices.
  @author René Møller Fonseca
*/

class CoordinateMapper : public Object {
protected:

  /** The dimension to be enumerated. */
  const Dimension dimension;
public:

  /**
    Initializes the coordinate mapper.

    @param dimension The dimension to be enumerated.
  */
  CoordinateMapper(const Dimension& dimension) throw();
  /**
    Returns the total number of valid indices.
  */
  virtual unsigned int getSize() const throw() = 0;

  /**
    Translates the specified index into a coordinate.
  */
  virtual Point2D translate(unsigned int index) throw(OutOfRange) = 0;

  /**
    Translates the specified coordinate into an index.
  */
  virtual unsigned int translate(const Point2D& coordinate) throw(OutOfRange) = 0;
};

inline CoordinateMapper::CoordinateMapper(const Dimension& d) throw() : dimension(d) {}

}; // end of namespace

#endif
