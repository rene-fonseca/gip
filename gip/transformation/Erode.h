/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__ERODE_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__ERODE_H

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/Histogram.h>
#include <gip/ImageException.h>

namespace gip {
  
  /**
    Erosion is a morphological operation.
    
    @short Erode
    @see Dilate
    @ingroup transformations
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */
  
  template<class DEST, class SRC, class KERNEL>
  class Erode3x3 : public Transformation<DEST, SRC> {
  private:

    class TopLeftKernel : public KERNEL {
    public:
      enum {M00 = false, M01 = false, M02 = false, M10 = false, M20 = false};
    };

    class TopRightKernel : public KERNEL {
    public:
      enum {M00 = false, M01 = false, M02 = false, M12 = false, M22 = false};
    };

    class TopKernel : public KERNEL {
    public:
      enum {M00 = false, M01 = false, M02 = false};
    };

    class BottomKernel : public KERNEL {
    public:
      enum {M20 = false, M21 = false, M22 = false};
    };

    class LeftKernel : public KERNEL {
    public:
      enum {M00 = false, M10 = false, M20 = false};
    };

    class RightKernel : public KERNEL {
    public:
      enum {M02 = false, M12 = false, M22 = false};
    };
  public:
    
    typedef GrayImage::Pixel Pixel;
    
    template<class PIXEL>
    class ApplyKernel {
    public:
      
      typedef typename SourceImage::ReadableRows::RowIterator::ElementIterator Iterator;
      
      inline Pixel operator()(Iterator previous, Iterator current, Iterator next) const throw() {
        Pixel minimum;
        if (KERNEL::M00) {
          minimum = previous[-1];
        } else if (KERNEL::M01) {
          minimum = previous[0];
        } else if (KERNEL::M02) {
          minimum = previous[1];
        } else if (KERNEL::M10) {
          minimum = current[-1];
        } else if (KERNEL::M11) {
          minimum = current[0];
        } else if (KERNEL::M12) {
          minimum = current[1];
        } else if (KERNEL::M20) {
          minimum = next[-1];
        } else if (KERNEL::M21) {
          minimum = next[0];
        } else if (KERNEL::M22) {
          minimum = next[1];
        } else {
          minimum = PixelTraits<Pixel>::MINIMUM;
        }
        
        if (KERNEL::M00 && (minimum > previous[-1])) {
          minimum = previous[-1];
        }
        if (KERNEL::M01 && (minimum > previous[0])) {
          minimum = previous[0];
        }
        if (KERNEL::M02 && (minimum > previous[1])) {
          minimum = previous[1];
        }
        if (KERNEL::M10 && (minimum > current[-1])) {
          minimum = current[-1];
        }
        if (KERNEL::M11 && (minimum > current[0])) {
          minimum = current[0];
        }
        if (KERNEL::M12 && (minimum > current[1])) {
          minimum = current[1];
        }
        if (KERNEL::M20 && (minimum > next[-1])) {
          minimum = next[-1];
        }
        if (KERNEL::M21 && (minimum > next[0])) {
          minimum = next[0];
        }
        if (KERNEL::M22 && (minimum > next[1])) {
          minimum = next[1];
        }
        return minimum;
      }
    };

    /**
      Initializes the transformation.

      @param destination The destination image.
      @param source The source image.
    */
    Erode3x3(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<DestinationImage, SourceImage>(destination, source) {
      
      assert(destination->getDimension() == source->getDimension(), ImageException(this));
    }

    void operator()() const throw() {
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

#endif
