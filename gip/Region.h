/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Ren� M�ller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__REGION_H
#define _DK_SDU_MIP__GIP__REGION_H

#include <gip/Point2D.h>
#include <base/Dimension.h>

using namespace base;

namespace gip {

/**
  A region of an image.

  @short A region of an image.
  @author Ren� M�ller Fonseca
*/

class Region {
private:

  /** The offset of the region. */
  Point2D offset;
  /** The dimension of the region. */
  Dimension dimension;
public:

  /**
    Initializes region with offset (0, 0) and dimension (0, 0).
  */
  inline Region() throw() : offset(0, 0), dimension(0, 0) {}

  /**
    Initializes the region with the specified offset and dimension.

    @param offset The desired offset of the region.
    @param dimension The desired dimension of the region.
  */
  Region(const Point2D& offset, const Dimension& dimension) throw();

  /**
    Initializes region from other region.
  */
  inline Region(const Region& copy) throw() : offset(copy.offset), dimension(copy.dimension) {}

  /**
    Assignment of region by region.
  */
  inline Region& operator=(const Region& eq) throw() {
    offset = eq.offset;
    dimension = eq.dimension;
    return *this;
  }

  /**
    Returns the offset of the region.
  */
  inline const Point2D& getOffset() const throw() {return offset;}

  /**
    Returns the dimension of the region.
  */
  inline const Dimension& getDimension() const throw() {return dimension;}

  /**
    Sets the offset of the region.
  */
  inline void setOffset(const Point2D& offset) throw() {
    this->offset = offset;
  }

  /**
    Sets the dimension of the region.
  */
  inline void setDimension(const Dimension& dimension) throw() {
    this->dimension = dimension;
  }

  /**
    Returns true if the region spans elements (i.e. the dimension is proper).
  */
  inline bool isProper() const throw() {return dimension.isProper();}
};

inline Region::Region(const Point2D& o, const Dimension& d) throw() : offset(o), dimension(d) {}

template<> inline bool isRelocateable<Region>() throw() {return isRelocateable<Point2D>() && isRelocateable<Dimension>();}

}; // end of namespace

#endif