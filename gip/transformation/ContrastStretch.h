/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__CONTRAST_STRETCH_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__CONTRAST_STRETCH_H

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/MinimumMaximum.h>
#include <gip/analysis/traverse.h>
#include <gip/ImageException.h>
#include <base/Functor.h>

namespace gip {
  
  /**
    @short Stretches the intensity of the image
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  
  class ContrastStretch : public Transformation<GrayImage, GrayImage> {
  public:

    typedef GrayImage::Pixel Pixel;
    
    class MapPixel : public UnaryOperation<Pixel, Pixel> {
    private:

      Pixel minimum;
      Pixel range;
    public:

      inline MapPixel(const Pixel& _minimum, const Pixel& _maximum) throw()
        : minimum(_minimum),
          range(_maximum - _minimum) {
      }
      
      inline Pixel operator()(const Pixel& value) const throw() {
        return static_cast<PixelTraits<SourceImage::Pixel>::Arithmetic>(value - minimum) *
          PixelTraits<SourceImage::Pixel>::MAXIMUM/range;
      }
    };
    
    /**
      Initializes the transformation.

      @param destination The destination image.
      @param source The source image.
    */
    ContrastStretch(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<GrayImage, GrayImage>(destination, source) {
      
      assert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    void operator()() const throw() {
      MinimumMaximum<Pixel> minmax;
      forEach(*source, minmax);
      MapPixel mapPixel(minmax.getMinimum(), minmax.getMaximum());
      fillWithUnary(*destination, *source, mapPixel);
    }
    
  };

}; // end of gip namespace

#endif
