/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

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
    Translation, scaling, and rotation transformation.
    
    @short Translation, scaling, and rotation transformation
    @author Rene Moeller Fonseca
    @version 1.0
  */
  
  class TSRTransformation : public Transformation<ColorImage, ColorImage> {
  public:
    
    TSRTransformation(DestinationImage* destination, const SourceImage* source) throw();
    
    void operator()() throw();
  };

}; // end of namespace

#endif
