/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

// TAG: need cast to unsigned integral value

#include <gip/io/BMPEncoder.h>
#include <gip/transformation/TSRTransformation.h>
#include <gip/transformation/Convert.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>
#include <base/string/InvalidFormat.h>
#include <base/Integer.h>

using namespace gip;
using namespace base;

class RotateApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  RotateApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("Rotate"), numberOfArguments, arguments, environment) {
  }

  void transform(const Dimension& dimension, const String& inputFile, const String& outputFile) throw() {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;
    
//      GrayImage grayOriginalImage(originalImage.getDimension());
//      {
//        Convert<GrayImage, ColorImage, RGBToGray> transform(&grayOriginalImage, &originalImage, RGBToGray());
//        transform();
//      }

//      GrayImage finalImage(dimension);

    ColorImage finalImage(dimension);

    {
      TSRTransformation<ColorImage, ColorImage> transform(&finalImage, &originalImage);
      transform.identity();
      transform.translate(
        -static_cast<long double>(originalImage.getDimension().getWidth()/2),
        -static_cast<long double>(originalImage.getDimension().getHeight()/2)
      );
      transform.scale(1.5);
      transform.rotate(33.333 * constant::PI/180);
      transform.translate(
        finalImage.getDimension().getWidth()/2,
        finalImage.getDimension().getHeight()/2
      );
      fout << MESSAGE("Transforming image: ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }
    
    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.write(outputFile, &finalImage);
  }

  Dimension getDimension(const String& value) const throw() {
    // dimension format "123x123"
    int x = value.indexOf('x');
    assert(x > 0, base::InvalidFormat("Invalid dimension", this));
    String width = value.substring(0, x);
    String height = value.substring(x + 1);
    assert(width.isProper() && height.isProper(), base::InvalidFormat("Invalid dimension", this));
    return Dimension(
      static_cast<unsigned int>(Integer::parse(width, true)),
      static_cast<unsigned int>(Integer::parse(height, true))
    );
  }
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("http://www.mip.sdu.dk/~fonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>") << EOL << ENDL;

    String dimension;
    String inputFile;
    String outputFile;
    
    const Array<String> arguments = getArguments();
    switch (arguments.getSize()) {
    case 3:
      dimension = arguments[0];
      inputFile = arguments[1]; // the file name of the source image
      outputFile = arguments[2]; // the file name of the destination image
      break;
    default:
      fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" dimension input output") << ENDL;
      return; // stop
    }

    transform(getDimension(dimension), inputFile, outputFile);
  }
};

STUB(RotateApplication);
