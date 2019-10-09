/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

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

using namespace com::azure::dev::gip;

class WalshToGray : public UnaryOperation<float, GrayPixel> {
private:

   long double scale = 0;
public:

  inline WalshToGray(const Dimension& dimension) throw()
    : scale(1.0/dimension.getSize()) {
  }

  inline GrayPixel operator()(const float& value) const throw() {
    long double temp = 0xff * Math::ln(1 + value * scale);
    if (temp < 0x00) {
      return 0x00;
    } else if (temp > 0xff) {
      return 0xff;
    } else {
      return static_cast<GrayPixel>(temp);
    }
  }
};

class WalshApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  WalshApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("WalshTransformation"), numberOfArguments, arguments, environment) {
  }

  void walshTransformation(const String& inputFile, const String& outputFile) throw() {    
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;
    
    Dimension dimension(
      Math::getPowerOf2(originalImage.getDimension().getWidth()),
      Math::getPowerOf2(originalImage.getDimension().getHeight())
    );
    
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
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for Walsh transformation: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    GrayImage grayImage(walshImage.getDimension());
    {
      Convert<GrayImage, FloatImage, WalshToGray> transform(&grayImage, &walshImage, WalshToGray(walshImage.getDimension()));
      fout << MESSAGE("Converting image: FloatImage->GrayImage") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.writeGray(outputFile, &grayImage);
  }
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("https://dev.azure.com/renefonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2001-2019 by Rene Moeller Fonseca") << EOL << ENDL;
    
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
    
    walshTransformation(inputFile, outputFile);
  }
};

STUB(WalshApplication);
