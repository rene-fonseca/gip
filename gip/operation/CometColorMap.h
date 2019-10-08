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

#include <gip/operation/ColorMap.h>

namespace gip {

  /**
    Color map. Black -> blue -> magenta -> red -> yellow -> white. Isothes.

    @short Comet color map.
    @ingroup colormaps
    @version 1.0
  */
  class CometColorMap : public UnaryOperation<RGBPixel<long double>, long double> {
  public:

    inline RGBPixel<long double> operator()(const long double& value) const throw() {
      RGBPixel<long double> result;
      if (value <= 0) { // clamp to black
        result.red = 0;
        result.green = 0;
        result.blue = 0;
        return result;
      }
      long double temp = value * 5;
      int section = static_cast<int>(temp);
      long double fraction = temp - section;
      switch (section) { // floor
      case 0: // black -> blue
        result.red = 0;
        result.green = 0;
        result.blue = fraction;
        break;
      case 1: // blue -> magenta
        result.red = fraction;
        result.green = 0;
        result.blue = 1;
        break;
      case 2: // magenta -> red
        result.red = 1;
        result.green = 0;
        result.blue = 1 - fraction;
        break;
      case 3: // red -> yellow
        result.red = 1;
        result.green = fraction;
        result.blue = 0;
        break;
      case 4: // yellow -> white
        result.red = 1;
        result.green = 1;
        result.blue = fraction;
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
