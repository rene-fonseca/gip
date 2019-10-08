/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Scale.h>

namespace gip {
  
  template Scale<GrayImage, GrayImage>;
  template Scale<ColorImage, ColorImage>;
  template Scale<ColorAlphaImage, ColorAlphaImage>;
  
  template<class DEST, class SRC>
  Scale<DEST, SRC>::Scale(DestinationImage* destination, const SourceImage* source) throw(ImageException)
    : Transformation<DEST, SRC>(destination, source) {
    bassert(source->getDimension().isProper(), ImageException("Unable to scale image", this));
  }
  
  template<class DEST, class SRC>
  void Scale<DEST, SRC>::operator()() throw() {    
    unsigned int rows = destination->getDimension().getHeight();
    unsigned int columns = destination->getDimension().getWidth();
    unsigned int srcRows = source->getDimension().getHeight();
    unsigned int srcColumns = source->getDimension().getWidth();
    long double rowRatio = static_cast<long double>(srcRows)/rows;
    long double columnRatio = static_cast<long double>(srcColumns)/columns;
    
    typename DestinationImage::Rows rowsLookup = destination->getRows();
    typename SourceImage::ReadableRows srcRowsLookup = source->getRows();
    
    typename DestinationImage::Rows::RowIterator row = rowsLookup.getFirst();
    for (unsigned int rowIndex = 0; rowIndex < rows; ++rowIndex) {
      typename SourceImage::ReadableRows::RowIterator srcRow = srcRowsLookup[static_cast<unsigned int>(rowIndex * rowRatio)];
      
      typename DestinationImage::Rows::RowIterator::ElementIterator column = row.getFirst();
      for (unsigned int columnIndex = 0; columnIndex < columns; ++columnIndex) {
        *column = srcRow[static_cast<unsigned int>(columnIndex * columnRatio)];
        ++column;
      }
      ++row;
    }
  }
  
}; // end of gip namespace
