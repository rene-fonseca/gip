/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

// TAG: need cast to unsigned integral value

#include <gip/io/BMPEncoder.h>
#include <gip/transformation/LinearScale.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>
#include <base/string/InvalidFormat.h>
#include <base/Integer.h>

using namespace com::azure::dev::gip;

class ScaleApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  ScaleApplication() throw()
    : Application(MESSAGE("Scale")) {
  }

  void scaleTransformation(const Dimension& dimension, const String& inputFile, const String& outputFile) throw() {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;
    
    ColorImage finalImage(dimension);
    
    {
      LinearScale transform(&finalImage, &originalImage);
      fout << MESSAGE("Transforming image: ") << ' ' << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for scale: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.write(outputFile, &finalImage);
  }

  Dimension getDimension(const String& value) const throw() {
    // dimension format "123x123"
    int x = value.indexOf('x');
    bassert(x > 0, base::InvalidFormat("Invalid dimension", this));
    String width = value.substring(0, x);
    String height = value.substring(x + 1);
    bassert(width.isProper() && height.isProper(), base::InvalidFormat("Invalid dimension", this));
    return Dimension(
      static_cast<unsigned int>(Integer::parse(width, true)),
      static_cast<unsigned int>(Integer::parse(height, true))
    );
  }
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("https://dev.azure.com/renefonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002-2019 by Rene Moeller Fonseca") << EOL << ENDL;

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

    scaleTransformation(getDimension(dimension), inputFile, outputFile);
  }
};

APPLICATION_STUB(ScaleApplication);
