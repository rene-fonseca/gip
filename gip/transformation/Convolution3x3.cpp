/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Convolution3x3.h>

namespace gip {

  void x() {
    ArrayImage<RGBPixel<float> >* dest = 0;
    ColorImage* src = 0;
    Convolution3x3<ArrayImage<RGBPixel<float> >, ColorImage, SmoothUniformRectangular3x3> convolution(dest, src);
    convolution();
  }
  
}; // end of namespace
