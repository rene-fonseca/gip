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
#include <gip/analysis/traverse.h>

namespace gip {

  // temporary - should be enable in base
  template<class TYPE>
  class Same : UnaryOperation<TYPE, TYPE> {
  public:

    typedef typename UnaryOperation<TYPE, TYPE>::Argument Argument;
    typedef typename UnaryOperation<TYPE, TYPE>::Result Result;
    
    inline Result operator()(const Argument& value) const throw() {
      return value;
    }
  };

  /**
    Duplicates the contents of an image.

    @short Image duplication operation
    @version 1.0
  */

  class _DK_SDU_MIP__BASE__API Duplicate : public Transformation<ColorImage, ColorImage> {
  public:

    /**
      Initializes duplication object.

      @param destination The destination image.
      @param source The source image.
    */
    Duplicate(DestinationImage* destination, const SourceImage* source) throw();

    /**
      Duplicates the contents of the source image to the destination image.
    */
    void operator()() throw();
  };

  Duplicate::Duplicate(DestinationImage* destination, const SourceImage* source) throw()
    : Transformation<DestinationImage, SourceImage>(destination, source) {
  }

  void Duplicate::operator()() throw() {
    Same<ColorPixel> operation;
    fillWithUnary(*destination, *source, operation);
  }

}; // end of gip namespace
