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

#include <gip/transformation/UnaryTransformation.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>

namespace gip {
  
  /**
    @short Finds the maximum element (normally intensity or mapped intensity)
    of an element.
    @ingroup transformations
    @version 1.1
  */
  
  template<class DEST>
  class FindMaximum : public UnaryTransformation<DEST> {
  public:

    typedef typename UnaryTransformation<DEST>::DestinationImage DestinationImage;
    
    /* Nests an unary operation to another unary operation. */
    template<class UNOPRARG, class UNOPRRES>
    class NestOperations : UnaryOperation<typename UNOPRARG::Argument, typename UNOPRRES::Result> {
    protected:

      UNOPRARG innerOperation;
      UNOPRRES outerOperation;
    public:

      typedef typename UnaryOperation<typename UNOPRARG::Argument, typename UNOPRRES::Result>::Argument Argument;
      typedef typename UnaryOperation<typename UNOPRARG::Argument, typename UNOPRRES::Result>::Result Result;
      
      inline NestOperations(UNOPRARG inner, UNOPRRES outer) noexcept
        : innerOperation(inner), outerOperation(outer) {
      }

      inline void operator()(const Argument& value) noexcept {
        outerOperation(innerOperation(value));
      }

      inline Result getResult() const noexcept {
        return outerOperation.getResult();
      }
    };

    /* Returns an unary operation from a binary operation using a value as the first argument. */
    template<class UNOPRARG, class UNOPRRES>
    inline NestOperations<UNOPRARG, UNOPRRES> nestOperations(UNOPRARG& inner, const UNOPRRES& outer) {
      return NestOperations<UNOPRARG, UNOPRRES>(inner, outer);
    }

    class ComplexToSqrModulus : public UnaryOperation<Complex, long double> {
    public:

      inline ComplexToSqrModulus() noexcept {
      }

      inline Result operator()(const Argument& value) noexcept {
        return value.getSqrModulus();
      }
    };
    
    FindMaximum(const DestinationImage* destination) noexcept
      : UnaryTransformation<DestinationImage>(destination) {
    }

    double operator()() noexcept { // DestinationImage::Pixel
      ComplexToSqrModulus innerOperation;
      NestOperations<ComplexToSqrModulus, Maximum<double> > operation(
        innerOperation,
        Maximum<double>(0)
      );
      forEach(*UnaryTransformation<DestinationImage>::destination, operation);
      return operation.getResult();
    }
  };

}; // end of gip namespace
