/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__EQUALIZE_HISTOGRAM_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__EQUALIZE_HISTOGRAM_H

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/Histogram.h>
#include <gip/ImageException.h>

namespace gip {
  
  /**
    @short Histogram equalization
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  
  class EqualizeHistogram : public Transformation<GrayImage, GrayImage> {
  public:

    typedef GrayImage::Pixel Pixel;
    
    class MapPixel : public UnaryOperation<Pixel, Pixel> {
    private:
      
      const Pixel* lookup;
    public:
      
      inline MapPixel(const Allocator<Pixel>& _lookup) throw()
        : lookup(_lookup.getElements()) {
      }
      
      inline Pixel operator()(const Pixel& value) const throw() {
        return lookup[value];
      }
    };
    
    /**
      Initializes the transformation.

      @param destination The destination image.
      @param source The source image.
    */
    EqualizeHistogram(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<DestinationImage, SourceImage>(destination, source) {
      
      assert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    void operator()() const throw() {
      GrayHistogram grayHistogram;
      forEach(*source, grayHistogram);
      Array<unsigned int> histogram = grayHistogram.getHistogram();
      
      const unsigned int* src = histogram.getElements();
      const unsigned int* end = src + histogram.getSize();

      Allocator<Pixel> lookup(PixelTraits<Pixel>::MAXIMUM + 1);
      Pixel* dest = lookup.getElements();

      unsigned int numberOfPixels = source->getDimension().getSize();
      unsigned long sum = 0;
      if (static_cast<unsigned long long>(numberOfPixels) * PixelTraits<Pixel>::MAXIMUM <= PrimitiveTraits<unsigned long>::MAXIMUM) {
        while (src < end) {
          sum += *src++;
          *dest++ = (sum * PixelTraits<Pixel>::MAXIMUM)/numberOfPixels;
        }
      } else {
        while (src < end) {
          sum += *src++;
          *dest++ = (static_cast<unsigned long long>(sum) * PixelTraits<Pixel>::MAXIMUM)/numberOfPixels;
        }
      }

      MapPixel mapPixel(lookup);
      fillWithUnary(*destination, *source, mapPixel);
    }
    
  };

}; // end of gip namespace

#endif
