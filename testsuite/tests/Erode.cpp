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
#include <gip/transformation/Erode.h>
#include <gip/transformation/Convert.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>

using namespace gip;
using namespace base;

class ErodeApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  class Kernel {
  public:

    enum {
      M00 = true, M01 = true, M02 = true,
      M10 = true, M11 = true, M12 = true,
      M20 = true, M21 = true, M22 = true
    };
  };

  ErodeApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("Erode"), numberOfArguments, arguments, environment) {
  }

  void erodeTransformation(const String& inputFile, const String& outputFile) throw() {
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

    GrayImage finalImage(grayOriginalImage.getDimension());
    {
      Erode3x3<GrayImage, GrayImage, Kernel> transform(&finalImage, &grayOriginalImage);
      fout << MESSAGE("Transforming image: ") << ' ' << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for erosion: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.writeGray(outputFile, &finalImage);
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
    
    erodeTransformation(inputFile, outputFile);
  }
};

STUB(ErodeApplication);
