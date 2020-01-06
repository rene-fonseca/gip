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

#include <gip/RGBPixel.h>

namespace gip {

  /**
    Color map from black to pure red.

    @short Pure red color map.
    @ingroup colormaps
    @version 1.0
  */
  class RedColorMap : public UnaryOperation<RGBPixel<double>, double> {
  public:

    inline void RGBPixel<double> operator()(const double& value) const noexcept
    {
      RGBPixel<double> result;
      result.red = clamp(0, value, 1);
      result.green = 0;
      result.blue = 0;
      return result;
    }

  };

}; // end of gip namespace
