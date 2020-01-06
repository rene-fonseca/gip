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
    Color map. Blue -> cyan -> green -> yellow -> red.

    @short Cold to hot color map.
    @ingroup colormaps
    @version 1.0
  */
  class ColdHotColorMap : public UnaryOperation<RGBPixel<double>, double> {
  public:

    inline RGBPixel<double> operator()(const double value) const noexcept {
      RGBPixel<double> result;
      if (value <= 0) {
        result.red = 0;
        result.green = 0;
        result.blue = 1;
        return result;
      }
      double temp = value * 4;
      int section = static_cast<int>(temp);
      double fraction = temp - section;
      switch (section) { // floor
      case 0: // blue -> cyan
        result.red = 0;
        result.green = fraction;
        result.blue = 1;
        break;
      case 1: // cyan -> green
        result.red = 0;
        result.green = 1;
        result.blue = 1 - fraction;
        break;
      case 2: // green -> yellow
        result.red = fraction;
        result.green = 1;
        result.blue = 0;
        break;
      case 3: // yellow -> red
        result.red = 1;
        result.green = 1 - fraction;
        result.blue = 0;
        break;
      default: // clamp to red
        result.red = 1;
        result.green = 0;
        result.blue = 0;
        break;
      }
      return result;
    }

  };

}; // end of gip namespace
