/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/transformation/Transformation.h>
#include <gip/Functor.h>

namespace gip {

  /**
    This transformation transposes the source image.

    @short Transpose.
    @ingroup transformations geometric
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class DEST, class SRC>
  class Transpose : public Transformation<DEST, SRC> {
  public:

    typedef typename Transformation<DEST, SRC>::DestinationImage DestinationImage;
    typedef typename Transformation<DEST, SRC>::SourceImage SourceImage;
    
    /**
      Initializes transformation object.
    */
    Transpose(DestinationImage* destination, SourceImage* source) throw(ImageException);

    /**
      Transpose the image.
    */
    void operator()() throw();
  };

  template<class DEST, class SRC>
  Transpose<DEST, SRC>::Transpose(DestinationImage* destination, SourceImage* source) throw(ImageException)
    : Transformation<DestinationImage, SourceImage>(destination, source) {
    bassert(
      (destination->getDimension().getWidth() == source->getDimension().getHeight()) &&
      (destination->getDimension().getHeight() == source->getDimension().getWidth()),
      ImageException("Incompatible dimensions", this)
    );
  }

  template<class DEST, class SRC>
  void Transpose<DEST, SRC>::operator()() throw() {
    if (!destination->getDimension().isProper()) {
      return; // nothing to do
    }

    typename DestinationImage::Rows::RowIterator destRow = destination->getRows().getFirst();
    typename DestinationImage::Rows::RowIterator destEnd = destination->getRows().getEnd();
    typename SourceImage::ReadableColumns::ColumnIterator srcColumn = source->getColumns().getFirst();

    while (destRow < destEnd) {
      copy(destRow.getFirst(), destRow.getEnd(), srcColumn.getFirst());
      ++destRow;
      ++srcColumn;
    }
  }

}; // end of gip namespace
