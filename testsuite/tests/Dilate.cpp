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
#include <gip/transformation/Dilate.h>
#include <gip/transformation/Convert.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>

using namespace com::azure::dev::gip;

class DilateApplication : public Application {
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

  DilateApplication() noexcept
    : Application(MESSAGE("Dilate")) {
  }

  void dilateTransformation(const String& inputFile, const String& outputFile) noexcept {
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
      Dilate</*GrayImage, GrayImage,*/ Kernel> transform(&finalImage, &grayOriginalImage);
      fout << MESSAGE("Transforming image: ") << ' ' << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for dilation: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.writeGray(outputFile, &finalImage);
  }
  
  void main() noexcept {
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
    
    dilateTransformation(inputFile, outputFile);
  }
};

APPLICATION_STUB(DilateApplication);
