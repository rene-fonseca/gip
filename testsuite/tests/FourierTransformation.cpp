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
#include <gip/transformation/Convert.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>
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

  inline FourierToGray(const Dimension& dimension) throw() : scale(1.0/dimension.getSize()) {}

  inline GrayPixel operator()(const Complex& value) const throw() {
    long double temp = 0xff * log(1 + value.getModulus() * scale);
    if (temp < 0x00) {
      return 0x00;
    } else if (temp > 0xff) {
      return 0xff;
    } else {
      return static_cast<GrayPixel>(temp);
    }
  }
};

class FourierApplication : public Application {
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

    GrayImage grayImage(fourierImage.getDimension());
    {
      Convert<GrayImage, ComplexImage, FourierToGray> transform(&grayImage, &fourierImage, FourierToGray(fourierImage.getDimension()));
      fout << MESSAGE("Converting image: ComplexImage->GrayImage") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.writeGray(outputFile, &grayImage);
  }
  
  void main() throw() {
    fout << MESSAGE("FourierTransformation version 1.0") << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("http://www.mip.sdu.dk/~fonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>") << EOL << ENDL;
    
    String inputFile;
    String outputFile;
    
    const Array<String> arguments = getArguments();
    switch (arguments.getSize()) {
    case 2:
      inputFile = arguments[0]; // the file name of the source image
      outputFile = arguments[1]; // the file name of the destination image
      break;
    default:
      fout << MESSAGE("USAGE: ") << getFormalName() << MESSAGE(" input output") << ENDL;
      return; // stop
    }
    
    fourierTransformation(inputFile, outputFile);
  }
};

int main(int argc, const char* argv[], const char* env[]) {
  FourierApplication application(argc, argv, env);
  try {
    application.main();
  } catch(Exception& e) {
    return Application::getApplication()->exceptionHandler(e);
  } catch(...) {
    return Application::getApplication()->exceptionHandler();
  }
  return Application::getApplication()->getExitCode();
}