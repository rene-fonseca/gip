/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__WALSH_TRANSFORMATION_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__WALSH_TRANSFORMATION_H

#include <gip/gip.h>
#include <gip/transformation/Transformation.h>
#include <base/mem/Allocator.h>

namespace gip {

/**
  Calculates the Fast Walsh Transformation of the specified gray image and
  stores the result in the destination image.

  @short Fast Walsh Transformation (FWT)
  @author René Møller Fonseca
*/

class WalshTransformation : public Transformation<FloatImage, FloatImage> {
private:

  /** Lookup table for row indices. */
  Allocator<unsigned int> mappedRows;
  /** Lookup table for column indices. */
  Allocator<unsigned int> mappedColumns;
public:

  /**
    Initializes Fast Walsh Transformation object.

    @param destination The destination image.
    @param source The source image.
  */
  WalshTransformation(DestinationImage* destination, const SourceImage* source) throw(ImageException);

  /**
    Calculate transformation.
  */
  void operator()() throw();
};

}; // end of namespace

#endif
