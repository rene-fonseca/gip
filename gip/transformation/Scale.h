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

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/ImageException.h>

namespace gip {

  /**
    Scale.

    @short Scaling operation
    @ingroup transformations
    @version 1.0
  */

  template<class DEST, class SRC>
  class Scale : public Transformation<DEST, SRC> {
  public:

    typedef typename Transformation<DEST, SRC>::DestinationImage DestinationImage;
    typedef typename Transformation<DEST, SRC>::SourceImage SourceImage;

    /**
      Initializes scale object.

      @param destination The destination image.
      @param source The source image.
    */
    Scale(DestinationImage* destination, const SourceImage* source) throw(ImageException);
    
    /**
      Scale the source image to the destination image.
    */
    void operator()() noexcept;
  };

}; // end of gip namespace
