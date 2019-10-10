/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/transformation/Scale.h>

namespace gip {
  
  template _COM_AZURE_DEV__BASE__API class Scale<GrayImage, GrayImage>;
  template _COM_AZURE_DEV__BASE__API class Scale<ColorImage, ColorImage>;
  template _COM_AZURE_DEV__BASE__API class Scale<ColorAlphaImage, ColorAlphaImage>;
  
  template<class DEST, class SRC>
  Scale<DEST, SRC>::Scale(DestinationImage* destination, const SourceImage* source) throw(ImageException)
    : Transformation<DEST, SRC>(destination, source) {
    bassert(source->getDimension().isProper(), ImageException("Unable to scale image", this));
  }
  
  template<class DEST, class SRC>
  void Scale<DEST, SRC>::operator()() throw() {    
    unsigned int rows = Transformation<DEST, SRC>::destination->getDimension().getHeight();
    unsigned int columns = Transformation<DEST, SRC>::destination->getDimension().getWidth();
    unsigned int srcRows = Transformation<DEST, SRC>::source->getDimension().getHeight();
    unsigned int srcColumns = Transformation<DEST, SRC>::source->getDimension().getWidth();
    long double rowRatio = static_cast<long double>(srcRows)/rows;
    long double columnRatio = static_cast<long double>(srcColumns)/columns;
    
    typename DestinationImage::Rows rowsLookup = Transformation<DEST, SRC>::destination->getRows();
    typename SourceImage::ReadableRows srcRowsLookup = Transformation<DEST, SRC>::source->getRows();
    
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
