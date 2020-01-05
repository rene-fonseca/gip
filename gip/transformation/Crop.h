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

namespace gip {

  /**
    This transformation crops the source image to fit in the destination image.
    Any part of the destination image which exceeds the common area of both
    images is not changed.

    @short Crop.
    @ingroup transformations geometric
    @version 1.0
  */

  template<class DEST, class SRC>
  class Crop : public Transformation<DEST, SRC> {
  public:

    /**
      Initializes transformation object.
    */
    Crop(DestinationImage* destination, SourceImage* source) noexcept;

    /**
      Crops the image.
    */
    void operator()() noexcept;
  };

  template<class DEST, class SRC>
  Crop<DEST, SRC>::Crop(DestinationImage* destination, SourceImage* source) noexcept
    : Transformation<DestinationImage, SourceImage>(destination, source) {
  }

  template<class DEST, class SRC>
  void Crop<DEST, SRC>::operator()() noexcept {
    Dimension min(
      minimum(destination->getDimension().getWidth(), source->getDimension().getWidth()),
      minimum(destination->getDimension().getHeight(), source->getDimension().getHeight())
    );

    typename DestinationImage::Rows::RowIterator destRow = destination->getRows().getFirst();
    typename DestinationImage::Rows::RowIterator destEnd = destRow + min.getHeight();
    typename SourceImage::ReadableRows::RowIterator srcRow = source->getRows().getFirst();

    while (destRow < destEnd) {
      copy(destRow.getFirst(), destRow.getFirst() + min.getWidth(), srcRow.getFirst());
//      typename DestinationImage::Rows::RowIterator::ElementIterator destColumn = destRow.getFirst();
//      typename DestinationImage::Rows::RowIterator::ElementIterator destColumnEnd = destColumn + min.getWidth();
//      typename SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
//      while (destColumn != destColumnEnd) {
//        *destColumn++ = *srcColumn++;
//      }
      ++destRow;
      ++srcRow;
    }
  }

}; // end of gip namespace
