/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/BMPEncoder.h>
#include <gip/transformation/LinearScale.h>
#include <gip/transformation/FourierTransformation.h>
#include <gip/transformation/FourierExchange.h>
#include <gip/transformation/Convert.h>
#include <gip/operation/HeatColorMap.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>
#include <gip/Functor.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/string/StringOutputStream.h>
#include <base/Functor.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>

using namespace gip;
using namespace base;

class FourierToGray : public UnaryOperation<Complex, GrayPixel> {
private:

   long double scale;
public:

  inline FourierToGray(const Dimension& dimension) throw()
    : scale(1.0/dimension.getSize()) {
  }

  inline GrayPixel operator()(const Complex& value) const throw() {
    return clamp(
      0,
      static_cast<GrayPixel>(255 * Math::ln(1 + value.getModulus() * scale)),
      255
    );
  }
};

class FourierToLogModulus : public UnaryOperation<Complex, long double> {
private:

   long double scale;
   long double max;
public:

  inline FourierToLogModulus(const Dimension& dimension) throw()
     : scale(1.0/dimension.getSize()),
       max(0) {
  }

  inline long double operator()(const Complex& value) throw() {
    long double result = Math::ln(1 + value.getModulus() * scale);
    if (result > max) {
      max = result;
    }
    return result;
  }

  inline long double getMaximum() const throw() {
    return max;
  }
};

class MapToHue : public UnaryOperation<ColorPixel, long double> {
private:

  long double scale;
  HeatColorMap map;
public:

  MapToHue(long double _scale) throw() : scale(_scale) {
  }

  inline ColorPixel operator()(const long double& value) const throw() {
    RGBPixel<long double> temp = map(Math::sqrt(value * scale));
    ColorPixel result;
    result.red = static_cast<PixelTraits<ColorPixel>::Component>(PixelTraits<ColorPixel>::MAXIMUM * temp.red + 0.5);
    result.green = static_cast<PixelTraits<ColorPixel>::Component>(PixelTraits<ColorPixel>::MAXIMUM * temp.green + 0.5);
    result.blue = static_cast<PixelTraits<ColorPixel>::Component>(PixelTraits<ColorPixel>::MAXIMUM * temp.blue + 0.5);
    return result;
  }
};

class FourierApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  FourierApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("FourierTransformation"), numberOfArguments, arguments, environment) {
  }

  unsigned int getPowerOf2(unsigned int value) throw(OutOfDomain) {
    assert(value <= (1 << 31), OutOfDomain(this));
    unsigned int powerOf2 = 1 << 31;
    while (powerOf2 > value) {
      powerOf2 >>= 1;
    }
    if (powerOf2 < value) {
      powerOf2 <<= 1;
    }
    return powerOf2;
  }
  
  void fourierTransformation(const String& inputFile, const String& outputFile) throw() {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;
    
    Dimension dimension(getPowerOf2(originalImage.getDimension().getWidth()), getPowerOf2(originalImage.getDimension().getHeight()));
    
    ColorImage scaledImage(dimension);
    {
      LinearScale transform(&scaledImage, &originalImage);
      fout << MESSAGE("Scaling image: ") << originalImage.getDimension() << MESSAGE("->") << scaledImage.getDimension() << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    ComplexImage spaceImage(scaledImage.getDimension());
    {
      Convert<ComplexImage, ColorImage, RGBToFloat> transform(&spaceImage, &scaledImage, RGBToFloat());
      fout << MESSAGE("Converting image: ColorImage->ComplexImage") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

    ComplexImage fourierImage(spaceImage.getDimension());
    {
      FourierTransformation transform(&fourierImage, &spaceImage);
      fout << MESSAGE("Transforming image: Space->Fourier") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for Fourier transformation: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

//    GrayImage grayImage(fourierImage.getDimension());
//    {
//      Convert<GrayImage, ComplexImage, FourierToGray> transform(&grayImage, &fourierImage, FourierToGray(fourierImage.getDimension()));
//      fout << MESSAGE("Converting image: ComplexImage->GrayImage") << ' '
//           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//      transform();
//    }

    ArrayImage<long double> modulusImage(fourierImage.getDimension());
    long double maximumModulus;
    {
      Convert<ArrayImage<long double>, ComplexImage, FourierToLogModulus> transform(&modulusImage, &fourierImage, FourierToLogModulus(fourierImage.getDimension()));
      fout << MESSAGE("Converting image: ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
      maximumModulus = transform.getResult().getMaximum();
    }

    ColorImage finalImage(modulusImage.getDimension());
    MapToHue map(1.0/maximumModulus);
    fillWithUnary(finalImage, modulusImage, map);

    {
      FourierExchange<ColorImage> transform(&finalImage);
      fout << MESSAGE("Transforming image: ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.write(outputFile, &finalImage);
//    encoder.writeGray(outputFile, &grayImage);
  }
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("http://www.mip.sdu.dk/~fonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>") << EOL << ENDL;
    
    String inputFile;
    String outputFile;
    
    const Array<String> arguments = getArguments();
    switch (arguments.getSize()) {
    case 2:
      inputFile = arguments[0]; // the file name of the source image
      outputFile = arguments[1]; // the file name of the destination image
      break;
    default:
      fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" input output") << ENDL;
      return; // stop
    }
    
    fourierTransformation(inputFile, outputFile);
  }
};

STUB(FourierApplication);
