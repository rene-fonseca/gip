/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_OPERATION__GRAY_COLOR_MAP_H
#define _DK_SDU_MIP__GIP_OPERATION__GRAY_COLOR_MAP_H

#include <gip/RGBPixel.h>

namespace gip {

  /**
    Color map from black to white.

    @short Black to white color map.
    @ingroup colormaps
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  class GrayColorMap : public UnaryOperation<RGBPixel<long double>, long double> {
  public:

    inline void RGBPixel<long double> operator()(const long double& value) const throw() {
      RGBPixel<long double> result;
      result.red = clamp(0, value, 1);
      result.green = clamp(0, value, 1);
      result.blue = clamp(0, value, 1);
      return result;
    }

  };

}; // end of gip namespace

#endif
