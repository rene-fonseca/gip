/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__SHADOW_IMAGE_H
#define _DK_SDU_MIP__GIP__SHADOW_IMAGE_H

#include <gip/Image.h>

namespace gip {

/**
  An image that maps onto another image (similar to FilterInputStream and FilterOutputStream).

  @short Shadow image.
  @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
  @version 1.0
*/

template<class PIXEL>
class ShadowImage : public Image<PIXEL> {
protected:

  /** The image to work on. */
  Image<PIXEL>* const image;
public:

  /**
    Initializes the shadow image.

    @param image The image.
  */
  ShadowImage(Image<PIXEL>* i) throw() : Image<PIXEL>(i->getDimension()), image(i) {}

  /**
    Returns the dimension of the image.
  */
  const Dimension& getDimension() const throw();
};

template<class PIXEL>
inline const Dimension& ShadowImage<PIXEL>::getDimension() const throw() {
  return image->getDimension();
}

}; // end of gip namespace

#endif
