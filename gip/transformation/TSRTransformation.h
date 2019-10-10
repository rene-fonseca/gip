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
#include <gip/operation/Interpolate.h>

namespace gip {

  /**
    Translation, scaling, and rotation transformation.
    
    @short Translation, scaling, and rotation transformation
    @version 1.0
  */

  template<class DEST, class SRC>
  class TSRTransformation : public Transformation<DEST, SRC> {
  };
  
  template<class PIXEL>
  class TSRTransformation<ArrayImage<PIXEL>, ArrayImage<PIXEL> > : public Transformation<ArrayImage<PIXEL>, ArrayImage<PIXEL> > {
  private:

    typedef PIXEL Pixel;
    typedef ArrayImage<Pixel> DestinationImage;
    typedef ArrayImage<Pixel> SourceImage;
    
    /** The homogeneous transformation matrix. */
    double matrix[2][3];
  public:
    
    TSRTransformation(DestinationImage* destination, const SourceImage* source) throw()
      : Transformation<DestinationImage, SourceImage>(destination, source) {
      // TAG: check dimension of images
      identity();
    }
    
    void load(double _matrix[2][3]) throw() {
      matrix[0][0] = _matrix[0][0];
      matrix[0][1] = _matrix[0][1];
      matrix[0][2] = _matrix[0][2];
      matrix[1][0] = _matrix[1][0];
      matrix[1][1] = _matrix[1][1];
      matrix[1][2] = _matrix[1][2];
    }
  
    void identity() throw() {
      matrix[0][0] = 1;
      matrix[0][1] = 0;
      matrix[0][2] = 0;
      matrix[1][0] = 0;
      matrix[1][1] = 1;
      matrix[1][2] = 0;
    }
  
    void rotate(long double alpha) throw() {
      const double cos = Math::cos(alpha);
      const double sin = Math::sin(alpha);

      double temp[2][3] = {
        {
          matrix[0][0] * cos - matrix[1][0] * sin,
          matrix[0][1] * cos - matrix[1][1] * sin,
          matrix[0][2] * cos - matrix[1][2] * sin
        },
        {
          matrix[1][0] * cos + matrix[0][0] * sin,
          matrix[1][1] * cos + matrix[0][1] * sin,
          matrix[1][2] * cos + matrix[0][2] * sin
        }
      };
    
      matrix[0][0] = temp[0][0];
      matrix[0][1] = temp[0][1];
      matrix[0][2] = temp[0][2];
      matrix[1][0] = temp[1][0];
      matrix[1][1] = temp[1][1];
      matrix[1][2] = temp[1][2];
    }

    void scale(double value) throw() {
      matrix[0][0] *= value;
      matrix[0][1] *= value;
      matrix[0][2] *= value;
      matrix[1][0] *= value;
      matrix[1][1] *= value;
      matrix[1][2] *= value;
    }

    void translate(double dx, double dy) throw() {
      matrix[0][2] += dx;
      matrix[1][2] += dy;
    }

    void operator()() throw() {
      const unsigned int height = Transformation<DestinationImage, SourceImage>::destination->getDimension().getHeight();
      const unsigned int width = Transformation<DestinationImage, SourceImage>::destination->getDimension().getWidth();
      Pixel* dest = Transformation<DestinationImage, SourceImage>::destination->getElements();
      Interpolate<Pixel> interpolate(*Transformation<DestinationImage, SourceImage>::source);
    
      // inverse of matrix (only x and y row)
      double inverse[2][3];
      const double factor = 1/(matrix[0][0] * matrix[1][1] - matrix[0][1] * matrix[1][0]);
    
      inverse[0][0] = matrix[1][1] * factor;
      inverse[0][1] = -matrix[0][1] * factor;
      inverse[0][2] = (matrix[0][1] * matrix[1][2] - matrix[0][2] * matrix[1][1]) * factor;
      inverse[1][0] = -matrix[1][0] * factor;
      inverse[1][1] = matrix[0][0] * factor;
      inverse[1][2] = (matrix[0][2] * matrix[1][0] - matrix[0][0] * matrix[1][2]) * factor;
    
      for (unsigned int y = 0; y < height; ++y) {
        double srcX = inverse[0][1] * y + inverse[0][2];
        double srcY = inverse[1][1] * y + inverse[1][2];
        for (unsigned int x = 0; x < width; ++x) {
          srcX += inverse[0][0];
          srcY += inverse[1][0];
          *dest++ = static_cast<Pixel>(interpolate(srcX, srcY)); // TAG: round to nearest
        }
      }
    }

  };

}; // end of gip namespace
