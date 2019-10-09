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
#include <gip/analysis/Histogram.h>
#include <gip/ImageException.h>

namespace gip {

  template<class DEST, class SRC>
  class EqualizeHistogram {
  };

  /**
    @short Histogram equalization
    @ingroup transformations
    @version 1.0
  */

  template<>
  class EqualizeHistogram<GrayImage, GrayImage> : public Transformation<GrayImage, GrayImage> {
  public:

    typedef SourceImage::Pixel Pixel;
    typedef PixelTraits<Pixel>::Arithmetic Arithmetic;

    class MapPixel : public UnaryOperation<Pixel, Pixel> {
    private:
      
      const Pixel* lookup = nullptr;
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
      
      bassert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    void operator()() const throw() {
      GrayHistogram grayHistogram;
      forEach(*source, grayHistogram);
      Array<unsigned int> histogram = grayHistogram.getHistogram();
      
      const unsigned int* src = histogram.getElements();
      const unsigned int* end = src + histogram.getSize();

      Allocator<Pixel> lookup(static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + 1);
      Pixel* dest = lookup.getElements();

      unsigned int numberOfPixels = source->getDimension().getSize();
      unsigned long sum = 0;
      if (static_cast<unsigned long long>(numberOfPixels) * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) <= PrimitiveTraits<unsigned long>::MAXIMUM) {
        while (src < end) {
          sum += *src++;
          *dest++ = (2 * sum * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + numberOfPixels)/(2 * numberOfPixels);
        }
      } else {
        while (src < end) {
          sum += *src++;
          *dest++ = (2 *  static_cast<unsigned long long>(sum) * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + numberOfPixels)/(2 * numberOfPixels);
        }
      }

      MapPixel mapPixel(lookup);
      fillWithUnary(*destination, *source, mapPixel);
    }
    
  };



  template<>
  class EqualizeHistogram<ColorImage, ColorImage> : public Transformation<ColorImage, ColorImage> {
  private:

    typedef SourceImage::Pixel Pixel;
    typedef PixelTraits<Pixel>::Component Component;
    typedef PixelTraits<Pixel>::Arithmetic Arithmetic;

    static void fillLookup(const Array<unsigned int>& histogram, Allocator<Arithmetic>& lookup, unsigned int numberOfPixels) throw() {
      const unsigned int* src = histogram.getElements();
      const unsigned int* end = src + histogram.getSize();
      Arithmetic* dest = lookup.getElements();

      unsigned long sum = 0;
      if (2 * static_cast<unsigned long long>(numberOfPixels) * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + numberOfPixels <= PrimitiveTraits<unsigned long>::MAXIMUM) {
        while (src < end) {
          sum += *src++;
          *dest++ = ((2 * sum * 3 * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM)) + numberOfPixels)/(2 * numberOfPixels);
        }
      } else {
        while (src < end) {
          sum += *src++;
          *dest++ = (2 * static_cast<unsigned long long>(sum) * 3 * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + numberOfPixels)/(2 * numberOfPixels);
        }
      }
    }
  public:

    class Histogram : public UnaryOperation<ColorPixel, void> {
    private:

      typedef PixelTraits<ColorPixel>::Arithmetic Arithmetic;
      /** The histogram. */
      Array<unsigned int> histogram;
      /** The histogram elements. */
      unsigned int* elements;
    public:

      inline Histogram() throw(MemoryException)
        : histogram(3 * PixelTraits<ColorPixel>::MAXIMUM + 1, 0), elements(histogram.getElements()) {
        fill<unsigned int>(histogram.getElements(), histogram.getSize(), 0);
      }

      inline void operator()(const ColorPixel& value) throw() {
        ++elements[static_cast<Arithmetic>(value.red) + static_cast<Arithmetic>(value.green) + static_cast<Arithmetic>(value.blue)];
      }

      inline Array<unsigned int> getHistogram() const throw() {
        return histogram;
      }
    };

    class FindMaximumComponent : public UnaryOperation<Pixel, void> {
    private:

      const Arithmetic* lookup;
      Arithmetic max;
      Arithmetic maxIntensity;
    public:

      inline FindMaximumComponent(const Allocator<Arithmetic>& _lookup) throw() : lookup(_lookup.getElements()) {
        max = 0;
        maxIntensity = 1;
      }

      inline Arithmetic getMaximum() throw() {
        return max;
      }

      inline Arithmetic getMaximumIntensity() throw() {
        return maxIntensity - 1; // return value > 0
      }

      inline void operator()(const Pixel& value) throw() {
        Arithmetic red = value.red;
        Arithmetic green = value.green;
        Arithmetic blue = value.blue;
        Arithmetic intensity = red + green + blue;
        Arithmetic mapped = maximum(red, green, blue) * lookup[intensity]; // mapped <= 255 * 3 * 255
        ++intensity; // intensity <= 3 * 255 + 1
        if (mapped * maxIntensity > max * intensity) { // products <= (3 * 255 + 1) * 255 * 3 * 255 => no overflow
          max = mapped;
          maxIntensity = intensity;
        }
      }
    };

    class MapPixel : public UnaryOperation<Pixel, Pixel> {
    private:

      const Arithmetic* lookup;
      Arithmetic max;
    public:

      inline MapPixel(const Allocator<Arithmetic>& _lookup, Arithmetic _max) throw()
         : lookup(_lookup.getElements()),
           max(_max) {
      }

      inline Pixel operator()(const Pixel& value) const throw() {
        // 0 < max <= 255 and 0 < maximumIntensity <= 3 * 255
        Arithmetic red = value.red;
        Arithmetic green = value.green;
        Arithmetic blue = value.blue;
        Arithmetic intensity = red + green + blue;
        if (intensity > 0) {
          Arithmetic newIntensity = lookup[intensity]; // <= 2 * 255 * 3 * 255 * 3 * 255
          red = (red * static_cast<long long>(newIntensity) + max * intensity)/(2 * max * intensity);
          green = (green * static_cast<long long>(newIntensity) + max * intensity)/(2 * max * intensity);
          blue = (blue * static_cast<long long>(newIntensity) + max * intensity)/(2 * max * intensity);
        }
        return makeColorPixel(red, green, blue);
      }
    };

    /**
      Initializes the transformation.

      @param destination The destination image.
      @param source The source image.
    */
    EqualizeHistogram(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<DestinationImage, SourceImage>(destination, source) {

      bassert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    void operator()() const throw() {
      Histogram intensityHistogram; // intensity = red + green + blue <= 3 * 255
      forEach(*source, intensityHistogram);
      Array<unsigned int> histogram = intensityHistogram.getHistogram();
      if (histogram[0] == source->getDimension().getSize()) { // all black image - maximum intensity = 0
        fill(destination->getElements(), destination->getDimension().getSize(), makeColorPixel(0, 0, 0)); // fill with black
        return;
      }
      Allocator<Arithmetic> lookup(3 * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) + 1);
      fillLookup(histogram, lookup, source->getDimension().getSize());
      // TAG: need alternative with clamp
      FindMaximumComponent findMaximumComponent(lookup);
      forEach(*source, findMaximumComponent);
      transform(lookup.getElements(), lookup.getSize(), bind2Second(Multiply<Arithmetic>(), 2 * static_cast<Arithmetic>(PixelTraits<Pixel>::MAXIMUM) * findMaximumComponent.getMaximumIntensity()));
      MapPixel mapPixel(lookup, findMaximumComponent.getMaximum());
      fillWithUnary(*destination, *source, mapPixel);
    }

  };

}; // end of gip namespace
