/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/MinMax.h>
#include <gip/analysis/traverse.h>

namespace gip {

  /** Nests an unary operation to another unary operation. */
  template<class UNOPRARG, class UNOPRRES>
  class NestOperations : UnaryOperation<typename UNOPRARG::Argument, typename UNOPRRES::Result> {
  protected:

    UNOPRARG innerOperation;
    UNOPRRES outerOperation;
  public:

    inline NestOperations(UNOPRARG inner, UNOPRRES outer) throw()
      : innerOperation(inner), outerOperation(outer) {}

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

  class ComplexToSqrModulus : public UnaryOperation<ComplexPixel, long double> {
  public:

    inline ComplexToSqrModulus() throw() {}

    inline Result operator()(const Argument& value) {
      return value.getModulus();
//    return log(1 + value.getModulus());
    }
  };

  FindMaximum::FindMaximum(DestinationImage* destination) throw()
    : UnaryTransformation<DestinationImage>(destination) {
  }

  long double FindMaximum::operator()() throw() {
    ComplexToSqrModulus innerOperation;
    NestOperations<ComplexToSqrModulus, Maximum<long double> > opr(innerOperation, Maximum<long double>(0));
    forEach(*destination, opr);
    return opr.getResult();
  }

}; // end of namespace
