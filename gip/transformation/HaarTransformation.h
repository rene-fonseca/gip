/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__HAAR_TRANSFORMATION_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__HAAR_TRANSFORMATION_H

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
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
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

    unsigned int numberOfColumnIterations;
    unsigned int numberOfRowIterations;
    
    static inline RowElementIterator partialTransform(RowElementIterator even, RowElementIterator odd, RowElementIterator src, unsigned int size) throw() {
      const RowElementIterator begin = src - size;
      while (src != begin) { // reversed order 'cause we must never overwrite results
        Pixel o = *--src;
        Pixel e = *--src;
        *--odd = o - e; // predict
        *--even = (e + o)/2; // update
      }
      return odd;      
    }
    
    static inline ColumnElementIterator partialColumnTransform(ColumnElementIterator even, ColumnElementIterator odd, ColumnElementIterator src, unsigned int size) throw() {
      const ColumnElementIterator begin = src - size;
      while (src != begin) { // reversed order 'cause we must never overwrite results
        Pixel o = *--src;
        Pixel e = *--src;
        *--odd = o - e; // predict
        *--even = (e + o)/2; // update
      }
      return odd;
    }
    
    static inline RowElementIterator partialITransform(RowElementIterator dest, RowElementIterator even, RowElementIterator odd, unsigned int size) throw() {
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
    HaarTransformation(DestinationImage* destination) throw(ImageException);

    /**
      Calculate transformation.
    */
    void operator()() throw();
  };

  template<>
  class HaarTransformation<GrayImage> : public UnaryTransformation<GrayImage> {
  private:
    
    typedef DestinationImage::Rows::RowIterator::ElementIterator RowElementIterator;
    typedef DestinationImage::Columns::ColumnIterator::ElementIterator ColumnElementIterator;
    typedef DestinationImage::Pixel Pixel;

    unsigned int numberOfColumnIterations;
    unsigned int numberOfRowIterations;

    static inline RowElementIterator partialTransform(RowElementIterator even, RowElementIterator odd, RowElementIterator src, unsigned int size) throw() {
      const RowElementIterator begin = src - size;
      while (src != begin) { // reversed order 'cause we must never overwrite results
        int o = *--src;
        int e = *--src;
        *--odd = o - e; // predict
        *--even = (e + o)/2; // update
      }
      return odd;
    }
    
    static inline ColumnElementIterator partialColumnTransform(ColumnElementIterator even, ColumnElementIterator odd, ColumnElementIterator src, unsigned int size) throw() {
      const ColumnElementIterator begin = src - size;
      while (src != begin) { // reversed order 'cause we must never overwrite results
        int o = *--src;
        int e = *--src;
        *--odd = o - e; // predict
        *--even = (e + o)/2; // update
      }
      return odd;
    }

    static inline RowElementIterator partialITransform(RowElementIterator dest, RowElementIterator even, RowElementIterator odd, unsigned int size) throw() {
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

    static inline ColumnElementIterator partialIColumnTransform(ColumnElementIterator dest, ColumnElementIterator even, ColumnElementIterator odd, unsigned int size) throw() {
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

    HaarTransformation(DestinationImage* destination) throw(ImageException);

    void operator()() throw();
    
    /**
      Calculates the inverse Haar transformation.
    */
    void inverse() throw();
  };
  
}; // end of namespace gip

#endif
