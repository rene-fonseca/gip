/***************************************************************************
    begin                : Mon Apr 23 2001
    copyright            : (C) 2001 by Ren� M�ller Fonseca
    email                : fonseca@mip.sdu.dk
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__IMAGE_H
#define _DK_SDU_MIP__GIP__IMAGE_H

#include <base/Object.h>
#include <base/Dimension.h>
#include <gip/Pixel.h>

using namespace base;

namespace gip {

/**
  Base class of all images.

  @short Image
  @author Ren� M�ller Fonseca
*/

template<class PIXEL>
class Image : public Object {
public:

  /** The type of the image element (pixel). */
  typedef PIXEL Pixel;
private:

  /** The dimension (width and height) of the image. */
  Dimension dimension;
  /** Prevent default assignment. */
  Image& operator=(const Image& eq) throw();
public:

  /**
    Initializes image with the specified dimension.

    @param dimension The dimension of the image.
  */
  Image(const Dimension& dimension) throw();

  /**
    Initializes image from other image.
  */
  inline Image(const Image& copy) throw() : dimension(copy.dimension) {}

  /**
    Returns the dimension of the image.
  */
  const Dimension& getDimension() const throw();

  /**
    Returns the height (i.e. the number of rows) of the image.
  */
  unsigned int getHeight() const throw();

  /**
    Returns the width (i.e. the number of columns) of the image.
  */
  unsigned int getWidth() const throw();
};

template<class PIXEL>
inline Image<PIXEL>::Image(const Dimension& d) throw() : dimension(d) {}

template<class PIXEL>
inline const Dimension& Image<PIXEL>::getDimension() const throw() {return dimension;}

template<class PIXEL>
inline unsigned int Image<PIXEL>::getHeight() const throw() {return dimension.getHeight();}

template<class PIXEL>
inline unsigned int Image<PIXEL>::getWidth() const throw() {return dimension.getWidth();}

}; // end of namespace

#endif
