/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__GRADIENT_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__GRADIENT_H

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/ImageException.h>

namespace gip {

  /**
    Calculate the gradient of an image.

    @short Gradient of image
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  
  class Gradient : public Transformation<GrayImage, GrayImage> {
  public:

    /**
      Initializes duplication object.

      @param destination The destination image.
      @param source The source image.
    */
    Gradient(DestinationImage* destination, const SourceImage* source) throw(ImageException);

    /**
      Duplicates the contents of the source image to the destination image.
    */
    void operator()() throw();
  };

}; // end of namespace

#endif
