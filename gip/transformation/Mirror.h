/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__MIRROR_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__MIRROR_H

#include <gip/transformation/UnaryTransformation.h>
#include <gip/gip.h>

namespace gip {

  /**
     This transformation reverses an image along its horizontal axis.

     @short Mirror
     @ingroup transformations geometric
     @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
     @version 1.0
  */
  
  template<class DEST>
  class Mirror : public UnaryTransformation<DEST> {
  public:

    Mirror(DestinationImage* destination) throw(ImageException);

    void operator()() throw();
  };

  template<class DEST>
  Mirror<DEST>::Mirror(DestinationImage* destination) throw(ImageException)
    : UnaryTransformation<DestinationImage>(destination) {
    assert(
      destination->getDimension().isProper(),
      ImageException("Dimension of image is invalid", this)
    );
  }

  template<class DEST>
  void Mirror<DEST>::operator()() throw() {
    unsigned int elementsPerRowToSwap = destination->getWidth()/2;
    typename DestinationImage::Rows rowLookup = destination->getRows();
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

#endif
