/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_ANALYSIS__STATISTIC_H
#define _DK_SDU_MIP__GIP_ANALYSIS__STATISTIC_H

#include <gip/Pixel.h>
#include <base/Functor.h>
#include <base/Cast.h>
#include <base/mathematics/Math.h>
#include <base/mathematics/Constants.h>

namespace gip {
  
  /**
    This class provides commonly used statistics support.
    
    @short Statistics.
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class IMAGE>
  class Statistic {
  private:
    
    /** The type of the source image. */
    typedef IMAGE Image;
    /** The pixel type. */
    typedef typename Image::Pixel Pixel;
    /** The number of different pixel values. */
    static const unsigned int NUMBER_OF_SYMBOLS = PixelTraits<Pixel>::MAXIMUM + 1;
    
    /** The number of samples. */
    unsigned int numberOfSamples;
    /** The pixel value frequencies. */
    unsigned int frequency[NUMBER_OF_SYMBOLS];
    /** The minimum frequency. */
    unsigned int minimumFrequency;
    /** The maximum frequency. */
    unsigned int maximumFrequency;
    /** The minimum pixel value. */
    Pixel minimum;
    /** The maximum pixel value. */
    Pixel maximum;
    /** The number of used pixel values. */
    unsigned int used;
    /** The mean value of the pixels. */
    long double mean;
    /** The median. */
    unsigned int median;
    /** The mode. */
    unsigned int mode;
    /** The variance of the pixels. */
    long double variance;
    /** The entropy of the pixels. */
    long double entropy;
  public:
    
    /**
      Initializes the statistic object.
      
      @param image The source image.
    */
    Statistic(const Image& image) throw()
      : numberOfSamples(image.getDimension().getSize()) {
      // count frequency of each pixel value
      unsigned int frequency[NUMBER_OF_SYMBOLS];
      fill<unsigned int>(frequency, getArraySize(frequency), 0);
      
      typename Image::ReadableRows rows = Cast::implicit<const Image>(image).getRows();
      typename Image::ReadableRows::RowIterator row = rows.getFirst();
      for (; row != rows.getEnd(); ++row) {
        typename Image::ReadableRows::RowIterator::ElementIterator column = row.getFirst();
        for (; column != row.getEnd(); ++column) {
          unsigned int value = *column;
          if ((value >= PixelTraits<Pixel>::MINIMUM) && (value <= PixelTraits<Pixel>::MAXIMUM)) {
            ++frequency[value];
          }
        }
      }
      
      mean = 0;
      for (unsigned int i = 0; i < NUMBER_OF_SYMBOLS; ++i) {
        mean += Cast::implicit<long double>(i) * frequency[i];
      }
      mean /= numberOfSamples;
      
      int count = numberOfSamples/2;
      minimumFrequency = numberOfSamples;
      maximumFrequency = 0;
      minimum = PixelTraits<Pixel>::MAXIMUM;
      maximum = PixelTraits<Pixel>::MINIMUM;
      used = 0;
      long double sqrsum = 0;
      entropy = numberOfSamples * Math::log(Cast::implicit<long double>(numberOfSamples));
      for (unsigned int i = 0; i < NUMBER_OF_SYMBOLS; ++i) {
        if (frequency[i] != 0) {
          if (frequency[i] < minimumFrequency) {
            minimumFrequency = frequency[i];
          }
          if (frequency[i] > maximumFrequency) {
            maximumFrequency = frequency[i];
            mode = i;
          }
          if (i < minimum) {
            minimum = i;
          }
          maximum = i;
          if (count >= 0) {
            count -= frequency[i];
            median = i;
          }
          ++used;
          sqrsum += frequency[i] * (i - mean) * (i - mean);
          entropy -= frequency[i] * Math::log(Cast::implicit<long double>(frequency[i]));
        } else {
          minimumFrequency = 0;
        }
      }
      if (numberOfSamples == 0) {
        variance = 0;
      } else {
        variance = sqrsum/(numberOfSamples - 1);
        entropy *= constant::LOG2E/numberOfSamples; // convert to binary units
      }
    }

    /**
      Returns the number of values/samples.
    */
    inline unsigned int getNumberOfSamples() const throw() {
      return numberOfSamples;
    }

    /**
      Returns the number of used pixel value.
    */
    inline unsigned int getUsedValues() const throw() {
      return used;
    }

    /**
      Returns the frequency of the specified pixel value.
    */
    inline unsigned int getFrequency(Pixel value) const throw() {
      if ((value >= PixelTraits<Pixel>::MINIMUM) && (value <= PixelTraits<Pixel>::MAXIMUM)) {
        return frequency[value];
      } else {
        return 0;
      }
    }

    /**
      Returns the minimum frequency.
    */
    inline unsigned int getMinimumFrequency() const throw() {
      return minimumFrequency;
    }

    /**
      Returns the maximum frequency.
    */
    inline unsigned int getMaximumFrequency() const throw() {
      return maximumFrequency;
    }

    /**
      Returns the dynamic range.
    */
    inline unsigned int getDynamicRange() const throw() {
      return maximum - minimum;
    }
    
    /**
      Returns the minimum value.
    */
    inline Pixel getMinimum() const throw() {
      return minimum;
    }

    /**
      Returns the maximum value.
    */
    inline Pixel getMaximum() const throw() {
      return maximum;
    }

    /**
      Returns the mean value.
    */
    inline long double getMean() const throw() {
      return mean;
    }

    /**
      Returns the median (rounded down).
    */
    inline Pixel getMedian() const throw() {
      return median;
    }
    
    /**
      Returns the mode (the most frequent value) of the image. The mode may not
      exist or be unique.
    */
    inline Pixel getMode() const throw() {
      return mode;
    }

    /**
      Returns the variance.
    */
    inline long double getVariance() const throw() {
      return variance;
    }

    /**
      Returns the standard deviation.
    */
    inline long double getDeviation() const throw() {
      return Math::sqrt(variance);
    }

    /**
      Returns the dimensionless coefficient-of-variation.
    */
    inline long double getCoefficientOfVariation() const throw() {
      return Math::sqrt(variance)/mean;
    }
    
    /**
      Returns the entropy in binary units.
    */
    inline long double getEntropy() const throw() {
      return entropy;
    }
  };
  
}; // end of gip namespace

#endif
