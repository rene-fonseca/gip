/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__TSR_TRANSFORMATION_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__TSR_TRANSFORMATION_H

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>

namespace gip {

/**
  @short Translation, scaling, and rotation transformation.
  @author René Møller Fonseca
*/

class TSRTransformation : public Transformation<ColorImage, ColorImage> {
public:

  TSRTransformation(DestinationImage* destination, const SourceImage* source) throw();

  void operator()() throw();
};

}; // end of namespace

#endif
