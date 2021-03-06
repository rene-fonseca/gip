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
    Calculates the gradient of an image.

    @short Gradient of image
    @ingroup transformations
    @version 1.0
  */
  
  class _COM_AZURE_DEV__GIP__API Gradient : public Transformation<GrayImage, GrayImage> {
  public:

    /**
      Initializes duplication object.

      @param destination The destination image.
      @param source The source image.
    */
    Gradient(DestinationImage* destination, const SourceImage* source);

    /**
      Duplicates the contents of the source image to the destination image.
    */
    void operator()() noexcept;
  };

}; // end of gip namespace
