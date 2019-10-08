/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/Pixel.h>
#include <gip/io/RASEncoder.h>
#include <gip/io/BMPEncoder.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>

using namespace dk::sdu::mip::gip;

class RASEncoderApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  RASEncoderApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("RASEncoder"), numberOfArguments, arguments, environment) {
  }

  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("http://www.mip.sdu.dk/~fonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002-2019 by Rene Moeller Fonseca") << EOL << ENDL;
    
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

    RASEncoder readEncoder;
    
    fout << MESSAGE("Information:") << ENDL;
    HashTable<String, AnyValue> information = readEncoder.getInformation(inputFile);
    fout << information << ENDL;
    
    if (readEncoder.isValid(inputFile)) {
      fout << MESSAGE("Importing image with encoder: ") << readEncoder.getDescription() << ENDL;

      ColorImage* orig;
      try {
        orig = readEncoder.read(inputFile);
      } catch (gip::InvalidFormat& e) {
        ferr << MESSAGE("Invalid or unsupported format") << ENDL;
        setExitCode(EXIT_CODE_ERROR);
        return;
      }

      fout << MESSAGE("Width of image=") << orig->getWidth() << EOL
           << MESSAGE("Height of image=") << orig->getHeight() << ENDL;

      BMPEncoder writeEncoder;
      fout << MESSAGE("Exporting image with encoder: ") << writeEncoder.getDescription() << ENDL;
      writeEncoder.write(outputFile, orig);
    } else {
      ferr << MESSAGE("File is not valid") << ENDL;
      setExitCode(EXIT_CODE_ERROR);
      return;
    }
  }
};

STUB(RASEncoderApplication);
