/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__CONVOLUTION_3X3_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__CONVOLUTION_3X3_H

#include <gip/gip.h>
#include <gip/transformation/Transformation.h>
#include <base/mem/Allocator.h>

namespace gip {

  class Kernel {
  public:
    enum {NORMALIZE = false};
  };

  class NormalizedKernel : public Kernel {
  public:
    enum {NORMALIZED = true};
  };
  
  class VerticalPrewitt : public Kernel {
  public:

    enum {
      M00 = -1, M01 = -1, M02 = -1,
      M10 = 0, M11 = 0, M12 = 0,
      M20 = 1, M21 = 1, M22 = 1
    };
  };

  class HorizontalPrewitt : public Kernel {
  public:

    enum {
      M00 = -1, M01 = 0, M02 = 1,
      M10 = -1, M11 = 0, M12 = 1,
      M20 = -1, M21 = 0, M22 = 1
    };
  };
  
  class VerticalSobel : public Kernel {
  public:

    enum {
      M00 = -1, M01 = -2, M02 = -1,
      M10 = 0, M11 = 0, M12 = 0,
      M20 = 1, M21 = 2, M22 = 1
    };
  };

  class HorizontalSobel : public NormalizedKernel {
  public:

    enum {
      M00 = -1, M01 = 0, M02 = 1,
      M10 = -2, M11 = 0, M12 = 2,
      M20 = -1, M21 = 0, M22 = 1
    };
  };

  class SmoothUniformRectangular3x3 : public NormalizedKernel {
  public:

    enum {
      M00 = 1, M01 = 1, M02 = 1,
      M10 = 1, M11 = 1, M12 = 1,
      M20 = 1, M21 = 1, M22 = 1
    };
  };
  
  class SmoothUniformRectangular5x5 : public NormalizedKernel {
  public:

    enum {
      M00 = 1, M01 = 1, M02 = 1, M03 = 1, M04 = 1,
      M10 = 1, M11 = 1, M12 = 1, M13 = 1, M14 = 1,
      M20 = 1, M21 = 1, M22 = 1, M23 = 1, M24 = 1,
      M30 = 1, M31 = 1, M32 = 1, M33 = 1, M34 = 1,
      M40 = 1, M41 = 1, M42 = 1, M43 = 1, M44 = 1
    };
  };

  class SmoothUniformCircular5x5 : public NormalizedKernel {
  public:

    enum {
      M00 = 0, M01 = 1, M02 = 1, M03 = 1, M04 = 0,
      M10 = 1, M11 = 1, M12 = 1, M13 = 1, M14 = 1,
      M20 = 1, M21 = 1, M22 = 1, M23 = 1, M24 = 1,
      M30 = 1, M31 = 1, M32 = 1, M33 = 1, M34 = 1,
      M40 = 0, M41 = 1, M42 = 1, M43 = 1, M44 = 0
    };
  };

  class SmoothPyramid5x5 : public NormalizedKernel {
  public:

    enum {
      M00 = 1, M01 = 2, M02 = 3, M03 = 2, M04 = 1,
      M10 = 2, M11 = 4, M12 = 6, M13 = 4, M14 = 2,
      M20 = 3, M21 = 6, M22 = 9, M23 = 6, M24 = 3,
      M30 = 2, M31 = 4, M32 = 6, M33 = 4, M34 = 2,
      M40 = 1, M41 = 2, M42 = 3, M43 = 2, M44 = 1
    };
  };

  class SmoothCone5x5 : public NormalizedKernel {
  public:

    enum {
      M00 = 0, M01 = 0, M02 = 1, M03 = 0, M04 = 0,
      M10 = 0, M11 = 2, M12 = 2, M13 = 2, M14 = 0,
      M20 = 1, M21 = 2, M22 = 5, M23 = 2, M24 = 1,
      M30 = 0, M31 = 2, M32 = 2, M33 = 2, M34 = 0,
      M40 = 0, M41 = 0, M42 = 1, M43 = 0, M44 = 0
    };
  };

  /**
    Convolution with 3x3 matrix.
    
    @short Convolution with 3x3 matrix
    @ingroup transformations filtering
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class DEST, class SRC, class KERNEL>
  class Convolution3x3 : public Transformation<DEST, SRC> {
  public:

    typedef typename Transformation<DEST, SRC>::DestinationImage DestinationImage;
    typedef typename Transformation<DEST, SRC>::SourceImage SourceImage;

    /**
      Initializes the transformation.

      @param destination The destination image.
      @param source The source image.
    */
    Convolution3x3(DestinationImage* destination, const SourceImage* source) throw(ImageException)
      : Transformation<DestinationImage, SourceImage>(destination, source) {
      
      assert(destination->getDimension() == source->getDimension(), ImageException(this));
    }
    
    template<class PIXEL>
    class ApplyKernel {
    public:
      
      typedef typename SourceImage::ReadableRows::RowIterator::ElementIterator Iterator;
      typedef typename PixelTraits<typename SourceImage::Pixel>::Arithmetic Arithmetic;
      typedef typename PixelTraits<typename DestinationImage::Pixel>::Arithmetic Result;
      
      inline Result operator()(Iterator previous, Iterator current, Iterator next) const throw() {
        return static_cast<Result>(KERNEL::M00 * static_cast<Arithmetic>(previous[-1]) +
                                   KERNEL::M01 * static_cast<Arithmetic>(previous[0]) +
                                   KERNEL::M02 * static_cast<Arithmetic>(previous[1]) +
                                   KERNEL::M10 * static_cast<Arithmetic>(current[-1]) +
                                   KERNEL::M11 * static_cast<Arithmetic>(current[0]) +
                                   KERNEL::M12 * static_cast<Arithmetic>(current[1]) +
                                   KERNEL::M20 * static_cast<Arithmetic>(next[-1]) +
                                   KERNEL::M21 * static_cast<Arithmetic>(next[0]) +
                                   KERNEL::M22 * static_cast<Arithmetic>(next[1]))/
          ((KERNEL::NORMALIZE) ? (static_cast<Arithmetic>(KERNEL::M00) +
                                  static_cast<Arithmetic>(KERNEL::M01) +
                                  static_cast<Arithmetic>(KERNEL::M02) +
                                  static_cast<Arithmetic>(KERNEL::M10) +
                                  static_cast<Arithmetic>(KERNEL::M11) +
                                  static_cast<Arithmetic>(KERNEL::M12) +
                                  static_cast<Arithmetic>(KERNEL::M20) +
                                  static_cast<Arithmetic>(KERNEL::M21) +
                                  static_cast<Arithmetic>(KERNEL::M22)) : 1);
      }
    };
    
    template<class COMPONENT>
    class ApplyKernel<GrayAlphaPixel<COMPONENT> > {
    public:
      
      typedef typename SourceImage::ReadableRows::RowIterator::ElementIterator Iterator;
      typedef typename PixelTraits<typename SourceImage::Pixel>::Arithmetic Arithmetic;
      typedef GrayAlphaPixel<typename PixelTraits<typename DestinationImage::Pixel>::Arithmetic> Result;

      inline Result operator()(Iterator previous, Iterator current, Iterator next) const throw() {
        Result result;
        result.gray = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].gray) +
                       KERNEL::M01 * static_cast<Arithmetic>(previous[0].gray) +
                       KERNEL::M02 * static_cast<Arithmetic>(previous[1].gray) +
                       KERNEL::M10 * static_cast<Arithmetic>(current[-1].gray) +
                       KERNEL::M11 * static_cast<Arithmetic>(current[0].gray) +
                       KERNEL::M12 * static_cast<Arithmetic>(current[1].gray) +
                       KERNEL::M20 * static_cast<Arithmetic>(next[-1].gray) +
                       KERNEL::M21 * static_cast<Arithmetic>(next[0].gray) +
                       KERNEL::M22 * static_cast<Arithmetic>(next[1].gray));
        
        result.alpha = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].alpha) +
                        KERNEL::M01 * static_cast<Arithmetic>(previous[0].alpha) +
                        KERNEL::M02 * static_cast<Arithmetic>(previous[1].alpha) +
                        KERNEL::M10 * static_cast<Arithmetic>(current[-1].alpha) +
                        KERNEL::M11 * static_cast<Arithmetic>(current[0].alpha) +
                        KERNEL::M12 * static_cast<Arithmetic>(current[1].alpha) +
                        KERNEL::M20 * static_cast<Arithmetic>(next[-1].alpha) +
                        KERNEL::M21 * static_cast<Arithmetic>(next[0].alpha) +
                        KERNEL::M22 * static_cast<Arithmetic>(next[1].alpha));
        
        return result/(KERNEL::NORMALIZE) ? (static_cast<Arithmetic>(KERNEL::M00) +
                                             static_cast<Arithmetic>(KERNEL::M01) +
                                             static_cast<Arithmetic>(KERNEL::M02) +
                                             static_cast<Arithmetic>(KERNEL::M10) +
                                             static_cast<Arithmetic>(KERNEL::M11) +
                                             static_cast<Arithmetic>(KERNEL::M12) +
                                             static_cast<Arithmetic>(KERNEL::M20) +
                                             static_cast<Arithmetic>(KERNEL::M21) +
                                             static_cast<Arithmetic>(KERNEL::M22)) : 1;
      }
    };

    template<class COMPONENT>
    class ApplyKernel<RGBPixel<COMPONENT> > {
    public:
      
      typedef typename SourceImage::ReadableRows::RowIterator::ElementIterator Iterator;
      typedef typename PixelTraits<typename SourceImage::Pixel>::Arithmetic Arithmetic;
      typedef RGBPixel<typename PixelTraits<typename DestinationImage::Pixel>::Arithmetic> Result;
      
      inline Result operator()(Iterator previous, Iterator current, Iterator next) const throw() {
        Result result;
        result.red = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].red) +
                      KERNEL::M01 * static_cast<Arithmetic>(previous[0].red) +
                      KERNEL::M02 * static_cast<Arithmetic>(previous[1].red) +
                      KERNEL::M10 * static_cast<Arithmetic>(current[-1].red) +
                      KERNEL::M11 * static_cast<Arithmetic>(current[0].red) +
                      KERNEL::M12 * static_cast<Arithmetic>(current[1].red) +
                      KERNEL::M20 * static_cast<Arithmetic>(next[-1].red) +
                      KERNEL::M21 * static_cast<Arithmetic>(next[0].red) +
                      KERNEL::M22 * static_cast<Arithmetic>(next[1].red));
        
        result.green = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].green) +
                        KERNEL::M01 * static_cast<Arithmetic>(previous[0].green) +
                        KERNEL::M02 * static_cast<Arithmetic>(previous[1].green) +
                        KERNEL::M10 * static_cast<Arithmetic>(current[-1].green) +
                        KERNEL::M11 * static_cast<Arithmetic>(current[0].green) +
                        KERNEL::M12 * static_cast<Arithmetic>(current[1].green) +
                        KERNEL::M20 * static_cast<Arithmetic>(next[-1].green) +
                        KERNEL::M21 * static_cast<Arithmetic>(next[0].green) +
                        KERNEL::M22 * static_cast<Arithmetic>(next[1].green));
        
        result.blue = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].blue) +
                       KERNEL::M01 * static_cast<Arithmetic>(previous[0].blue) +
                       KERNEL::M02 * static_cast<Arithmetic>(previous[1].blue) +
                       KERNEL::M10 * static_cast<Arithmetic>(current[-1].blue) +
                       KERNEL::M11 * static_cast<Arithmetic>(current[0].blue) +
                       KERNEL::M12 * static_cast<Arithmetic>(current[1].blue) +
                       KERNEL::M20 * static_cast<Arithmetic>(next[-1].blue) +
                       KERNEL::M21 * static_cast<Arithmetic>(next[0].blue) +
                       KERNEL::M22 * static_cast<Arithmetic>(next[1].blue));
        
        return result/((KERNEL::NORMALIZE) ? (static_cast<Arithmetic>(KERNEL::M00) +
                                              static_cast<Arithmetic>(KERNEL::M01) +
                                              static_cast<Arithmetic>(KERNEL::M02) +
                                              static_cast<Arithmetic>(KERNEL::M10) +
                                              static_cast<Arithmetic>(KERNEL::M11) +
                                              static_cast<Arithmetic>(KERNEL::M12) +
                                              static_cast<Arithmetic>(KERNEL::M20) +
                                              static_cast<Arithmetic>(KERNEL::M21) +
                                              static_cast<Arithmetic>(KERNEL::M22)) : 1);
      }
    };

    template<class COMPONENT>
    class ApplyKernel<RGBAPixel<COMPONENT> > {
    public:
      
      typedef typename SourceImage::ReadableRows::RowIterator::ElementIterator Iterator;
      typedef typename PixelTraits<typename SourceImage::Pixel>::Arithmetic Arithmetic;
      typedef RGBAPixel<typename PixelTraits<typename DestinationImage::Pixel>::Arithmetic> Result;
      
      inline Result operator()(Iterator previous, Iterator current, Iterator next) const throw() {
        Result result;
        result.red = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].red) +
                      KERNEL::M01 * static_cast<Arithmetic>(previous[0].red) +
                      KERNEL::M02 * static_cast<Arithmetic>(previous[1].red) +
                      KERNEL::M10 * static_cast<Arithmetic>(current[-1].red) +
                      KERNEL::M11 * static_cast<Arithmetic>(current[0].red) +
                      KERNEL::M12 * static_cast<Arithmetic>(current[1].red) +
                      KERNEL::M20 * static_cast<Arithmetic>(next[-1].red) +
                      KERNEL::M21 * static_cast<Arithmetic>(next[0].red) +
                      KERNEL::M22 * static_cast<Arithmetic>(next[1].red));
        
        result.green = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].green) +
                        KERNEL::M01 * static_cast<Arithmetic>(previous[0].green) +
                        KERNEL::M02 * static_cast<Arithmetic>(previous[1].green) +
                        KERNEL::M10 * static_cast<Arithmetic>(current[-1].green) +
                        KERNEL::M11 * static_cast<Arithmetic>(current[0].green) +
                        KERNEL::M12 * static_cast<Arithmetic>(current[1].green) +
                        KERNEL::M20 * static_cast<Arithmetic>(next[-1].green) +
                        KERNEL::M21 * static_cast<Arithmetic>(next[0].green) +
                        KERNEL::M22 * static_cast<Arithmetic>(next[1].green));
        
        result.blue = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].blue) +
                       KERNEL::M01 * static_cast<Arithmetic>(previous[0].blue) +
                       KERNEL::M02 * static_cast<Arithmetic>(previous[1].blue) +
                       KERNEL::M10 * static_cast<Arithmetic>(current[-1].blue) +
                       KERNEL::M11 * static_cast<Arithmetic>(current[0].blue) +
                       KERNEL::M12 * static_cast<Arithmetic>(current[1].blue) +
                       KERNEL::M20 * static_cast<Arithmetic>(next[-1].blue) +
                       KERNEL::M21 * static_cast<Arithmetic>(next[0].blue) +
                       KERNEL::M22 * static_cast<Arithmetic>(next[1].blue));
        
        result.alpha = (KERNEL::M00 * static_cast<Arithmetic>(previous[-1].alpha) +
                        KERNEL::M01 * static_cast<Arithmetic>(previous[0].alpha) +
                        KERNEL::M02 * static_cast<Arithmetic>(previous[1].alpha) +
                        KERNEL::M10 * static_cast<Arithmetic>(current[-1].alpha) +
                        KERNEL::M11 * static_cast<Arithmetic>(current[0].alpha) +
                        KERNEL::M12 * static_cast<Arithmetic>(current[1].alpha) +
                        KERNEL::M20 * static_cast<Arithmetic>(next[-1].alpha) +
                        KERNEL::M21 * static_cast<Arithmetic>(next[0].alpha) +
                        KERNEL::M22 * static_cast<Arithmetic>(next[1].alpha));
        
        return result/(KERNEL::NORMALIZE) ? (static_cast<Arithmetic>(KERNEL::M00) +
                                             static_cast<Arithmetic>(KERNEL::M01) +
                                             static_cast<Arithmetic>(KERNEL::M02) +
                                             static_cast<Arithmetic>(KERNEL::M10) +
                                             static_cast<Arithmetic>(KERNEL::M11) +
                                             static_cast<Arithmetic>(KERNEL::M12) +
                                             static_cast<Arithmetic>(KERNEL::M20) +
                                             static_cast<Arithmetic>(KERNEL::M21) +
                                             static_cast<Arithmetic>(KERNEL::M22)) : 1;
      }
    };
    
    /**
      Calculate transformation.
    */
    void operator()() throw() {
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
