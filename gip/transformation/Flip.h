/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__FLIP_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__FLIP_H

#include <gip/transformation/UnaryTransformation.h>

namespace gip {

  /**
    This transformation reverses an image along its vertical axis.

    @short Vertical flip.
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class DEST>
  class Flip : public UnaryTransformation<DEST> {
  public:

    /**
      Initializes transformation object.
    */
    Flip(DestinationImage* destination) throw(ImageException);

    /**
      Flip the image.
    */
    void operator()() throw();
  };

  template<class DEST>
  Flip<DEST>::Flip(DestinationImage* destination) throw(ImageException)
    : UnaryTransformation<DestinationImage>(destination) {

    assert(
      destination->getDimension().isProper(),
      ImageException("Dimension of image is invalid", this)
    );
  }

  template<class DEST>
  void Flip<DEST>::operator()() throw() {
    typename DestinationImage::Rows rowLookup = destination->getRows();

    typename DestinationImage::Rows::RowIterator topRow = rowLookup.getFirst();
    typename DestinationImage::Rows::RowIterator bottomRow = rowLookup.getEnd();
    unsigned int count = destination->getHeight()/2;

    while (count--) {
      --bottomRow; // order is important
      typename DestinationImage::Rows::RowIterator::ElementIterator topColumn = topRow.getFirst();
      typename DestinationImage::Rows::RowIterator::ElementIterator bottomColumn = bottomRow.getFirst();
      while (topColumn != topRow.getEnd()) {
        swapper(*topColumn, *bottomColumn);
        ++topColumn;
        ++bottomColumn;
      }
      ++topRow;
    }
  }

}; // end of gip namespace

#endif
