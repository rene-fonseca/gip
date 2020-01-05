/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/Histogram.h>
#include <gip/ImageException.h>

namespace gip {
  
  /**
    Dilation.
    
    @short Dilate
    @see Erode
    @ingroup transformations morphological
    @version 1.0
  */

  template<class KERNEL>
  class Dilate : public Transformation<GrayImage, GrayImage> {
  public:

    typedef GrayImage::Pixel Pixel;

    template<class PIXEL>
    class ApplyKernel {
    public:
      
      typedef typename SourceImage::ReadableRows::RowIterator::ElementIterator Iterator;
      
      inline Pixel operator()(Iterator previous, Iterator current, Iterator next) const noexcept {
        Pixel maximum;
        if (KERNEL::M00) {
          maximum = previous[-1];
        } else if (KERNEL::M01) {
          maximum = previous[0];
        } else if (KERNEL::M02) {
          maximum = previous[1];
        } else if (KERNEL::M10) {
          maximum = current[-1];
        } else if (KERNEL::M11) {
          maximum = current[0];
        } else if (KERNEL::M12) {
          maximum = current[1];
        } else if (KERNEL::M20) {
          maximum = next[-1];
        } else if (KERNEL::M21) {
          maximum = next[0];
        } else if (KERNEL::M22) {
          maximum = next[1];
        } else {
          maximum = PixelTraits<Pixel>::MINIMUM;
        }
        
        if (KERNEL::M00 && (maximum < previous[-1])) {
          maximum = previous[-1];
        }
        if (KERNEL::M01 && (maximum < previous[0])) {
          maximum = previous[0];
        }
        if (KERNEL::M02 && (maximum < previous[1])) {
          maximum = previous[1];
        }
        if (KERNEL::M10 && (maximum < current[-1])) {
          maximum = current[-1];
        }
        if (KERNEL::M11 && (maximum < current[0])) {
          maximum = current[0];
        }
        if (KERNEL::M12 && (maximum < current[1])) {
          maximum = current[1];
        }
        if (KERNEL::M20 && (maximum < next[-1])) {
          maximum = next[-1];
        }
        if (KERNEL::M21 && (maximum < next[0])) {
          maximum = next[0];
        }
        if (KERNEL::M22 && (maximum < next[1])) {
          maximum = next[1];
        }
        return maximum;
      }
    };
    
    /**
      Initializes the transformation.

      @param destination The destination image.
      @param source The source image.
    */
    Dilate(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<DestinationImage, SourceImage>(destination, source) {
      
      bassert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    void operator()() const noexcept {
      typename SourceImage::ReadableRows rowLookup = source->getRows();
      typename SourceImage::ReadableRows::RowIterator endRow = rowLookup.getEnd();
      typename SourceImage::ReadableRows::RowIterator previousRow = rowLookup.getFirst();
      typename SourceImage::ReadableRows::RowIterator currentRow = previousRow + 1;
      typename SourceImage::ReadableRows::RowIterator nextRow = currentRow + 1;
      
      typename DestinationImage::Rows::RowIterator destRow = destination->getRows().getFirst();
      
      // handle first row
      ++destRow;

      while (nextRow < endRow) {
        typename SourceImage::ReadableRows::RowIterator::ElementIterator previousRowColumn = previousRow.getFirst();
        typename SourceImage::ReadableRows::RowIterator::ElementIterator currentRowColumn = currentRow.getFirst();
        typename SourceImage::ReadableRows::RowIterator::ElementIterator nextRowColumn = nextRow.getFirst();
        typename SourceImage::ReadableRows::RowIterator::ElementIterator endCurrentRowColumn = currentRow.getEnd() - 1;
        typename DestinationImage::Rows::RowIterator::ElementIterator dest = destRow.getFirst();
        
        // first column
        ++previousRowColumn;
        ++currentRowColumn;
        ++nextRowColumn;
        ++dest;
        
        ApplyKernel<typename SourceImage::Pixel> applyKernel;
        while (currentRowColumn < endCurrentRowColumn) {
          *dest = applyKernel(previousRowColumn, currentRowColumn, nextRowColumn);
          ++dest;
          ++previousRowColumn;
          ++currentRowColumn;
          ++nextRowColumn;
        }
        
        // last column
        
        ++destRow;
        previousRow = currentRow;
        currentRow = nextRow;
        ++nextRow;
      }

      // handle last row
    }
    
  };

}; // end of gip namespace
