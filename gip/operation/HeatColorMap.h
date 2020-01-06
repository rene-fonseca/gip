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
    Color map. Black -> dark orange -> bright yellow -> white.

    @short Heat color map.
    @ingroup colormaps
    @version 1.0
  */
  class HeatColorMap : public UnaryOperation<RGBPixel<double>, double> {
  public:

    inline RGBPixel<double> operator()(const double& value) const noexcept
    {
      RGBPixel<double> result;
      if (value <= 0) {
        result.red = 0;
        result.green = 0;
        result.blue = 0;
        return result;
      }
      double temp = value * 3;
      int section = static_cast<int>(temp);
      double fraction = temp - section;
      switch (section) { // floor
      case 0: // black -> dark orange
        result.red = fraction;
        result.green = fraction * 140.0/255;
        result.blue = 0;
        break;
      case 1: // dark orange -> bright yellow
        result.red = 1;
        result.green = fraction * (1 - 140.0/255) + 140.0/255;
        result.blue = fraction/2;
        break;
      case 2: // bright yellow -> white
        result.red = 1;
        result.green = 1;
        result.blue = fraction/2 + 0.5;
        break;
      default: // clamp to white
        result.red = 1;
        result.green = 1;
        result.blue = 1;
        break;
      }
      return result;
    }

  };

}; // end of gip namespace
