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

namespace gip {

// hint: HINT_BEST, HINT_NICE, HINT_FAST, HINT_FASTEST

  /**
    Blend operator.
    
    @short Blend operator.
    @ingroup operator
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  template<class PIXEL>
  class Blend : public BinaryOperation<PIXEL, PIXEL, PIXEL> {
  public:
  };
  
  template<>
  class Blend<float> : public BinaryOperation<float, float, float> {
  private:
    
    const long double opacity;
    const long double transparency;
  public:
    
    inline Blend(const long double _opacity, const long double _opaque) throw()
      : opacity(_opacity/_opaque),
        transparency(1 - _opacity/_opaque) {
    }
    
    inline float operator()(const float front, const float back) const throw() {
      return opacity * front + transparency * back;
    }
  };

  template<>
  class Blend<double> : public BinaryOperation<double, double, double> {
  private:
    
    const long double opacity;
    const long double transparency;
  public:
    
    inline Blend(const long double _opacity, const long double _opaque) throw()
      : opacity(_opacity/_opaque),
        transparency(1 - _opacity/_opaque) {
    }
    
    inline double operator()(const double front, const double back) const throw() {
      return opacity * front + transparency * back;
    }
  };

  template<>
  class Blend<long double> : public BinaryOperation<long double, long double, long double> {
  private:

    const long double opacity;
    const long double transparency;
  public:

    inline Blend(const long double _opacity, const long double _opaque) throw()
      : opacity(_opacity/_opaque),
        transparency(1 - _opacity/_opaque) {
    }
    
    inline long double operator()(const long double front, const long double back) const throw() {
      return opacity * front + transparency * back;
    }
  };

  template<class COMPONENT>
  class Blend<RGBPixel<COMPONENT> > : public BinaryOperation<RGBPixel<COMPONENT>, RGBPixel<COMPONENT>, RGBPixel<COMPONENT> > {
  private:

    typedef RGBPixel<COMPONENT> Pixel;
    typedef typename PixelTraits<Pixel>::Arithmetic Arithmetic;
    const Arithmetic opacity;
    const Arithmetic transparency;
    const Arithmetic opaque;
  public:
    
    inline Blend(const Arithmetic _opacity, const Arithmetic _opaque) throw()
      : opacity(_opacity),
        transparency(_opaque - _opacity),
        opaque(_opaque) {
    }
    
    inline Pixel operator()(const Pixel front, const Pixel back) const throw() {
      Pixel result;
      result.red = (transparency * static_cast<Arithmetic>(back.red) +
                    opacity * static_cast<Arithmetic>(front.red))/opaque;
      result.green = (transparency * static_cast<Arithmetic>(back.green) +
                      opacity * static_cast<Arithmetic>(front.green))/opaque;
      result.blue = (transparency * static_cast<Arithmetic>(back.blue) +
                     opacity * static_cast<Arithmetic>(front.blue))/opaque;
      return result;
    }
  };

  template<>
  class Blend<ColorPixel> : public BinaryOperation<ColorPixel, ColorPixel, ColorPixel> {
  private:

    typedef typename PixelTraits<ColorPixel>::Arithmetic Arithmetic;
    const Arithmetic opacity;
    const Arithmetic transparency;
    const Arithmetic opaque;
    const Arithmetic doubleOpaque;
  public:
    
    inline Blend(const Arithmetic _opacity, const Arithmetic _opaque) throw()
      : opacity(2 * _opacity),
        transparency(2 * _opaque - 2 * _opacity),
        opaque(_opaque),
        doubleOpaque(2 * _opaque) {
    }
    
    inline ColorPixel operator()(const ColorPixel front, const ColorPixel back) const throw() {
      ColorPixel result;
      result.red = (transparency * static_cast<Arithmetic>(back.red) +
                    opacity * static_cast<Arithmetic>(front.red) +
                    opaque)/doubleOpaque;
      result.green = (transparency * static_cast<Arithmetic>(back.green) +
                      opacity * static_cast<Arithmetic>(front.green) +
                      opaque)/doubleOpaque;
      result.blue = (transparency * static_cast<Arithmetic>(back.blue) +
                     opacity * static_cast<Arithmetic>(front.blue) +
                     opaque)/doubleOpaque;
      return result;
    }
  };
  
  template<>
  class Blend<RGBPixel<float> > : public BinaryOperation<RGBPixel<float>, RGBPixel<float>, RGBPixel<flaot> > {
  private:

    typedef RGBPixel<float> Pixel;
    typedef typename PixelTraits<Pixel>::Arithmetic Arithmetic;
    const Arithmetic opacity;
    const Arithmetic transparency;
  public:
    
    inline Blend(const Arithmetic _opacity, const Arithmetic _opaque) throw()
      : opacity(_opacity/_opaque),
        transparency(1 - _opacity/_opaque) {
    }
    
    inline Pixel operator()(const Pixel front, const Pixel back) const throw() {
      Pixel result;
      result.red = transparency * back.red + opacity * front.red;
      result.green = transparency * back.green + opacity * front.green;
      result.blue = transparency * back.blue + opacity * front.blue;
      return result;
    }
  };
  
  template<>
  class Blend<RGBPixel<double> > : public BinaryOperation<RGBPixel<double>, RGBPixel<double>, RGBPixel<double> > {
  private:

    typedef RGBPixel<double> Pixel;
    typedef typename PixelTraits<Pixel>::Arithmetic Arithmetic;
    const Arithmetic opacity;
    const Arithmetic transparency;
  public:
    
    inline Blend(const Arithmetic _opacity, const Arithmetic _opaque) throw()
      : opacity(_opacity/_opaque),
        transparency(1 - _opacity/_opaque) {
    }
    
    inline Pixel operator()(const Pixel front, const Pixel back) const throw() {
      Pixel result;
      result.red = transparency * back.red + opacity * front.red;
      result.green = transparency * back.green + opacity * front.green;
      result.blue = transparency * back.blue + opacity * front.blue;
      return result;
    }
  };

  template<>
  class Blend<RGBPixel<long double> > : public BinaryOperation<RGBPixel<long double>, RGBPixel<long double>, RGBPixel<long double> > {
  private:

    typedef RGBPixel<long double> Pixel;
    typedef typename PixelTraits<Pixel>::Arithmetic Arithmetic;
    const Arithmetic opacity;
    const Arithmetic transparency;
  public:
    
    inline Blend(const Arithmetic _opacity, const Arithmetic _opaque) throw()
      : opacity(_opacity/_opaque),
        transparency(1 - _opacity/_opaque) {
    }
    
    inline Pixel operator()(const Pixel front, const Pixel back) const throw() {
      Pixel result;
      result.red = transparency * back.red + opacity * front.red;
      result.green = transparency * back.green + opacity * front.green;
      result.blue = transparency * back.blue + opacity * front.blue;
      return result;
    }
  };
  
}; // end of gip namespace
