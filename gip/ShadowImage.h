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

#include <gip/Image.h>

namespace gip {

/**
  An image that maps onto another image (similar to FilterInputStream and FilterOutputStream).

  @short Shadow image.
  @version 1.0
*/

template<class PIXEL>
class ShadowImage : public Image<PIXEL> {
protected:

  /** The image to work on. */
  Image<PIXEL>* const image = nullptr;
public:

  /**
    Initializes the shadow image.

    @param image The image.
  */
  ShadowImage(Image<PIXEL>* i) noexcept
    : Image<PIXEL>(i->getDimension()), image(i) {
  }

  /**
    Returns the dimension of the image.
  */
  const Dimension& getDimension() const noexcept;
};

template<class PIXEL>
inline const Dimension& ShadowImage<PIXEL>::getDimension() const noexcept {
  return image->getDimension();
}

}; // end of gip namespace
