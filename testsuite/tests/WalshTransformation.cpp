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
#include <gip/transformation/WalshTransformation.h>
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

// class WalshToGray : public UnaryOperation<float, GrayPixel> {
// private:

//   long double scale;
// public:

//   inline WalshToGray(const Dimension& dimension) throw() : scale(1./dimension.getSize()) {}

//   inline Result operator()(const Argument& value) {
//     long double temp = 0xff * log(1 + value * scale);
//     if (temp < 0x00) {
//       return 0x00;
//     } else if (temp > 0xff) {
//       return 0xff;
//     } else {
//       return GrayPixel(static_cast<unsigned char>(temp));
//     }
//   }
// };

class GIPApplication : public Application {
public:

  GIPApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("WalshTransformation"), numberOfArguments, arguments, environment) {
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
  
  void walshTransformation(const String& inputFile, const String& outputFile) throw() {    
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
    FloatImage spaceImage(scaledImage.getDimension());
    {
      Convert<FloatImage, ColorImage, RGBToFloat> transform(&spaceImage, &scaledImage, RGBToFloat());
      fout << MESSAGE("Converting image: ColorImage->FloatImage") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    
    FloatImage walshImage(spaceImage.getDimension());
    {
      WalshTransformation transform(&walshImage, &spaceImage);
      fout << MESSAGE("Transforming image: Space->Walsh") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    
    GrayImage grayImage(walshImage.getDimension());
    {
      long double scale = 1./walshImage.getDimension().getSize();
      Convert<GrayImage, FloatImage, FloatToGrayWithScale> transform(&grayImage, &spaceImage, FloatToGrayWithScale(scale));
      transform();
    }
    
    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.writeGray(outputFile, &grayImage);
  }
  
  void main() throw() {
    fout << MESSAGE("WalshTransformation version 1.0") << EOL
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
    
    Timer timer;
    walshTransformation(inputFile, outputFile);
    
    fout << MESSAGE("Time elapsed: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
  }
};

int main(int argc, const char* argv[], const char* env[]) {
  GIPApplication application(argc, argv, env);
  try {
    application.main();
  } catch(Exception& e) {
    return Application::getApplication()->exceptionHandler(e);
  } catch(...) {
    return Application::getApplication()->exceptionHandler();
  }
  return Application::getApplication()->getExitCode();
}
