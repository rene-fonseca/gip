/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__MEDIAN_FILTER_3X3_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__MEDIAN_FILTER_3X3_H

#include <gip/gip.h>
#include <gip/transformation/Transformation.h>
#include <base/mem/Allocator.h>

namespace gip {

  /**
    Median filter with 3x3 window. The median filter is commonly used to remove
    salt and pepper noise from an image.
    
    @short Median filter with 3x3 window
    @ingroup transformations filtering
    @author Rene Moeller Fonseca
    @version 1.0
  */
  
  class MedianFilter3x3 : public Transformation<GrayImage, GrayImage> {
  private:
    
    typedef PixelTraits<SourceImage::Pixel>::Component Component;

    struct Elements2 {
      Component left;
      Component right;
    };

    struct Elements {
      Component left;
      Component middle;
      Component right;
    };
    
    static inline Elements sort(Component left, Component middle, Component right) throw() {
      if (middle <= right) {
        if (left <= middle) {
          return (Elements){left, middle, right};
        } else {
          return (Elements){middle, (left <= right) ? left : right, (left <= right) ? right : left};
        }
      } else if (left <= right) {
        return (Elements){left, right, middle};
      } else {
        return (Elements){right, (left <= middle) ? left : middle, (left <= middle) ? middle : left};
      }
    }

    static inline Elements2 sort(Component left, Component right) throw() {
      if (left <= right) {
        return (Elements2){left, right};
      } else {
        return (Elements2){right, left};
      }
    }
    
    static inline Component getMedian4(Component a, Component b, Component c, Component d) throw() {
      Elements2 left0 = sort(a, b);
      Elements2 right0 = sort(c, d);
      Elements2 left1 = sort(left0.left, right0.left);
      Elements2 right1 = sort(left0.right, right0.right);
      return sort(left1.right, right1.left).left;
    }
    
    static inline Component getMedian6(Component a, Component b, Component c, Component d, Component e, Component f) throw() {
      Elements left0 = sort(a, b, c);
      Elements right0 = sort(d, e, f);
      return sort(
        maximum(left0.left, right0.left),
        minimum(left0.middle, right0.middle),
        minimum(left0.right, right0.right)
      ).middle;
    }

    static inline Component getMedian9(Component a, Component b, Component c, Component d, Component e, Component f, Component g, Component h, Component i) throw() {
      Elements left0 = sort(a, b, c);
      Elements middle0 = sort(d, e, f);
      Elements right0 = sort(g, h, i);
      return sort(
        maximum(left0.left, middle0.left, right0.left),
        sort(left0.middle, middle0.middle, right0.middle).middle,
        minimum(left0.right, middle0.right, right0.right)
      ).middle;
    }
  public:
    
    /**
      Initializes the median transformation.
      
      @param destination The destination image.
      @param source The source image.
    */
    MedianFilter3x3(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<DestinationImage, SourceImage>(destination, source) {
      
      assert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    /**
      Calculate transformation.
    */
    void operator()() throw() {
      /*typename*/ SourceImage::ReadableRows rowLookup = source->getRows();
      /*typename*/ SourceImage::ReadableRows::RowIterator endRow = rowLookup.getEnd();
      /*typename*/ SourceImage::ReadableRows::RowIterator currentRow = rowLookup.getFirst();
      /*typename*/ SourceImage::ReadableRows::RowIterator nextRow = currentRow + 1;
      
      /*typename*/ DestinationImage::Rows::RowIterator destRow = destination->getRows().getFirst();
      
      // handle first row
      {
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator currentRowColumn = currentRow.getFirst();
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator nextRowColumn = nextRow.getFirst();
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator endCurrentRowColumn = currentRow.getEnd() - 1;
        /*typename*/ DestinationImage::Rows::RowIterator::ElementIterator dest = destRow.getFirst();
        
        // handle left corner
        *dest++ = getMedian4(currentRowColumn[0], currentRowColumn[1], nextRowColumn[0], nextRowColumn[1]);
        ++currentRowColumn;
        ++nextRowColumn;
        
        while (currentRowColumn < endCurrentRowColumn) {
          *dest++ = getMedian6(
            currentRowColumn[-1], currentRowColumn[0], currentRowColumn[1],
            nextRowColumn[-1], nextRowColumn[0], nextRowColumn[1]
          );
          ++currentRowColumn;
          ++nextRowColumn;
        }
        
        // handle right corner
        *dest++ = getMedian4(currentRowColumn[-1], currentRowColumn[0], nextRowColumn[-1], nextRowColumn[0]);        
      }
      
      /*typename*/ SourceImage::ReadableRows::RowIterator previousRow = currentRow;
      currentRow = nextRow;
      ++nextRow;
      ++destRow;
      
      while (nextRow < endRow) {
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator previousRowColumn = previousRow.getFirst();
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator currentRowColumn = currentRow.getFirst();
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator nextRowColumn = nextRow.getFirst();
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator endCurrentRowColumn = currentRow.getEnd() - 1;
        /*typename*/ DestinationImage::Rows::RowIterator::ElementIterator dest = destRow.getFirst();
        
        // first column
        *dest++ = getMedian6(
          previousRowColumn[0], previousRowColumn[1], currentRowColumn[0], currentRowColumn[1], nextRowColumn[0], nextRowColumn[1]
        );
        ++previousRowColumn;
        ++currentRowColumn;
        ++nextRowColumn;
        
        while (currentRowColumn < endCurrentRowColumn) {
          *dest++ = getMedian9(
            previousRowColumn[-1], previousRowColumn[0], previousRowColumn[1],
            currentRowColumn[-1], currentRowColumn[0], currentRowColumn[1],
            nextRowColumn[-1], nextRowColumn[0], nextRowColumn[1]
          );
          ++previousRowColumn;
          ++currentRowColumn;
          ++nextRowColumn;
        }
        
        // last column
        *dest++ = getMedian6(
          previousRowColumn[-1], previousRowColumn[0], currentRowColumn[-1], currentRowColumn[0], nextRowColumn[-1], nextRowColumn[0]
        );
        
        previousRow = currentRow;
        currentRow = nextRow;
        ++nextRow;
        ++destRow;
      }
      
      // handle second row
      {
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator previousRowColumn = previousRow.getFirst();
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator currentRowColumn = currentRow.getFirst();
        /*typename*/ SourceImage::ReadableRows::RowIterator::ElementIterator endCurrentRowColumn = currentRow.getEnd() - 1;
        /*typename*/ DestinationImage::Rows::RowIterator::ElementIterator dest = destRow.getFirst();
        
        // handle left corner
        *dest++ = getMedian4(previousRowColumn[0], previousRowColumn[1], currentRowColumn[0], currentRowColumn[1]);
        ++previousRowColumn;
        ++currentRowColumn;
        
        while (currentRowColumn < endCurrentRowColumn) {
          *dest++ = getMedian6(
            previousRowColumn[-1], previousRowColumn[0], previousRowColumn[1],
            currentRowColumn[-1], currentRowColumn[0], currentRowColumn[1]
          );
          ++previousRowColumn;
          ++currentRowColumn;
        }
        
        // handle right corner
        *dest++ = getMedian4(previousRowColumn[0], previousRowColumn[1], currentRowColumn[-1], currentRowColumn[0]);
      }
    }
  };
  
}; // end of gip namespace

#endif
