/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__NOISE_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__NOISE_H

#include <gip/transformation/UnaryTransformation.h>

namespace gip {

/**
  Fills an image with noise.

  @author René Møller Fonseca
*/

class Noise : public UnaryTransformation<ColorImage> {
public:

  /**
    Initializes noise object.

    @param destination The destination image.
  */
  Noise(DestinationImage* destination) throw();

  /**
    Fills the destination image with noise.
  */
  void operator()() throw();
};

}; // end of namespace

#endif
