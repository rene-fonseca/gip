/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/DiscreteCosineTransformation.h>
#include <base/mathematics/Constants.h>
#include <base/mathematics/Math.h>

namespace gip {

DiscreteCosineTransformation::DiscreteCosineTransformation(DestinationImage* destination, const SourceImage* source, bool _forward) throw(ImageException) :
  Transformation<DestinationImage, SourceImage>(destination, source), forward(_forward) {

  bassert(
    source->getDimension().isProper(),
    ImageException("Source image has inproper dimension", this)
  );
  bassert(
    destination->getDimension() == source->getDimension(),
    ImageException("Source and destination images must have equal dimension", this)
  );
  bassert(
    Math::isPowerOf2(source->getDimension().getWidth()) && Math::isPowerOf2(source->getDimension().getHeight()),
    ImageException("Width and height of images must be power of two", this)
  );

  {
    this->mappedRows.setSize(source->getHeight());
    unsigned int* mappedRows = this->mappedRows.getElements();
    unsigned int count = 1;
    mappedRows[0] = 0;
    for (unsigned int difference = source->getHeight() >> 1; difference != 0; difference >>= 1) {
      transformByUnary(mappedRows + count, mappedRows, count, bind2First(Add<unsigned int>(), difference));
      count <<= 1;
    }
  }

  {
    this->mappedColumns.setSize(source->getWidth());
    unsigned int* mappedColumns = this->mappedColumns.getElements();
    unsigned int count = 1;
    mappedColumns[0] = 0;
    for (unsigned int difference = source->getWidth() >> 1; difference != 0; difference >>= 1) {
      transformByUnary(mappedColumns + count, mappedColumns, count, bind2First(Add<unsigned int>(), difference));
      count <<= 1;
    }
  }
}

void DiscreteCosineTransformation::operator()() throw() {
  unsigned int rows = source->getHeight();
  unsigned int columns = source->getWidth();

  // TAG: should work with GrayImage
  // copy/map source image to destination image
  {
    const unsigned int* mappedRow = mappedRows.getElements();
    DestinationImage::Rows rowLookup = destination->getRows();
    SourceImage::ReadableRows srcRowLookup = source->getRows();
    SourceImage::ReadableRows::RowIterator srcRow = srcRowLookup.getFirst();
    for (; srcRow != srcRowLookup.getEnd(); ++srcRow) { // traverse all rows
      const unsigned int* mappedColumn = mappedColumns.getElements();
      DestinationImage::Rows::RowIterator row = rowLookup[*mappedRow++];
      SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
      for (; srcColumn != srcRow.getEnd(); ++srcColumn) { // traverse all columns of current row
        row[*mappedColumn++] = *srcColumn;
      }
    }
  }
  
  typedef DestinationImage::Rows::RowIterator::ElementIterator ElementIterator;

  // Discrete cosine transformation row by row
  {
    DestinationImage::Rows rowsLookup = destination->getRows();
    DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
    for (; row != rowsLookup.getEnd(); ++row) {
      ElementIterator endPoint = row.getEnd();
      for (unsigned int halfBlockSize = 1; halfBlockSize < columns; halfBlockSize <<= 1) { // double size of block per loop
        const unsigned int blockSize = 2 * halfBlockSize;
        const long double delta = constant::PI/halfBlockSize;
        ElementIterator offset = row.getFirst();
        const ElementIterator endOffset = offset + halfBlockSize;
        for (; offset != endOffset; ++offset) {
          ElementIterator evenBlockPoint = offset;
          ElementIterator oddBlockPoint = offset + halfBlockSize;
          long double u = constant::PI/halfBlockSize * 0.25;
          while (evenBlockPoint < endPoint) {
            long double odd = (*evenBlockPoint - *oddBlockPoint) * Math::cos(u); // TAG: use LUT
            u += delta;
            *evenBlockPoint += *oddBlockPoint;
            *oddBlockPoint = odd;
            evenBlockPoint += blockSize;
            oddBlockPoint += blockSize;
          }
        }
      }
    }
  }

  // Discrete cosine transformation column by column
  {
    Pixel* column = destination->getElements();
    const Pixel* endColumn = column + columns;
    const Pixel* endPoint = column + columns * rows;
    for (; column < endColumn; ++column) { // traverse all columns
      unsigned int halfBlockSize = 1;
      for (unsigned int i = rows/2; i > 0; i >>= 1, halfBlockSize <<= 1) {
        const unsigned int halfStep = halfBlockSize * columns;
        const unsigned int fullStep = 2 * halfBlockSize * columns;
        const long double delta = constant::PI/halfBlockSize;
        const Pixel* endOffset = column + halfStep;
        for (Pixel* offset = column; offset < endOffset; offset += columns) {
          Pixel* evenBlockPoint = offset;
          Pixel* oddBlockPoint = offset + halfStep;
          long double u = constant::PI/halfBlockSize * 0.25;
          while (evenBlockPoint < endPoint) {
            long double odd = (*evenBlockPoint - *oddBlockPoint) * Math::cos(u); // TAG: use LUT
            u += delta;
            *evenBlockPoint += *oddBlockPoint;
            *oddBlockPoint = odd;
            evenBlockPoint += fullStep;
            oddBlockPoint += fullStep;
          }
        }
      }
    }
  }
  
}

}; // end of gip namespace
