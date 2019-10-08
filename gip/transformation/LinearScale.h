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
    Linear scale.

    @see Scale
    @short Linear scale operation
    @ingroup transformations
    @version 1.0
  */

  class LinearScale : public Transformation<ColorImage, ColorImage> {
  public:

    /**
      Initializes scale object.

      @param destination The destination image.
      @param source The source image.
    */
    LinearScale(DestinationImage* destination, const SourceImage* source) throw(ImageException);

    /**
      Scale the source image to the destination image.
    */
    void operator()() throw();
  };

}; // end of gip namespace
