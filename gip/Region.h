/***************************************************************************
    begin                : Thu Apr 26 2001
    copyright            : (C) 2001 by René Møller Fonseca
    email                : fonseca@mip.sdu.dk
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
  @author René Møller Fonseca
*/

class Region {
private:

  /** The offset of the region. */
  Point2D offset;
  /** The dimension of the region. */
  Dimension dimension;
public:

  /**
    Initializes region.
  */
  inline Region() throw() {}

  /**
    Initializes the region with the specified value.

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
    Returns true if the region spans elements.
  */
  inline bool isProper() const throw() {return dimension.isProper();}
};

inline Region::Region(const Point2D& o, const Dimension& d) throw() : offset(o), dimension(d) {}

template<> inline bool isRelocateable<Region>() throw() {return isRelocateable<Point2D>() && isRelocateable<Dimension>();}

}; // end of namespace

#endif
