/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/HaarTransformation.h>

namespace gip {

  HaarTransformation<FloatImage>::HaarTransformation<FloatImage>(
    DestinationImage* destination) throw(ImageException)
    : UnaryTransformation<FloatImage>(destination) {

    bassert(
      destination->getDimension().isProper(),
      ImageException("Image has inproper dimension", this)
    );
    bassert(
      Math::isPowerOf2(destination->getDimension().getWidth()) &&
      Math::isPowerOf2(destination->getDimension().getHeight()),
      ImageException("Width and height of images must be power of two", this)
    );
    
    numberOfColumnIterations = Math::iLog2(destination->getDimension().getWidth());
    numberOfRowIterations = Math::iLog2(destination->getDimension().getHeight());
  }

  void HaarTransformation<FloatImage>::operator()() throw() {
    // Haar transformation row by row
    if (numberOfColumnIterations > 0) {
      Pixel buffer[1 << (numberOfColumnIterations - 1)];
      RowElementIterator even(buffer + (1 << (numberOfColumnIterations - 1)));
      
      DestinationImage::Rows rowsLookup = destination->getRows();
      for (DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
           row != rowsLookup.getEnd();
           ++row) {
        unsigned int iteration = numberOfColumnIterations;
        RowElementIterator odd = partialTransform(even, row.getEnd(), row.getEnd(), 1 << iteration--);
        while (iteration >= 8) {
          odd = partialTransform(even, odd, even, 1 << iteration--);
        }
        switch (iteration) {
        case 7:
          odd = partialTransform(even, odd, even, 1 << 7);
        case 6:
          odd = partialTransform(even, odd, even, 1 << 6);
        case 5:
          odd = partialTransform(even, odd, even, 1 << 5);
        case 4:
          odd = partialTransform(even, odd, even, 1 << 4);
        case 3:
          odd = partialTransform(even, odd, even, 1 << 3);
        case 2:
          odd = partialTransform(even, odd, even, 1 << 2);
        case 1:
          odd = partialTransform(even, odd, even, 1 << 1);
        case 0:
          *--odd = *--even; // do not forget DC value
        }
      }
    }

    // Haar transformation column by column
    if (numberOfRowIterations > 0) {
      Pixel buffer[1 << (numberOfRowIterations - 1)];
      ColumnElementIterator even(buffer + (1 << (numberOfRowIterations - 1)), 1);
      
      DestinationImage::Columns columnsLookup = destination->getColumns();
      DestinationImage::Columns::ColumnIterator column = columnsLookup.getFirst();
      for (; column != columnsLookup.getEnd(); ++column) {
        unsigned int iteration = numberOfRowIterations;
        ColumnElementIterator odd =
          partialColumnTransform(even, column.getEnd(), column.getEnd(), 1 << iteration--);
        while (iteration >= 8) {
          odd = partialColumnTransform(even, odd, even, 1 << iteration--);
        }
        switch (iteration) {
        case 7:
          odd = partialColumnTransform(even, odd, even, 1 << 7);
        case 6:
          odd = partialColumnTransform(even, odd, even, 1 << 6);
        case 5:
          odd = partialColumnTransform(even, odd, even, 1 << 5);
        case 4:
          odd = partialColumnTransform(even, odd, even, 1 << 4);
        case 3:
          odd = partialColumnTransform(even, odd, even, 1 << 3);
        case 2:
          odd = partialColumnTransform(even, odd, even, 1 << 2);
        case 1:
          odd = partialColumnTransform(even, odd, even, 1 << 1);
        case 0:
          *--odd = *--even; // do not forget DC value
        }
        
      }
    }
  }

  HaarTransformation<GrayImage>::HaarTransformation<GrayImage>(
    DestinationImage* destination) throw(ImageException)
    : UnaryTransformation<GrayImage>(destination) {
    
    bassert(
      destination->getDimension().isProper(),
      ImageException("Image has inproper dimension", this)
    );
    bassert(
      Math::isPowerOf2(destination->getDimension().getWidth()) &&
      Math::isPowerOf2(destination->getDimension().getHeight()),
      ImageException("Width and height of images must be power of two", this)
    );

    numberOfColumnIterations = Math::iLog2(destination->getDimension().getWidth());
    numberOfRowIterations = Math::iLog2(destination->getDimension().getHeight());
  }

  void HaarTransformation<GrayImage>::operator()() throw() {
    // Haar transformation row by row
    if (numberOfColumnIterations > 0) {
      Pixel buffer[1 << (numberOfColumnIterations - 1)];
      RowElementIterator even(buffer + (1 << (numberOfColumnIterations - 1)));
      
      DestinationImage::Rows rowsLookup = destination->getRows();
      for (DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
           row != rowsLookup.getEnd();
           ++row) {
        unsigned int iteration = numberOfColumnIterations;
        RowElementIterator odd = partialTransform(even, row.getEnd(), row.getEnd(), 1 << iteration--);
        while (iteration >= 1) {
          odd = partialTransform(even, odd, even, 1 << iteration--);
        }
        
//         switch (iteration) {
//         case 7:
//           odd = partialTransform(even, odd, even, 1 << 7);
//         case 6:
//           odd = partialTransform(even, odd, even, 1 << 6);
//         case 5:
//           odd = partialTransform(even, odd, even, 1 << 5);
//         case 4:
//           odd = partialTransform(even, odd, even, 1 << 4);
//         case 3:
//           odd = partialTransform(even, odd, even, 1 << 3);
//         case 2:
//           odd = partialTransform(even, odd, even, 1 << 2);
//         case 1:
//           odd = partialTransform(even, odd, even, 1 << 1);
//         case 0:
          *--odd = *--even; // do not forget DC value
//        }
      }
    }

    // Haar transformation column by column
    if (numberOfRowIterations > 0) {
      Pixel buffer[1 << (numberOfRowIterations - 1)];
      ColumnElementIterator even(buffer + (1 << (numberOfRowIterations - 1)), 1);
      
      DestinationImage::Columns columnsLookup = destination->getColumns();
      for (DestinationImage::Columns::ColumnIterator column = columnsLookup.getFirst();
           column != columnsLookup.getEnd();
           ++column) {
        unsigned int iteration = numberOfRowIterations;
        ColumnElementIterator odd =
          partialColumnTransform(even, column.getEnd(), column.getEnd(), 1 << iteration--);
        while (iteration >= 1) {
          odd = partialColumnTransform(even, odd, even, 1 << iteration--);
        }
//         switch (iteration) {
//         case 7:
//           odd = partialColumnTransform(even, odd, even, 1 << 7);
//         case 6:
//           odd = partialColumnTransform(even, odd, even, 1 << 6);
//         case 5:
//           odd = partialColumnTransform(even, odd, even, 1 << 5);
//         case 4:
//           odd = partialColumnTransform(even, odd, even, 1 << 4);
//         case 3:
//           odd = partialColumnTransform(even, odd, even, 1 << 3);
//         case 2:
//           odd = partialColumnTransform(even, odd, even, 1 << 2);
//         case 1:
//           odd = partialColumnTransform(even, odd, even, 1 << 1);
//         case 0:
          *--odd = *--even; // do not forget DC value
//        }
        
      }
    }
  }

  void HaarTransformation<GrayImage>::inverse() throw() {
    // Haar transformation column by column
    if (numberOfRowIterations > 0) {
      Pixel buffer[1 << (numberOfRowIterations - 1)];
      ColumnElementIterator even(buffer + (1 << (numberOfRowIterations - 1)), 1);
      
      DestinationImage::Columns columnsLookup = destination->getColumns();
      for (DestinationImage::Columns::ColumnIterator column = columnsLookup.getFirst();
           column != columnsLookup.getEnd();
           ++column) {
        unsigned int iteration = 1;
        ColumnElementIterator odd = column.getFirst();
        even[-1] = *odd++; // do not forget DC value
        while (iteration < numberOfRowIterations) {
          odd = partialIColumnTransform(even, even, odd, 1 << iteration++);
        }
        odd = partialIColumnTransform(column.getEnd(), even, odd, 1 << iteration++);
      }
    }

    // Haar transformation row by row
    if (numberOfColumnIterations > 0) {
      Pixel buffer[1 << (numberOfColumnIterations - 1)];
      RowElementIterator even(buffer + (1 << (numberOfColumnIterations - 1)));
      
      DestinationImage::Rows rowsLookup = destination->getRows();
      for (DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
           row != rowsLookup.getEnd();
           ++row) {
        unsigned int iteration = 1;
        RowElementIterator odd = row.getFirst();
        even[-1] = *odd++; // do not forget DC value
        while (iteration < numberOfColumnIterations) {
          odd = partialITransform(even, even, odd, 1 << iteration++);
        }
        odd = partialITransform(row.getEnd(), even, odd, 1 << iteration++);
      }
    }
  }
  
}; // end of gip namespace
