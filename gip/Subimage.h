/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__SUBIMAGE_H
#define _DK_SDU_MIP__GIP__SUBIMAGE_H

#include <gip/ShadowImage.h>
#include <gip/Point2D.h>
#include <gip/Region.h>

namespace gip {

/**
  This class associates an offset with an image.

  @short Image that is part of a greater context.
  @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
  @version 1.0
*/

template<class PIXEL>
class Subimage : public ShadowImage<PIXEL> {
protected:

  /** The offset of this image within another image. */
  const Point2D offset;
public:

  /**
    Initializes subimage with the specified offset.

    @param image The image.
    @param offset The offset to associate with the image.
  */
  Subimage(Image<PIXEL>* image, const Point2D& offset) throw();

  /**
    Returns the offset of the image.
  */
  const Point2D& getOffset() const throw();

  /**
    Returns the region associated with the image.
  */
  Region getRegion() const throw();
};

template<class PIXEL>
inline Subimage<PIXEL>::Subimage(Image<PIXEL>* image, const Point2D& o) throw() :
  ShadowImage<PIXEL>(image), offset(o) {
}

template<class PIXEL>
inline const Point2D& Subimage<PIXEL>::getOffset() const throw() {return offset;}

template<class PIXEL>
inline Region Subimage<PIXEL>::getRegion() const throw() {
  return Region(offset, getDimension());
}

}; // end of namespace

#endif
