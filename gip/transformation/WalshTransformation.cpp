/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/WalshTransformation.h>

namespace gip {

  WalshTransformation::WalshTransformation(DestinationImage* destination, const SourceImage* source) throw(ImageException)
    : Transformation<DestinationImage, SourceImage>(destination, source) {

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

  void WalshTransformation::operator()() throw() {

    unsigned int rows = source->getHeight();
    unsigned int columns = source->getWidth();

    // copy/reorder source image to destination image
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

    // Walsh transformation row by row
    {
      DestinationImage::Rows rowsLookup = destination->getRows();
      DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
      for (; row != rowsLookup.getEnd(); ++row) {
        ElementIterator endPoint = row.getEnd();
        for (unsigned int halfBlockSize = 1; halfBlockSize < columns; halfBlockSize <<= 1) { // double size of block per loop
          unsigned int blockSize = halfBlockSize << 1;
          ElementIterator offset = row.getFirst();
          ElementIterator endOffset = offset + halfBlockSize;
          for (; offset != endOffset; ++offset) {
            ElementIterator evenBlockPoint = offset;
            ElementIterator oddBlockPoint = offset + halfBlockSize;
            while (evenBlockPoint < endPoint) {
              DestinationImage::Pixel temp = *oddBlockPoint;
              *oddBlockPoint = *evenBlockPoint - temp;
              *evenBlockPoint += temp;
              evenBlockPoint += blockSize;
              oddBlockPoint += blockSize;
            }
          }
        }
      }
    }

    // Walsh transformation column by column
    {
      DestinationImage::Pixel* column = destination->getElements();
      const DestinationImage::Pixel* endColumn = column + columns;
      const DestinationImage::Pixel* endPoint = column + columns * rows;
      for (; column < endColumn; ++column) {
        for (unsigned int halfStep = columns; halfStep < columns * rows; halfStep <<= 1) {
          unsigned int fullStep = halfStep << 1;
          DestinationImage::Pixel* offset = column;
          const DestinationImage::Pixel* endOffset = offset + halfStep;
          for (; offset < endOffset; offset += columns) {
            DestinationImage::Pixel* evenBlockPoint = offset;
            DestinationImage::Pixel* oddBlockPoint = offset + halfStep;
            while (evenBlockPoint < endPoint) {
              DestinationImage::Pixel temp = *oddBlockPoint;
              *oddBlockPoint = *evenBlockPoint - temp;
              *evenBlockPoint += temp;
              evenBlockPoint += fullStep;
              oddBlockPoint += fullStep;
            }
          }
        }
      }
    }
  }

}; // end of gip namespace
