/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Scale.h>

namespace gip {

template Scale<ColorImage, ColorImage>;

template<class DEST, class SRC>
Scale<DEST, SRC>::Scale(DestinationImage* destination, const SourceImage* source) throw() :
  Transformation<DEST, SRC>(destination, source) {
}

template<class DEST, class SRC>
void Scale<DEST, SRC>::operator()() throw() {
  assert(source->getDimension().isProper(), Exception("Unable to scale image"));

  unsigned int rows = destination->getDimension().getHeight();
  unsigned int columns = destination->getDimension().getWidth();
  unsigned int srcRows = source->getDimension().getHeight();
  unsigned int srcColumns = source->getDimension().getWidth();
  double rowRatio = (double)srcRows/rows;
  double columnRatio = (double)srcColumns/columns;

  typename DestinationImage::Rows rowsLookup = destination->getRows();
  typename SourceImage::ReadableRows srcRowsLookup = source->getRows();

  typename DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
  for (unsigned int rowIndex = 0; rowIndex < rows; ++rowIndex) {
    typename SourceImage::ReadableRows::RowIterator srcRow = srcRowsLookup[static_cast<unsigned int>(rowIndex * rowRatio)];

    typename DestinationImage::Rows::RowIterator::ElementIterator column = row.getFirst();
    for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex) {
      *column = *srcRow[static_cast<unsigned int>(columnIndex * columnRatio)];
      ++column;
    }
    ++row;
  }
}

}; // end of namespace
