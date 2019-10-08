/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/ImageException.h>

namespace gip {

  /**
    Bresenham scale.

    @short Bresenham scale transformation.
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class DEST, class SRC>
  class BresenhamScale : public Transformation<DEST, SRC> {
  public:

    typedef typename Transformation<DEST, SRC>::DestinationImage DestinationImage;
    typedef typename Transformation<DEST, SRC>::SourceImage SourceImage;
    
    /**
      Initializes scale transformation.
      
      @param destination The destination image.
      @param source The source image.
    */
    BresenhamScale(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<DEST, SRC>(destination, source) {
      bassert(source->getDimension().isProper(), ImageException("Unable to scale image", this));  
      bassert(
        (destination->getWidth() <= source->getWidth()) && (destination->getHeight() <= source->getHeight()),
        ImageException("Unable to scale image", this)
      );
    }
    
    /**
      Scale the source image to the destination image.
    */
    void operator()() throw() {
      const unsigned int destWidth = destination->getDimension().getWidth();
      const unsigned int destHeight = destination->getDimension().getHeight();
      const unsigned int srcWidth = source->getDimension().getWidth();
      const unsigned int srcHeight = source->getDimension().getHeight();
      const long double scale = 1.0/(srcHeight * srcWidth);
      
      typename DestinationImage::Rows::RowIterator destRow = destination->getRows().getFirst();
      typename SourceImage::ReadableRows srcRowsLookup = source->getRows();
      typename SourceImage::ReadableRows::RowIterator srcRow = srcRowsLookup.getFirst();
      const typename SourceImage::ReadableRows::RowIterator srcEndRow = srcRowsLookup.getEnd();
      
      //Allocator<long double> buffer(destWidth);
      long double buffer[destWidth];
      fill<long double>(buffer, destWidth, 0);
      
      unsigned int y = 0;
      unsigned int rowModulus = destHeight;
      
      while (srcRow != srcEndRow) {
        typename SourceImage::ReadableRows::RowIterator::ElementIterator srcColumn = srcRow.getFirst();
        const typename SourceImage::ReadableRows::RowIterator::ElementIterator srcEndColumn = srcRow.getEnd();
        
        long double* currentRow = buffer;
        long double integrate = 0;
        unsigned int columnModulus = destWidth;
        
        if (rowModulus < srcHeight) {
          while (srcColumn != srcEndColumn) {
            if (columnModulus < srcWidth) {
              integrate += destWidth * *srcColumn;
            } else {
              columnModulus -= srcWidth;
              integrate += (destWidth - columnModulus) * *srcColumn;
              *currentRow++ += destHeight * integrate;
              integrate = columnModulus * *srcColumn;
            }
            columnModulus += destWidth;
            ++srcColumn;
          }
        } else {
          rowModulus -= srcHeight;
          const unsigned int rowWeight = destHeight - rowModulus;
          typename DestinationImage::Rows::RowIterator::ElementIterator destColumn = destRow.getFirst();
          ++destRow;
          while (srcColumn != srcEndColumn) {
            if (columnModulus < srcWidth) {
              integrate += destWidth * *srcColumn;
            } else {
              columnModulus -= srcWidth;
              integrate += (destWidth - columnModulus) * *srcColumn;
              *currentRow += rowWeight * integrate;
              *destColumn++ = static_cast<typename DestinationImage::Pixel>(scale * *currentRow + 0.5); // TAG: round to nearest
              *currentRow++ = rowModulus * integrate; // next row
              integrate = columnModulus * *srcColumn;
            }
            columnModulus += destWidth;
            ++srcColumn;
          }
        }
        rowModulus += destHeight;
        ++srcRow;
      }
    }
    
  };

}; // end of gip namespace
