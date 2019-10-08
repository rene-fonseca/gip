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
#include <gip/Pixel.h>
#include <gip/ImageException.h>

namespace gip {

  /**
    @defgroup images Images
  */

  /**
    Base class of all images.

    @short Image
    @ingroup images
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
    Image(const Dimension& dimension) throw(ImageException);

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
  inline Image<PIXEL>::Image(const Dimension& _dimension) throw(ImageException) : dimension(_dimension) {
    bassert(
      (dimension.getWidth() <= 0xffff) && (dimension.getHeight() <= 0xffff),
      ImageException("Image dimension limit exceeded", this)
    );
  }

}; // end of gip namespace
