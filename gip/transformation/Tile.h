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
    This transformation tiles the source image into the destination image.

    @short Tile.
    @ingroup transformations geometric
    @version 1.0
  */

  template<class DEST, class SRC>
  class Tile : public Transformation<DEST, SRC> {
  public:

    typedef typename Transformation<DEST, SRC>::DestinationImage DestinationImage;
    typedef typename Transformation<DEST, SRC>::SourceImage SourceImage;

    /**
      Initializes transformation object.
    */
    Tile(DestinationImage* destination, SourceImage* source) noexcept;

    /**
      Tiles the image.
    */
    void operator()() noexcept;
  };

  template<class DEST, class SRC>
  Tile<DEST, SRC>::Tile(DestinationImage* destination, SourceImage* source) noexcept
    : Transformation<DestinationImage, SourceImage>(destination, source) {
  }

  template<class DEST, class SRC>
  void Tile<DEST, SRC>::operator()() noexcept {
    Dimension dimension = Transformation<DEST, SRC>::destination->getDimension();
    if (!dimension.isProper()) {
      return; // nothing to do
    }
    // TAG: need offset support

    typename DestinationImage::Rows::RowIterator destRow = Transformation<DEST, SRC>::destination->getRows().getFirst();
    typename SourceImage::ReadableRows::RowIterator srcBegin = Transformation<DEST, SRC>::source->getRows().getFirst();
    typename SourceImage::ReadableRows::RowIterator srcEnd = Transformation<DEST, SRC>::source->getRows().getEnd();

    unsigned int verticalCycles = Transformation<DEST, SRC>::destination->getDimension().getHeight()/Transformation<DEST, SRC>::source->getDimension().getHeight();
    unsigned int horizontalCycles = Transformation<DEST, SRC>::destination->getDimension().getWidth()/Transformation<DEST, SRC>::source->getDimension().getWidth();

    while (verticalCycles--) {
      typename SourceImage::ReadableRows::RowIterator srcRow = srcBegin;
      while (srcRow != srcEnd) {
        typename DestinationImage::Rows::RowIterator::ElementIterator destColumn = destRow.getFirst();
        typename SourceImage::ReadableRows::RowIterator::ElementIterator srcColumnEnd = srcRow.getEnd();

        for (unsigned int cycles = horizontalCycles; cycles > 0; --cycles) {
//          destColumn = copy(destColumn, srcRow.getFirst(), srcRow.getEnd());
          typename SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
          while (srcColumn != srcColumnEnd) {
            *destColumn++ = *srcColumn++;
          }
        }

//        copy(destColumn, destRow.getEnd(), srcRow.getFirst());
        typename DestinationImage::Rows::RowIterator::ElementIterator destColumnEnd = destRow.getEnd();
        typename SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
        while (destColumn != destColumnEnd) {
          *destColumn++ = *srcColumn++;
        }
        ++destRow;
        ++srcRow;
      }
    }

    typename DestinationImage::Rows::RowIterator destEnd = Transformation<DEST, SRC>::destination->getRows().getEnd();
    typename SourceImage::ReadableRows::RowIterator srcRow = srcBegin;
    while (destRow < destEnd) {
      typename DestinationImage::Rows::RowIterator::ElementIterator destColumn = destRow.getFirst();
      typename SourceImage::ReadableRows::RowIterator::ElementIterator srcColumnEnd = srcRow.getEnd();

      for (unsigned int cycles = horizontalCycles; cycles > 0; --cycles) {
//        destColumn = copy(destColumn, srcRow.getFirst(), srcRow.getEnd());
        typename SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
        while (srcColumn != srcColumnEnd) {
          *destColumn++ = *srcColumn++;
        }
      }

//      copy(destColumn, destRow.getEnd(), srcRow.getFirst());
      typename DestinationImage::Rows::RowIterator::ElementIterator destColumnEnd = destRow.getEnd();
      typename SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
      while (destColumn != destColumnEnd) {
        *destColumn++ = *srcColumn++;
      }
      ++destRow;
      ++srcRow;
    }
  }

}; // end of gip namespace
