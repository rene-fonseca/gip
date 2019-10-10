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

#include <gip/RGBPixel.h>
#include <base/Cast.h>

namespace gip {

  /**
    Interpolate operator.
    
    @short Interpolate operator.
    @ingroup operator
    @version 1.0
  */
  
  template<class PIXEL>
  class Interpolate : public BinaryOperation<PIXEL, PIXEL, PIXEL> {
  private:
    
    typedef PIXEL Pixel;
    const Pixel* elements = nullptr;
    Dimension dimension;
  public:
    
    inline Interpolate(const ArrayImage<Pixel>& image) throw()
      : elements(image.getElements()), dimension(image.getDimension()) {
    }
    
    inline double operator()(double x, double y) const throw() {
      const int x0 = static_cast<int>(Math::floor(x));
      const int y0 = static_cast<int>(Math::floor(y));
      
      const double xFraction = x - x0;
      const double yFraction = y - y0;
      
      double result = 0; // 0 is background

      const Pixel* p = elements + y0 * dimension.getWidth() + x0;
      
      if ((x0 >= 0) && (x0 < dimension.getWidth())) {
        if ((y0 >= 0) && (y0 < dimension.getHeight())) {
          const double w0 = (1 - xFraction) * (1 - yFraction);
          result += w0 * p[0];
        }
        if (((y0 + 1) >= 0) && ((y0 + 1) < dimension.getHeight())) {
          const double w2 = (1 - xFraction) * yFraction;
          result += w2 * p[dimension.getWidth()];
        }
      }
      
      ++p;
      
      if (((x0 + 1) >= 0) && ((x0 + 1) < dimension.getWidth())) {
        if ((y0 >= 0) && (y0 < dimension.getHeight())) {
          const double w1 = xFraction * (1 - yFraction);
          result += w1 * p[0];
        }
        if (((y0 + 1) >= 0) && ((y0 + 1) < dimension.getHeight())) {
          const double w3 = xFraction * yFraction;
          result += w3 * p[dimension.getWidth()];
        }
      }
      
      return result;
    }
  };
  
  template<>
  class Interpolate<GrayPixel> : BinaryOperation<long double, long double, long double> {
  private:
    
    typedef GrayPixel Pixel;
    const Pixel* elements = nullptr;
    Dimension dimension;
  public:
    
    inline Interpolate(const ArrayImage<Pixel>& image) throw()
      : elements(image.getElements()), dimension(image.getDimension()) {
    }
    
    inline long double operator()(long double x, long double y) const throw() {
      const int x0 = static_cast<int>(Math::floor(x));
      const int y0 = static_cast<int>(Math::floor(y));
      
      const long double xFraction = x - x0;
      const long double yFraction = y - y0;
      
      long double result = 0; // 0 is background

      const Pixel* p = elements + y0 * dimension.getWidth() + x0;
      
      if ((x0 >= 0) && (static_cast<unsigned int>(x0) < dimension.getWidth())) {
        if ((y0 >= 0) && (static_cast<unsigned int>(y0) < dimension.getHeight())) {
          const long double w0 = (1 - xFraction) * (1 - yFraction);
          result += w0 * p[0];
        }
        if (((y0 + 1) >= 0) && (static_cast<unsigned int>(y0 + 1) < dimension.getHeight())) {
          const long double w2 = (1 - xFraction) * yFraction;
          result += w2 * p[dimension.getWidth()];
        }
      }
      
      ++p;
      
      if (((x0 + 1) >= 0) && (static_cast<unsigned int>(x0 + 1) < dimension.getWidth())) {
        if ((y0 >= 0) && (static_cast<unsigned int>(y0) < dimension.getHeight())) {
          const long double w1 = xFraction * (1 - yFraction);
          result += w1 * p[0];
        }
        if (((y0 + 1) >= 0) && (static_cast<unsigned int>(y0 + 1) < dimension.getHeight())) {
          const long double w3 = xFraction * yFraction;
          result += w3 * p[dimension.getWidth()];
        }
      }
      
      return result;
    }
  };
  
  template<class COMPONENT>
  class Interpolate<RGBPixel<COMPONENT> > : BinaryOperation<RGBPixel<long double>, long double, long double> {
  private:
    
    typedef RGBPixel<COMPONENT> Pixel;
    const Pixel* elements = nullptr;
    Dimension dimension;
  public:
    
    inline Interpolate(const ArrayImage<Pixel>& image) throw()
      : elements(image.getElements()), dimension(image.getDimension()) {
    }
    
    inline RGBPixel<COMPONENT> operator()(long double x, long double y) const throw() {
      const int x0 = static_cast<int>(Math::floor(x));
      const int y0 = static_cast<int>(Math::floor(y));
      
      const long double xFraction = x - x0;
      const long double yFraction = y - y0;
      
      long double red = 0; // 0 is background
      long double green = 0;
      long double blue = 0;
      
      const Pixel* p = elements + y0 * dimension.getWidth() + x0;
      
      if ((x0 >= 0) && (x0 < dimension.getWidth())) {
        if ((y0 >= 0) && (y0 < dimension.getHeight())) {
          const long double w0 = (1 - xFraction) * (1 - yFraction);
          red += w0 * p[0].red;
          green += w0 * p[0].green;
          blue += w0 * p[0].blue;
        }
        if (((y0 + 1) >= 0) && ((y0 + 1) < dimension.getHeight())) {
          const long double w2 = (1 - xFraction) * yFraction;
          red += w2 * p[dimension.getWidth()].red;
          green += w2 * p[dimension.getWidth()].green;
          blue += w2 * p[dimension.getWidth()].blue;
        }
      }
      
      ++p;
      
      if (((x0 + 1) >= 0) && ((x0 + 1) < dimension.getWidth())) {
        if ((y0 >= 0) && (y0 < dimension.getHeight())) {
          const long double w1 = xFraction * (1 - yFraction);
          red += w1 * p[0].red;
          green += w1 * p[0].green;
          blue += w1 * p[0].blue;
        }
        if (((y0 + 1) >= 0) && ((y0 + 1) < dimension.getHeight())) {
          const long double w3 = xFraction * yFraction;
          red += w3 * p[dimension.getWidth()].red;
          green += w3 * p[dimension.getWidth()].green;
          blue += w3 * p[dimension.getWidth()].blue;
        }
      }

      return makeRGBPixel<COMPONENT>(
        static_cast<COMPONENT>(red),
        static_cast<COMPONENT>(green),
        static_cast<COMPONENT>(blue)
      );
    }
  };
  
}; // end of gip namespace
