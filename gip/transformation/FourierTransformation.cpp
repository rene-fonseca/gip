/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Ren� M�ller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/FourierTransformation.h>
#include <math.h>

namespace gip {

inline bool isPowerOfTwo(unsigned int value) throw() {
  if (value == 0) {
    return true;
  }
  unsigned int current = 1;
  while (current) { // until bit has been shifted out
    if (value == current) {
      return true;
    }
    current <<= 1; // times two
  }
  return false;
}

FourierTransformation::FourierTransformation(DestinationImage* destination, const SourceImage* source, bool f) throw(ImageException) :
  Transformation<DestinationImage, SourceImage>(destination, source), forward(f) {

  assert(
    source->getDimension().isProper(),
    ImageException("Source image has inproper dimension")
  );
  assert(
    destination->getDimension() == source->getDimension(),
    ImageException("Source and destination images must have equal dimension")
  );
  assert(
    isPowerOfTwo(source->getDimension().getWidth()) && isPowerOfTwo(source->getDimension().getHeight()),
    ImageException("Width and height of images must be power of two")
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

void FourierTransformation::operator()() throw() {

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
        *row[*mappedColumn++] = *srcColumn;
      }
    }
  }

  typedef DestinationImage::Rows::RowIterator::ElementIterator ElementIterator;

  // Fourier transformation row by row
  {
    DestinationImage::Rows rowsLookup = destination->getRows();
    DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
    for (; row != rowsLookup.getEnd(); ++row) {
      ElementIterator endPoint = row.getEnd();
      double delta = forward ? M_PIl : -M_PIl;
      for (unsigned int halfBlockSize = 1; halfBlockSize < columns; halfBlockSize <<= 1) { // double size of block per loop
        unsigned int blockSize = halfBlockSize << 1;
        Complex u(1, 0); // (cos(0); sin(0))
        Complex w(cos(delta), -sin(delta));
        delta *= 0.5;
        ElementIterator offset = row.getFirst();
        ElementIterator endOffset = offset + halfBlockSize;
        for (; offset != endOffset; ++offset) {
          ElementIterator evenBlockPoint = offset;
          ElementIterator oddBlockPoint = offset + halfBlockSize;
          while (evenBlockPoint < endPoint) {
            Complex temp = u * *oddBlockPoint;
            *oddBlockPoint = *evenBlockPoint - temp;
            *evenBlockPoint += temp;
            evenBlockPoint += blockSize;
            oddBlockPoint += blockSize;
          }
          u *= w;
        }
      }
    }
  }

  // Fourier transformation column by column
  {
    Complex* column = destination->getElements();
    const Complex* endColumn = column + columns;
    const Complex* endPoint = column + columns * rows;
    for (; column < endColumn; ++column) {
      double delta = forward ? M_PIl : -M_PIl;
      for (unsigned int halfStep = columns; halfStep < columns * rows; halfStep <<= 1) {
        unsigned int fullStep = halfStep << 1;
        Complex u(1, 0); // (cos(0); sin(0))
        Complex w(cos(delta), -sin(delta));
        delta *= 0.5;
        Complex* offset = column;
        const Complex* endOffset = offset + halfStep;
        for (; offset < endOffset; offset += columns) {
          Complex* evenBlockPoint = offset;
          Complex* oddBlockPoint = offset + halfStep;
          while (evenBlockPoint < endPoint) {
            Complex temp = u * *oddBlockPoint;
            *oddBlockPoint = *evenBlockPoint - temp;
            *evenBlockPoint += temp;
            evenBlockPoint += fullStep;
            oddBlockPoint += fullStep;
          }
          u *= w;
        }
      }
    }
  }

//      for (unsigned int blockSize = 1; blockSize < rows; blockSize <<= 1) { // double size of block per loop
//        double beta = sin(delta);
//        delta *= 0.5;
//        double alpha = sin(delta);
//        alpha *= 2 * alpha;
//        ComplexPixel* evenBlockPoint = elements + columnIndex;
//        ComplexPixel* oddBlockPoint = evenBlockPoint;
//        while (evenBlockPoint < endPoint) {
//          oddBlockPoint += blockSize * columns;
//          double ar = 1; // cos(0)
//          double ai = 0; // sin(0)
//          const ComplexPixel* stopPoint = oddBlockPoint;
//          while (evenBlockPoint < stopPoint) {
//            double tr = ar * oddBlockPoint->real - ai * oddBlockPoint->imaginary;
//            double ti = ar * oddBlockPoint->imaginary + ai * oddBlockPoint->real;
//            oddBlockPoint->real = evenBlockPoint->real - tr;
//            oddBlockPoint->imaginary = evenBlockPoint->imaginary - ti;
//            evenBlockPoint->real += tr;
//            evenBlockPoint->imaginary += ti;
//            double delta_ar = alpha * ar + beta * ai;
//            ai -= (alpha * ai - beta * ar);
//            ar -= delta_ar;
//            evenBlockPoint += columns;
//            oddBlockPoint += columns;
//          }
//          evenBlockPoint = oddBlockPoint;
//        }
//      }
//    }
//  }
}

}; // end of namespace