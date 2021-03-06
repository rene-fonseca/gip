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

#include <gip/gip.h>
#include <base/collection/Array.h>

namespace gip {

  template<class PIXEL>
  class Histogram : public UnaryOperation<PIXEL, void> {
  };

  template<>
  class Histogram<GrayPixel> : public UnaryOperation<GrayPixel, void> {
  private:

    /** The gray histogram. */
    Array<unsigned int> histogram;
    /** The histogram elements. */
    unsigned int* elements = nullptr;
  public:

    Histogram()
      : histogram(PixelTraits<GrayPixel>::MAXIMUM + 1, 0), elements(histogram.getElements())
    {
      reset();
    }

    inline void operator()(const GrayPixel& value) noexcept
    {
      ++elements[value];
    }

    void reset() noexcept
    {
      fill<unsigned int>(histogram.getElements(), histogram.getSize(), 0);
    }

    const Array<unsigned int>& getHistogram() const noexcept {
      return histogram;
    }
  };

  /**
    Gray level histogram operation.
    
    @short Gray level histogram operation
    @ingroup analysis
    @version 1.0
  */

  class _COM_AZURE_DEV__GIP__API GrayHistogram : public UnaryOperation<GrayPixel, void> {
  public:

    typedef Array<unsigned int> Histogram;
  private:

    /** The gray histogram. */
    Histogram gray;
    /** The histogram elements. */
    unsigned int* elements = nullptr;
  public:

    GrayHistogram()
      : gray(PixelTraits<GrayPixel>::MAXIMUM + 1, 0), elements(gray.getElements()) {
      reset();
    }
 
    inline void operator()(const Argument& value) noexcept {
      ++elements[static_cast<unsigned char>(value)];
    }

    void reset() noexcept {
      fill<unsigned int>(gray.getElements(), gray.getSize(), 0);
    }
    
    const Histogram& getHistogram() const noexcept {
      return gray;
    }
  };



  /**
    Color histogram operation.
    
    @short Color histogram operation.
    @version 1.0
  */
  
  class _COM_AZURE_DEV__GIP__API ColorHistogram : public UnaryOperation<ColorPixel, void> {
  public:

    typedef Array<unsigned int> Histogram;
  private:

    /** The red histogram. */
    Histogram red;
    /** The green histogram. */
    Histogram green;
    /** The blue histogram. */
    Histogram blue;
    /** The red histogram elements. */
    unsigned int* redElements = nullptr;
    /** The green histogram elements. */
    unsigned int* greenElements = nullptr;
    /** The blue histogram elements. */
    unsigned int* blueElements = nullptr;
  public:

    ColorHistogram()
      : red(PixelTraits<ColorPixel>::MAXIMUM + 1, 0),
        green(PixelTraits<ColorPixel>::MAXIMUM + 1, 0),
        blue(PixelTraits<ColorPixel>::MAXIMUM + 1, 0),
        redElements(red.getElements()),
        greenElements(green.getElements()),
        blueElements(blue.getElements()) { // TAG: #intensities depends on pixel type
      reset();
    }
    
    inline void operator()(const Argument& value) noexcept {
      ++redElements[static_cast<unsigned char>(value.red)];
      ++greenElements[static_cast<unsigned char>(value.green)];
      ++blueElements[static_cast<unsigned char>(value.blue)];
    }

    void reset() noexcept {
      fill<unsigned int>(red.getElements(), red.getSize(), 0);
      fill<unsigned int>(green.getElements(), green.getSize(), 0);
      fill<unsigned int>(blue.getElements(), blue.getSize(), 0);
    }

    const Histogram& getBlueHistogram() const noexcept {
      return blue;
    }

    const Histogram& getGreenHistogram() const noexcept {
      return green;
    }
    
    const Histogram& getRedHistogram() const noexcept {
      return red;
    }
  };

}; // end of gip namespace
