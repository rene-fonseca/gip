/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__FIND_MAXIMUM_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__FIND_MAXIMUM_H

#include <gip/transformation/UnaryTransformation.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>

namespace gip {
  
  /**
    @short Find the maximum element (normally intensity or mapped intensity) of an element
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.1
  */
  
  template<class DEST>
  class FindMaximum : public UnaryTransformation<DEST> {
  public:

    typedef typename UnaryTransformation<DEST>::DestinationImage DestinationImage;
    
    /** Nests an unary operation to another unary operation. */
    template<class UNOPRARG, class UNOPRRES>
    class NestOperations : UnaryOperation<typename UNOPRARG::Argument, typename UNOPRRES::Result> {
    protected:

      UNOPRARG innerOperation;
      UNOPRRES outerOperation;
    public:

      typedef typename UnaryOperation<typename UNOPRARG::Argument, typename UNOPRRES::Result>::Argument Argument;
      typedef typename UnaryOperation<typename UNOPRARG::Argument, typename UNOPRRES::Result>::Result Result;
      
      inline NestOperations(UNOPRARG inner, UNOPRRES outer) throw()
        : innerOperation(inner), outerOperation(outer) {
      }

      inline void operator()(const Argument& value) throw() {
        outerOperation(innerOperation(value));
      }

      inline Result getResult() const throw() {
        return outerOperation.getResult();
      }
    };

    /** Returns an unary operation from a binary operation using a value as the first argument. */
    template<class UNOPRARG, class UNOPRRES>
    inline NestOperations<UNOPRARG, UNOPRRES> nestOperations(UNOPRARG& inner, const UNOPRRES& outer) {
      return NestOperations<UNOPRARG, UNOPRRES>(inner, outer);
    }

    class ComplexToSqrModulus : public UnaryOperation<Complex, long double> {
    public:

      inline ComplexToSqrModulus() throw() {
      }

      inline Result operator()(const Argument& value) throw() {
        return value.getSqrModulus();
      }
    };
    
    FindMaximum(const DestinationImage* destination) throw()
      : UnaryTransformation<DestinationImage>(destination) {
    }

    long double operator()() throw() { // DestinationImage::Pixel
      ComplexToSqrModulus innerOperation;
      NestOperations<ComplexToSqrModulus, Maximum<long double> > operation(
        innerOperation,
        Maximum<long double>(0)
      );
      forEach(*destination, operation);
      return operation.getResult();
    }
  };

}; // end of gip namespace

#endif
