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

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/MinimumMaximum.h>
#include <gip/analysis/traverse.h>
#include <gip/ImageException.h>
#include <base/Functor.h>

namespace gip {
  
  template<class DEST, class SRC>
  class ContrastStretch {
  };

  /**
    @short Stretches the intensity of the image
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<>
  class ContrastStretch<GrayImage, GrayImage> : public Transformation<GrayImage, GrayImage> {
  public:

    typedef SourceImage::Pixel Pixel;
    
    class MapPixel : public UnaryOperation<Pixel, Pixel> {
    private:

      typedef PixelTraits<SourceImage::Pixel>::Arithmetic Arithmetic;
      Allocator<Pixel> allocatorLookup;
      Pixel* lookup;
    public:

      inline MapPixel(const Pixel& minimum, const Pixel& maximum) throw()
        : allocatorLookup(static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + 1) {
          lookup = allocatorLookup.getElements();
          lookup += minimum;
          Arithmetic range = maximum - minimum;
          for (int i = 0; i <= range; ++i) {
            *lookup++ = (2 * i * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + range)/(2 * range);
          }
          lookup = allocatorLookup.getElements();
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
    ContrastStretch(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<GrayImage, GrayImage>(destination, source) {
      
      bassert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    void operator()() const throw() {
      MinimumMaximum<Pixel> minmax;
      forEach(*source, minmax);
      MapPixel mapPixel(minmax.getMinimum(), minmax.getMaximum());
      fillWithUnary(*destination, *source, mapPixel);
    }
    
  };

  template<>
  class ContrastStretch<ColorImage, ColorImage> : public Transformation<ColorImage, ColorImage> {
  public:

    typedef SourceImage::Pixel Pixel;
    typedef PixelTraits<SourceImage::Pixel>::Component Component;
    typedef PixelTraits<SourceImage::Pixel>::Arithmetic Arithmetic;

    class MapPixel : public UnaryOperation<Pixel, Pixel> {
    private:

      Allocator<Component> redAllocatorLookup;
      Allocator<Component> greenAllocatorLookup;
      Allocator<Component> blueAllocatorLookup;
      Component* redLookup;
      Component* greenLookup;
      Component* blueLookup;
    public:

      inline MapPixel(const Pixel& minimum, const Pixel& maximum) throw()
        : redAllocatorLookup(static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + 1),
          greenAllocatorLookup(static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + 1),
          blueAllocatorLookup(static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + 1) {
          Arithmetic range;

          redLookup = redAllocatorLookup.getElements();
          redLookup += minimum.red;
          range = maximum.red - minimum.red;
          for (int i = 0; i <= range; ++i) {
            *redLookup++ = (2 * i * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + range)/(2 * range);
          }
          redLookup = redAllocatorLookup.getElements();

          greenLookup = greenAllocatorLookup.getElements();
          greenLookup += minimum.green;
          range = maximum.green - minimum.green;
          for (int i = 0; i <= range; ++i) {
            *greenLookup++ = (2 * i * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + range)/(2 * range);
          }
          greenLookup = greenAllocatorLookup.getElements();

          blueLookup = blueAllocatorLookup.getElements();
          blueLookup += minimum.blue;
          range = maximum.blue - minimum.blue;
          for (int i = 0; i <= range; ++i) {
            *blueLookup++ = (2 * i * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + range)/(2 * range);
          }
          blueLookup = blueAllocatorLookup.getElements();

      }

      inline Pixel operator()(const Pixel& value) const throw() {
        return makeColorPixel(redLookup[value.red], greenLookup[value.green], blueLookup[value.blue]);
      }
    };

    /**
      Initializes the transformation.

      @param destination The destination image.
      @param source The source image.
    */
    ContrastStretch(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<ColorImage, ColorImage>(destination, source) {

      bassert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    void operator()() const throw() {
      MinimumMaximum<Pixel> minmax;
      forEach(*source, minmax);
      MapPixel mapPixel(minmax.getMinimum(), minmax.getMaximum());
      fillWithUnary(*destination, *source, mapPixel);
    }

  };

}; // end of gip namespace
