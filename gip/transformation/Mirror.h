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
#include <gip/gip.h>

namespace gip {

  /**
     This transformation reverses an image along its horizontal axis.

     @short Mirror
     @ingroup transformations geometric
     @version 1.0
  */
  
  template<class DEST>
  class Mirror : public UnaryTransformation<DEST> {
  public:

    typedef typename UnaryTransformation<DEST>::DestinationImage DestinationImage;

    Mirror(DestinationImage* destination);

    void operator()() noexcept;
  };

  template<class DEST>
  Mirror<DEST>::Mirror(DestinationImage* destination) 
    : UnaryTransformation<DestinationImage>(destination) {
    bassert(
      destination->getDimension().isProper(),
      ImageException("Dimension of image is invalid", this)
    );
  }

  template<class DEST>
  void Mirror<DEST>::operator()() noexcept {
    unsigned int elementsPerRowToSwap = UnaryTransformation<DestinationImage>::destination->getWidth()/2;
    typename DestinationImage::Rows rowLookup = UnaryTransformation<DestinationImage>::destination->getRows();
    typename DestinationImage::Rows::RowIterator row = rowLookup.getFirst();
    for ( ; row != rowLookup.getEnd(); ++row) {
      typename DestinationImage::Rows::RowIterator::ElementIterator leftColumn = row.getFirst();
      typename DestinationImage::Rows::RowIterator::ElementIterator rightColumn = row.getEnd();
      unsigned int count = elementsPerRowToSwap;
      while (count--) {
        --rightColumn; // order is important
        swapper(*leftColumn, *rightColumn);
        ++leftColumn;
      }
    }
  }

}; // end of gip namespace
