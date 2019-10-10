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

#include <base/Object.h>
#include <base/Dimension.h>
#include <base/OutOfRange.h>
#include <gip/Point2D.h>

namespace gip {

  /**
    Class responsible for mapping coordinates (spanned by a specified dimension)
    into one-dimensional indices in a consistent manner. Offen all the
    coordinates have a corresponding index but this is not required.

    @short Mapper of coordinates into one-dimensional indices.
    @version 1.0
  */

  class _COM_AZURE_DEV__BASE__API CoordinateMapper : public Object {
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

  inline CoordinateMapper::CoordinateMapper(const Dimension& _dimension) throw()
    : dimension(_dimension) {
  }

}; // end of gip namespace
