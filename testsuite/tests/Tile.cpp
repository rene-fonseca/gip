/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/BMPEncoder.h>
#include <gip/transformation/Tile.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>
#include <base/string/InvalidFormat.h>
#include <base/Integer.h>

using namespace gip;
using namespace base;

class TileApplication : public Application {
public:

  TileApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("Tile"), numberOfArguments, arguments, environment) {
  }

  void tileTransformation(const Dimension& dimension, const String& inputFile, const String& outputFile) throw() {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;
    
    ColorImage finalImage(dimension);
    
    {
      Tile<ColorImage, ColorImage> transform(&finalImage, &originalImage);
      fout << MESSAGE("Transforming image: ") << ' ' << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for tile: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
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
    fout << MESSAGE("Tile version 1.0") << EOL
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
      fout << MESSAGE("USAGE: ") << getFormalName() << MESSAGE(" dimension input output") << ENDL;
      return; // stop
    }

    tileTransformation(getDimension(dimension), inputFile, outputFile);
  }
};

int main(int argc, const char* argv[], const char* env[]) {
  TileApplication application(argc, argv, env);
  try {
    application.main();
  } catch(Exception& e) {
    return Application::getApplication()->exceptionHandler(e);
  } catch(...) {
    return Application::getApplication()->exceptionHandler();
  }
  return Application::getApplication()->getExitCode();
}