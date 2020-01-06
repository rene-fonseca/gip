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
#include <gip/transformation/UnaryTransformation.h>
#include <base/mem/Allocator.h>

namespace gip {

  /**
    Calculates the Fast Haar Transformation of the specified gray image and
    stores the result in the destination image. Implemented using the lifting
    scheme.
    
    @short Fast Haar Transformation (FWT)
    @ingroup transformations
    @version 1.0
  */

  template<class DEST>
  class HaarTransformation : public UnaryTransformation<DEST> {
  };

  template<>
  class HaarTransformation<FloatImage> : public UnaryTransformation<FloatImage> {
  private:

    typedef DestinationImage::Rows::RowIterator::ElementIterator RowElementIterator;
    typedef DestinationImage::Columns::ColumnIterator::ElementIterator ColumnElementIterator;
    typedef DestinationImage::Pixel Pixel;

    unsigned int numberOfColumnIterations = 0;
    unsigned int numberOfRowIterations = 0;
    
    static inline RowElementIterator partialTransform(RowElementIterator even, RowElementIterator odd, RowElementIterator src, unsigned int size) noexcept {
      const RowElementIterator begin = src - size;
      while (src != begin) { // reversed order 'cause we must never overwrite results
        Pixel o = *--src;
        Pixel e = *--src;
        *--odd = o - e; // predict
        *--even = (e + o)/2; // update
      }
      return odd;
    }
    
    static inline ColumnElementIterator partialColumnTransform(ColumnElementIterator even, ColumnElementIterator odd, ColumnElementIterator src, unsigned int size) noexcept {
      const ColumnElementIterator begin = src - size;
      while (src != begin) { // reversed order 'cause we must never overwrite results
        Pixel o = *--src;
        Pixel e = *--src;
        *--odd = o - e; // predict
        *--even = (e + o)/2; // update
      }
      return odd;
    }
    
    static inline RowElementIterator partialITransform(RowElementIterator dest, RowElementIterator even, RowElementIterator odd, unsigned int size) noexcept {
      const RowElementIterator end = dest + size;
      while (dest != end) { // reversed order 'cause we must never overwrite results
        Pixel e = *even - *odd/2;
        Pixel o = *odd + e;
        ++even;
        ++odd;
        *dest++ = e;
        *dest++ = o;
      }
      return odd;
    }
  public:

    /**
      Initializes Fast Haar Transformation object.

      @param destination The destination image.
    */
    HaarTransformation(DestinationImage* destination);

    /**
      Calculate transformation.
    */
    void operator()() noexcept;
  };

  template<>
  class HaarTransformation<GrayImage> : public UnaryTransformation<GrayImage> {
  private:
    
    typedef DestinationImage::Rows::RowIterator::ElementIterator RowElementIterator;
    typedef DestinationImage::Columns::ColumnIterator::ElementIterator ColumnElementIterator;
    typedef DestinationImage::Pixel Pixel;

    unsigned int numberOfColumnIterations = 0;
    unsigned int numberOfRowIterations = 0;

    static inline RowElementIterator partialTransform(RowElementIterator even, RowElementIterator odd, RowElementIterator src, unsigned int size) noexcept {
      const RowElementIterator begin = src - size;
      while (src != begin) { // reversed order 'cause we must never overwrite results
        int o = *--src;
        int e = *--src;
        *--odd = o - e; // predict
        *--even = (e + o)/2; // update
      }
      return odd;
    }
    
    static inline ColumnElementIterator partialColumnTransform(ColumnElementIterator even, ColumnElementIterator odd, ColumnElementIterator src, unsigned int size) noexcept {
      const ColumnElementIterator begin = src - size;
      while (src != begin) { // reversed order 'cause we must never overwrite results
        int o = *--src;
        int e = *--src;
        *--odd = o - e; // predict
        *--even = (e + o)/2; // update
      }
      return odd;
    }

    static inline RowElementIterator partialITransform(RowElementIterator dest, RowElementIterator even, RowElementIterator odd, unsigned int size) noexcept {
      const RowElementIterator end = dest;
      dest -= size;
      even -= size/2;

      while (dest != end) {
        int avg = static_cast<int>(*even);
        int diff = static_cast<int>(*odd);
        int o = (2 * avg + 1 + diff)/2; // predict
        int e = o - diff; // update
        *dest++ = e;
        *dest++ = o;
        ++even;
        ++odd;
      }
      return odd;
    }

    static inline ColumnElementIterator partialIColumnTransform(ColumnElementIterator dest, ColumnElementIterator even, ColumnElementIterator odd, unsigned int size) noexcept {
      const ColumnElementIterator end = dest;
      dest -= size;
      even -= size/2;
      
      while (dest != end) {
        int avg = static_cast<int>(*even);
        int diff = static_cast<int>(*odd);
        int o = (2 * avg + 1 + diff)/2; // predict
        int e = o - diff; // update
        *dest++ = e;
        *dest++ = o;
        ++even;
        ++odd;
      }
      return odd;
    }
  public:

    HaarTransformation(DestinationImage* destination);

    void operator()() noexcept;
    
    /**
      Calculates the inverse Haar transformation.
    */
    void inverse() noexcept;
  };
  
}; // end of gip namespace
