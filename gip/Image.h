/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__IMAGE_H
#define _DK_SDU_MIP__GIP__IMAGE_H

#include <base/Object.h>
#include <base/Dimension.h>
#include <gip/Pixel.h>

namespace gip {

  /**
    @defgroup images Images
  */

  /**
    Base class of all images.

    @short Image
    @ingroup images
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class PIXEL>
  class Image : public Object {
  public:

    /** The type of the image element (pixel). */
    typedef PIXEL Pixel;
  private:

    /** The dimension (width and height) of the image. */
    Dimension dimension;
  public:

    /**
      Initializes image.
    */
    inline Image() throw() {
    }
    
    /**
      Initializes image with the specified dimension.

      @param dimension The dimension of the image.
    */
    Image(const Dimension& dimension) throw();

    /**
      Initializes image from other image.
    */
    inline Image(const Image& copy) throw() : dimension(copy.dimension) {
    }

    Image& operator=(const Image& eq) throw() {
      dimension = eq.dimension;
      return *this;
    }
    
    /**
      Returns the dimension of the image.
    */
    inline const Dimension& getDimension() const throw() {
      return dimension;
    }

    /**
      Returns the height (i.e. the number of rows) of the image.
    */
    inline unsigned int getHeight() const throw() {
      return dimension.getHeight();
    }

    /**
      Returns the width (i.e. the number of columns) of the image.
    */
    inline unsigned int getWidth() const throw() {
      return dimension.getWidth();
    }
  };

  template<class PIXEL>
  inline Image<PIXEL>::Image(const Dimension& _dimension) throw() : dimension(_dimension) {
  }

}; // end of gip namespace

#endif
