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

namespace gip {

  /**
    This transformation reverses an image along its vertical axis.

    @short Vertical flip.
    @ingroup transformations geometric
    @version 1.0
  */

  template<class DEST>
  class Flip : public UnaryTransformation<DEST> {
  public:

    typedef typename UnaryTransformation<DEST>::DestinationImage DestinationImage;

    /**
      Initializes transformation object.
    */
    Flip(DestinationImage* destination);

    /**
      Flip the image.
    */
    void operator()() noexcept;
  };

  template<class DEST>
  Flip<DEST>::Flip(DestinationImage* destination) 
    : UnaryTransformation<DestinationImage>(destination) {

    bassert(
      destination->getDimension().isProper(),
      ImageException("Dimension of image is invalid", this)
    );
  }

  template<class DEST>
  void Flip<DEST>::operator()() noexcept {
    typename DestinationImage::Rows rowLookup = UnaryTransformation<DEST>::destination->getRows();

    typename DestinationImage::Rows::RowIterator topRow = rowLookup.getFirst();
    typename DestinationImage::Rows::RowIterator bottomRow = rowLookup.getEnd();
    unsigned int count = UnaryTransformation<DEST>::destination->getHeight()/2;

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
