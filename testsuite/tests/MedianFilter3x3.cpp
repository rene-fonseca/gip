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
#include <gip/transformation/MedianFilter3x3.h>
#include <gip/transformation/Convert.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>

using namespace gip;
using namespace base;

class MedianApplication : public Application {
public:

  MedianApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("MedianFilter3x3"), numberOfArguments, arguments, environment) {
  }

  void medianTransformation(const String& inputFile, const String& outputFile) throw() {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;

    GrayImage grayOriginalImage(originalImage.getDimension());
    {
      Convert<GrayImage, ColorImage, RGBToGray> transform(&grayOriginalImage, &originalImage, RGBToGray());
      fout << MESSAGE("Converting image: ColorImage->GrayImage") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

    GrayImage medianImage(grayOriginalImage.getDimension());
    {
      MedianFilter3x3 transform(&medianImage, &grayOriginalImage);
      fout << MESSAGE("Transforming image: ") << ' ' << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for Median filter transformation: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.writeGray(outputFile, &medianImage);
  }
  
  void main() throw() {
    fout << MESSAGE("MedianFilter3x3 version 1.0") << EOL
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
    
    medianTransformation(inputFile, outputFile);
  }
};

int main(int argc, const char* argv[], const char* env[]) {
  MedianApplication application(argc, argv, env);
  try {
    application.main();
  } catch(Exception& e) {
    return Application::getApplication()->exceptionHandler(e);
  } catch(...) {
    return Application::getApplication()->exceptionHandler();
  }
  return Application::getApplication()->getExitCode();
}