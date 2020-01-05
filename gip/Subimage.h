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

#include <gip/ShadowImage.h>
#include <gip/Point2D.h>
#include <gip/Region.h>

namespace gip {

/**
  This class associates an offset with an image.

  @short Image that is part of a greater context.
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
  Subimage(Image<PIXEL>* image, const Point2D& offset) noexcept;

  /**
    Returns the offset of the image.
  */
  inline const Point2D& getOffset() const noexcept {
    return offset;
  }

  /**
    Returns the region associated with the image.
  */
  Region getRegion() const noexcept {
    return Region(offset, ShadowImage<PIXEL>::getDimension());
  }
};

template<class PIXEL>
inline Subimage<PIXEL>::Subimage(Image<PIXEL>* image, const Point2D& _offset) noexcept :
  ShadowImage<PIXEL>(image), offset(_offset) {
}

}; // end of gip namespace
