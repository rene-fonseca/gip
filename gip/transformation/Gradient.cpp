/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Gradient.h>
#include <base/math/Constants.h>
#include <base/math/Math.h>

namespace gip {

  Gradient::Gradient(DestinationImage* destination, const SourceImage* source) 
    : Transformation<DestinationImage, SourceImage>(destination, source) {
    bassert(
      destination->getDimension() == source->getDimension(),
      ImageException("Images must have identical dimensions", this)
    );
  }

  void Gradient::operator()() noexcept {
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
        verticalGray += -constant::SQRT2 * srcRow0[columnIndex - 1];
        verticalGray += -2 * srcRow0[columnIndex];
        verticalGray += -constant::SQRT2 * srcRow0[columnIndex + 1];

//      verticalGray += 0 * srcRow1[columnIndex - 1];
//      verticalGray += 0 * srcRow1[columnIndex];
//      verticalGray += 0 * srcRow1[columnIndex + 1];

        verticalGray += constant::SQRT2 * srcRow2[columnIndex - 1];
        verticalGray += 2 * srcRow2[columnIndex];
        verticalGray += constant::SQRT2 * srcRow2[columnIndex + 1];

        double horizontalGray = 0;
        horizontalGray += -constant::SQRT2 * srcRow0[columnIndex - 1];
//      horizontalGray += 0 * srcRow0[columnIndex];
        horizontalGray += constant::SQRT2 * srcRow0[columnIndex + 1];

        horizontalGray += -2 * srcRow1[columnIndex - 1];
//      horizontalGray += 0 * srcRow1[columnIndex];
        horizontalGray += 2 * srcRow1[columnIndex + 1];

        horizontalGray += -constant::SQRT2 * srcRow2[columnIndex - 1];
//      horizontalGray += 0 * srcRow2[columnIndex];
        horizontalGray += constant::SQRT2 * srcRow2[columnIndex + 1];

//      double gray = Math::abs(verticalGray) + Math::abs(horizontalGray);
        double gray = Math::sqrt(verticalGray * verticalGray + horizontalGray * horizontalGray);
        *column++ = minimum<unsigned char>(static_cast<unsigned char>(gray), 0xff);
      }
      ++row;
    }
  }

}; // end of gip namespace
