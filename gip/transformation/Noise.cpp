/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Noise.h>

namespace gip {

  class NoiseOperation {
  private:

    ColorPixel pixel;
  public:

    inline NoiseOperation() throw() {
    }

    inline ColorPixel operator()() throw() {
      return pixel;
    }
  };



  Noise::Noise(DestinationImage* destination) throw() : UnaryTransformation<DestinationImage>(destination) {
  }

  void Noise::operator()() throw() {
    //forEach(destination.getElements(), destination.getDimension().getSize(), NoiseOperation());
  }

}; // end of namespace
