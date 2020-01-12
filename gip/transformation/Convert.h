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
#include <base/math/Math.h>

namespace gip {

  class RGBToFloat : public UnaryOperation<ColorPixel, float> {
  public:

    inline RGBToFloat() noexcept {
    }

    inline Result operator()(const Argument& value) const noexcept {
      return float((static_cast<unsigned int>(value.blue) + value.green + value.red)/3);
    }
  };

  class RGBToGray : public UnaryOperation<ColorPixel, GrayPixel> {
  public:

    inline RGBToGray() noexcept {
    }

    inline Result operator()(const Argument& value) const noexcept {
      unsigned int temp = static_cast<unsigned int>(value.blue) + value.green + value.red;
      if (temp % 3 == 2) {
        ++temp;
      }
      return GrayPixel(temp/3);
    }
  };

  class FloatToGray : public UnaryOperation<float, GrayPixel> {
  public:

    inline FloatToGray() noexcept {
    }

    inline Result operator()(const Argument& value) const noexcept {
      return GrayPixel(static_cast<unsigned char>(value));
    }
  };

  class FloatToGrayWithScale : public UnaryOperation<float, GrayPixel> {
  private:

    double scale = 0;
  public:

    inline FloatToGrayWithScale(double _scale) noexcept : scale(_scale) {
    }

    inline GrayPixel operator()(const float& value) const noexcept {
      double temp = scale * value;
      if (temp < 0x00) {
        return 0x00;
      } else if (temp > 0xff) {
        return 0xff;
      } else {
        return static_cast<GrayPixel>(temp);
      }
    }
  };

  class GrayToFloat : public UnaryOperation<GrayPixel, float> {
  public:

    inline GrayToFloat() noexcept {
    }

    inline float operator()(const GrayPixel& value) const noexcept {
      return value;
    }
  };

  class RGBToComplex : public UnaryOperation<ColorPixel, Complex<float> > {
  private:

    double scale = 0;
  public:

    inline RGBToComplex(double _scale) noexcept : scale(_scale) {
    }

    inline Complex<float> operator()(const ColorPixel& value) const noexcept {
      RGBToGray opr;
      return Complex<float>(scale * opr(value), 0);
    }
  };

  class ComplexToRGB : public UnaryOperation<Complex<float>, ColorPixel> {
  private:

    const double scale = 0;
  public:

    inline ComplexToRGB(double _scale) noexcept : scale(_scale) {
    }

    inline Result operator()(const Argument& value) const noexcept
    {
      double temp = scale * value.getReal();
      if (temp < 0x00) {
        return makeColorPixel(0x00, 0x00, 0x00);
      } else if (temp > 0xff) {
        return makeColorPixel(0xff, 0xff, 0xff);
      } else {
        ConvertPixel<ColorPixel, unsigned char> convertPixel;
        return convertPixel(static_cast<unsigned char>(temp));
      }
    }
  };

  class ComplexToRGBImaginary : public UnaryOperation<Complex<float>, ColorPixel> {
  private:

    double scale = 0;
  public:

    inline ComplexToRGBImaginary(double _scale) noexcept
      : scale(_scale)
    {
    }

    inline Result operator()(const Argument& value) const noexcept
    {
      double temp = scale * value.getImaginary();
      if (temp < 0x00) {
        return makeColorPixel(0x00, 0x00, 0x00);
      } else if (temp > 0xff) {
        return makeColorPixel(0xff, 0xff, 0xff);
      } else {
        ConvertPixel<ColorPixel, unsigned char> convertPixel;
        return convertPixel(static_cast<unsigned char>(temp));
      }
    }
  };

  class ComplexToRGBSqrModulus : public UnaryOperation<Complex<float>, ColorPixel> {
  private:

    double scale = 0;
  public:

    inline ComplexToRGBSqrModulus(double _scale) noexcept
      : scale(_scale)
    {
    }

    inline Result operator()(const Argument& value) const noexcept
    {
      double temp = scale * value.getSqrModulus();
      if (temp < 0x00) {
        return makeColorPixel(0x00, 0x00, 0x00);
      } else if (temp > 0xff) {
        return makeColorPixel(0xff, 0xff, 0xff);
      } else {
        ConvertPixel<ColorPixel, unsigned char> convertPixel;
        return convertPixel(static_cast<unsigned char>(temp));
      }
    }
  };

  class ComplexToRGBModulus : public UnaryOperation<Complex<float>, ColorPixel> {
  private:

    double scale = 0;
  public:

    inline ComplexToRGBModulus(double _scale) noexcept
      : scale(_scale)
    {
    }

    inline Result operator()(const Argument& value) const noexcept
    {
      double temp = scale * value.getModulus();
      if (temp < 0x00) {
        return makeColorPixel(0x00, 0x00, 0x00);
      } else if (temp > 0xff) {
        return makeColorPixel(0xff, 0xff, 0xff);
      } else {
        ConvertPixel<ColorPixel, unsigned char> convertPixel;
        return convertPixel(static_cast<unsigned char>(temp));
      }
    }
  };

  class ComplexToRGBLogModulus : public UnaryOperation<Complex<float>, ColorPixel> {
  private:

    double scale = 0;
  public:

    inline ComplexToRGBLogModulus(double _scale) noexcept
      : scale(_scale)
    {
    }

    inline Result operator()(const Argument& value) const noexcept
    {
      double temp = scale * Math::ln(1 + value.getModulus());
      if (temp < 0x00) {
        return makeColorPixel(0x00, 0x00, 0x00);
      } else if (temp > 0xff) {
        return makeColorPixel(0xff, 0xff, 0xff);
      } else {
        ConvertPixel<ColorPixel, unsigned char> convertPixel;
        return convertPixel(static_cast<unsigned char>(temp));
      }
    }
  };

  /**
    Duplicates the contents of an image.

    @short Image convertion
    @ingroup transformation
    @version 1.0
  */
  template<class DEST, class SRC, class UNARY>
  class Convert : public Transformation<DEST, SRC> {
  private:

    UNARY convert;
  public:

    typedef typename Transformation<DEST, SRC>::DestinationImage DestinationImage;
    typedef typename Transformation<DEST, SRC>::SourceImage SourceImage;
    
    /**
      Initializes convert transformation.

      @param destination The destination image.
      @param source The source image.
      @param convert The convertion operation.
    */
    Convert(DestinationImage* destination, const SourceImage* source, const UNARY& _convert)
      : Transformation<DestinationImage, SourceImage>(destination, source), convert(_convert)
    {
      bassert(
        destination->getDimension() == source->getDimension(),
        Exception("Images must have identical dimensions", this)
      );
    }

    /**
      Duplicates the contents of the source image to the destination image.
    */
    void operator()() noexcept
    {
      fillWithUnary(*Transformation<DEST, SRC>::destination, *Transformation<DEST, SRC>::source, convert);
    }

    UNARY getResult() const noexcept
    {
      return convert;
    }
  };

}; // end of gip namespace
