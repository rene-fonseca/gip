/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__CONVERT_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__CONVERT_H

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>

namespace gip {

class RGBToFloat : public UnaryOperation<ColorPixel, FloatPixel> {
public:

  inline RGBToFloat() throw() {}

  inline Result operator()(const Argument& value) const throw() {
    return FloatPixel((static_cast<unsigned int>(value.blue) + value.green + value.red)/3);
  }
};

class RGBToGray : public UnaryOperation<ColorPixel, GrayPixel> {
public:

  inline RGBToGray() throw() {}

  inline Result operator()(const Argument& value) const throw() {
    unsigned int temp = static_cast<unsigned int>(value.blue) + value.green + value.red;
    if (temp % 3 == 2) {
      ++temp;
    }
    return GrayPixel(temp/3);
  }
};

class FloatToGray : public UnaryOperation<FloatPixel, GrayPixel> {
public:

  inline FloatToGray() throw() {}

  inline Result operator()(const Argument& value) const throw() {
    return GrayPixel(static_cast<Intensity>(value));
  }
};

class FloatToGrayWithScale : public UnaryOperation<FloatPixel, GrayPixel> {
private:

  long double scale;
public:

  inline FloatToGrayWithScale(long double s) throw() : scale(s) {}

  inline Result operator()(const Argument& value) const throw() {
    long double temp = scale * value;
    if (temp < MINIMUM_INTENSITY) {
      return MINIMUM_INTENSITY;
    } else if (temp > MAXIMUM_INTENSITY) {
      return MAXIMUM_INTENSITY;
    } else {
      return GrayPixel(static_cast<Intensity>(temp));
    }
  }
};

class GrayToFloat : public UnaryOperation<GrayPixel, FloatPixel> {
public:

  inline GrayToFloat() throw() {}

  inline Result operator()(const Argument& value) const throw() {
    return FloatPixel(value);
  }
};

class RGBToComplex : public UnaryOperation<ColorPixel, ComplexPixel> {
private:

  long double scale;
public:

  inline RGBToComplex(long double _scale) throw() : scale(_scale) {}

  inline Result operator()(const Argument& value) const throw() {
    RGBToGray opr;
    return ComplexPixel(scale * opr(value), 0);
  }
};

class ComplexToRGB : public UnaryOperation<ComplexPixel, RGBPixel> {
private:

  const long double scale;
public:

  inline ComplexToRGB(long double _scale) throw() : scale(_scale) {}

  inline Result operator()(const Argument& value) const throw() {
    long double temp = scale * value.getReal();
    if (temp < 0) {
      return RGB_PIXEL_BLACK;
    } else if (temp > MAXIMUM_INTENSITY) {
      return RGB_PIXEL_WHITE;
    } else {
      ConvertPixel<RGBPixel, Intensity> convertPixel;
      return convertPixel(static_cast<Intensity>(temp));
    }
  }
};

class ComplexToRGBImaginary : public UnaryOperation<ComplexPixel, RGBPixel> {
private:

  long double scale;
public:

  inline ComplexToRGBImaginary(long double _scale) throw() : scale(_scale) {}

  inline Result operator()(const Argument& value) const throw() {
    long double temp = scale * value.getImaginary();
    if (temp < 0) {
      return RGB_PIXEL_BLACK;
    } else if (temp > MAXIMUM_INTENSITY) {
      return RGB_PIXEL_WHITE;
    } else {
      ConvertPixel<RGBPixel, Intensity> convertPixel;
      return convertPixel(static_cast<Intensity>(temp));
    }
  }
};

class ComplexToRGBSqrModulus : public UnaryOperation<ComplexPixel, RGBPixel> {
private:

  long double scale;
public:

  inline ComplexToRGBSqrModulus(long double _scale) throw() : scale(_scale) {}

  inline Result operator()(const Argument& value) const throw() {
    long double temp = scale * value.getSqrModulus();
    if (temp < 0) {
      return RGB_PIXEL_BLACK;
    } else if (temp > MAXIMUM_INTENSITY) {
      return RGB_PIXEL_WHITE;
    } else {
      ConvertPixel<RGBPixel, Intensity> convertPixel;
      return convertPixel(static_cast<Intensity>(temp));
    }
  }
};

class ComplexToRGBModulus : public UnaryOperation<ComplexPixel, RGBPixel> {
private:

  long double scale;
public:

  inline ComplexToRGBModulus(long double _scale) throw() : scale(_scale) {}

  inline Result operator()(const Argument& value) const throw() {
    long double temp = scale * value.getModulus();
    if (temp < 0) {
      return RGB_PIXEL_BLACK;
    } else if (temp > MAXIMUM_INTENSITY) {
      return RGB_PIXEL_WHITE;
    } else {
      ConvertPixel<RGBPixel, Intensity> convertPixel;
      return convertPixel(static_cast<Intensity>(temp));
    }
  }
};

class ComplexToRGBLogModulus : public UnaryOperation<ComplexPixel, RGBPixel> {
private:

  long double scale;
public:

  inline ComplexToRGBLogModulus(double _scale) throw() : scale(_scale) {}

  inline Result operator()(const Argument& value) const throw() {
    long double temp = scale * log(1 + value.getModulus());
    if (temp < 0) {
      return RGB_PIXEL_BLACK;
    } else if (temp > MAXIMUM_INTENSITY) {
      return RGB_PIXEL_WHITE;
    } else {
      ConvertPixel<RGBPixel, Intensity> convertPixel;
      return convertPixel(static_cast<Intensity>(temp));
    }
  }
};

/**
  Duplicates the contents of an image.

  @author Rene Moeller Fonseca
*/
template<class DEST, class SRC, class UNARY>
class Convert : public Transformation<DEST, SRC> {
private:

  UNARY convert;
public:

  /**
    Initializes duplication object.

    @param destination The destination image.
    @param source The source image.
  */
  Convert(DestinationImage* destination, const SourceImage* source, const UNARY& opr) throw() :
    Transformation<DestinationImage, SourceImage>(destination, source), convert(opr) {
    assert(
      destination->getDimension() == source->getDimension(),
      Exception("Images must have identical dimensions")
    );
  }

  /**
    Duplicates the contents of the source image to the destination image.
  */
  void operator()() throw() {
    fillWithUnary(*destination, *source, convert);
  }
};

}; // end of namespace

#endif
