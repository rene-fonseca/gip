/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__SCALE_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__SCALE_H

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>

namespace gip {

/**
  Scale.

  @author René Møller Fonseca
*/

template<class DEST, class SRC>
class Scale : public Transformation<DEST, SRC> {
public:

  /**
    Initializes scale object.

    @param destination The destination image.
    @param source The source image.
  */
  Scale(DestinationImage* destination, const SourceImage* source) throw();

  /**
    Scale the source image to the destination image.
  */
  void operator()() throw();
};

}; // end of namespace

#endif
