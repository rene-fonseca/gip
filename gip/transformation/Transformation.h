/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__TRANSFORMATION_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__TRANSFORMATION_H

#include <base/Object.h>
#include <gip/ArrayImage.h>

namespace gip {

  /**
    @defgroup transformations Transformations
  */

  /**
    Transformation.

    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  
  template<class DEST, class SRC>
  class Transformation : public Object {
  public:

    typedef DEST DestinationImage;
    typedef SRC SourceImage;
  protected:

    /** The destination image of the transformation. */
    DestinationImage* const destination;
    /** The source image of the transformation. */
    const SourceImage* const source;
  public:

    /**
      Initializes transformation object.
      
      @param destination The destination image.
      @param source The source image.
    */
    Transformation(DestinationImage* destination, const SourceImage* source) throw();
  };

  template<class DEST, class SRC>
  inline Transformation<DEST, SRC>::Transformation(DestinationImage* dest, const SourceImage* src) throw()
    : destination(dest), source(src) {
  }

}; // end of gip namespace

#endif
