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
#include <gip/transformation/Mirror.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>

using namespace com::azure::dev::gip;

class MirrorApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  MirrorApplication() throw()
    : Application(MESSAGE("Mirror")) {
  }

  void mirrorTransformation(const String& inputFile, const String& outputFile) throw() {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;

    {
      Mirror<ColorImage> transform(&originalImage);
      fout << MESSAGE("Transforming image: ") << ' ' << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for mirror: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.write(outputFile, &originalImage);
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
    
    mirrorTransformation(inputFile, outputFile);
  }
};

APPLICATION_STUB(MirrorApplication);
