/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Gradient.h>
#include <math.h>

namespace gip {

Gradient::Gradient(DestinationImage* destination, const SourceImage* source) throw(ImageException) :
  Transformation<DestinationImage, SourceImage>(destination, source) {
  assert(
    destination->getDimension() == source->getDimension(),
    ImageException("Images must have identical dimensions")
  );
}

void Gradient::operator()() throw() {
  unsigned int rows = destination->getDimension().getHeight();
  unsigned int columns = destination->getDimension().getWidth();

  DestinationImage::Rows rowsLookup = destination->getRows();
  SourceImage::ReadableRows srcRowsLookup = source->getRows();

  DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
  ++row;
  for (unsigned int rowIndex = 1; rowIndex < rows - 1; ++rowIndex) {

    SourceImage::ReadableRows::RowIterator srcRow0 = srcRowsLookup[rowIndex - 1];
    SourceImage::ReadableRows::RowIterator srcRow1 = srcRowsLookup[rowIndex];
    SourceImage::ReadableRows::RowIterator srcRow2 = srcRowsLookup[rowIndex + 1];

    DestinationImage::Rows::RowIterator::ElementIterator column = row.getFirst();
    ++column;

    for (unsigned int columnIndex = 1; columnIndex < columns - 1; ++columnIndex) {
      double verticalGray = 0;
      verticalGray += -M_SQRT2 * *srcRow0[columnIndex - 1];
      verticalGray += -2 * *srcRow0[columnIndex];
      verticalGray += -M_SQRT2 * *srcRow0[columnIndex + 1];

//      verticalGray += 0 * *srcRow1[columnIndex - 1];
//      verticalGray += 0 * *srcRow1[columnIndex];
//      verticalGray += 0 * *srcRow1[columnIndex + 1];

      verticalGray += M_SQRT2 * *srcRow2[columnIndex - 1];
      verticalGray += 2 * *srcRow2[columnIndex];
      verticalGray += M_SQRT2 * *srcRow2[columnIndex + 1];

      double horizontalGray = 0;
      horizontalGray += -M_SQRT2 * *srcRow0[columnIndex - 1];
//      horizontalGray += 0 * *srcRow0[columnIndex];
      horizontalGray += M_SQRT2 * *srcRow0[columnIndex + 1];

      horizontalGray += -2 * *srcRow1[columnIndex - 1];
//      horizontalGray += 0 * *srcRow1[columnIndex];
      horizontalGray += 2 * *srcRow1[columnIndex + 1];

      horizontalGray += -M_SQRT2 * *srcRow2[columnIndex - 1];
//      horizontalGray += 0 * *srcRow2[columnIndex];
      horizontalGray += M_SQRT2 * *srcRow2[columnIndex + 1];

//      double gray = fabs(verticalGray) + fabs(horizontalGray);
      double gray = sqrt(verticalGray*verticalGray + horizontalGray*horizontalGray);
      *column++ = minimum(static_cast<Intensity>(gray), MAXIMUM_INTENSITY);
    }
    ++row;
  }
}

}; // end of namespace
