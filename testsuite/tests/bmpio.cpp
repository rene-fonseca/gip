/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <base/string/FormatOutputStream.h>
#include <gip/Pixel.h>
#include <gip/io/BMPEncoder.h>

using namespace base;
using namespace gip;

#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <gip/Pixel.h>
#include <gip/io/GIFEncoder.h>

using namespace base;
using namespace gip;

class TestBMPEncoder : public Application {
public:

  TestBMPEncoder(int numberOfArguments, const char* arguments[], const char* environment[]) throw() :
          Application(MESSAGE("bmpio"), numberOfArguments, arguments, environment) {
  }

  static void main() throw() {
    fout << "Testing BMPEncoder..." << ENDL;
    
    String filename = MESSAGE("graphics\\image.bmp");

    Array<String> arguments = Application::getApplication()->getArguments();
    switch (arguments.getSize()) {
    case 0: // use default
      break;
    case 1:
      filename = arguments[0];
      break;
    default:
      fout << MESSAGE("usage: ") << Application::getApplication()->getName() << MESSAGE(" file") << ENDL;
      Application::getApplication()->setExitCode(EXIT_CODE_ERROR);
      return;
    }

    BMPEncoder readEncoder;
    
    fout << MESSAGE("Information:") << ENDL;
    readEncoder.getInfo(fout, filename);
    
    if (readEncoder.isValid(filename)) {
      fout << MESSAGE("Importing image with encoder: ") << readEncoder.getDescription() << ENDL;

      ColorImage* orig;
      try {
        orig = readEncoder.read(filename);
      } catch(gip::InvalidFormat& e) {
        ferr << MESSAGE("Invalid or unsupported BMP format") << ENDL;
        Application::getApplication()->setExitCode(EXIT_CODE_ERROR);
        return;
      }

      fout << MESSAGE("Width of image=") << orig->getWidth() << EOL
           << MESSAGE("Height of image=") << orig->getHeight() << ENDL;
      
      ColorPixel* current = orig->getElements();
      for (unsigned int y = 0; y < orig->getHeight(); ++y) {
        for (unsigned int x = 0; x < orig->getWidth(); ++x) {
          //++current;
        }
      }
      
      BMPEncoder writeEncoder;
      fout << MESSAGE("Exporting image with encoder: ") << writeEncoder.getDescription() << ENDL;
      writeEncoder.write(MESSAGE("output.bmp"), orig);
    } else {
      ferr << MESSAGE("File is not valid") << ENDL;
      Application::getApplication()->setExitCode(EXIT_CODE_ERROR);
      return;
    }
  }
};

int main(int argc, const char* argv[], const char* envp[]) {
  TestBMPEncoder application(argc, argv, envp);
  try {
    TestBMPEncoder::main();
  } catch(Exception& e) {
    return Application::getApplication()->exceptionHandler(e);
  } catch(...) {
    return Application::getApplication()->exceptionHandler();
  }
  return Application::EXIT_CODE_NORMAL;
}
