/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <gip/Pixel.h>
#include <gip/io/BMPEncoder.h>

using namespace com::azure::dev::gip;

class BMPEncoderApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  BMPEncoderApplication() noexcept
    : Application(MESSAGE("bmpio")) {
  }

  void main() noexcept {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL << ENDL;
    
    String filename = MESSAGE("graphics\\image.bmp");

    Array<String> arguments = getArguments();
    switch (arguments.getSize()) {
    case 0: // use default
      break;
    case 1:
      filename = arguments[0];
      break;
    default:
      fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" file") << ENDL;
      setExitCode(EXIT_CODE_ERROR);
      return;
    }

    BMPEncoder readEncoder;
    
    fout << MESSAGE("Information:") << ENDL;
    ArrayMap<String, AnyValue> information = readEncoder.getInformation(filename);
    fout << information << ENDL;
    
    if (readEncoder.isValid(filename)) {
      fout << MESSAGE("Importing image with encoder: ") << readEncoder.getDescription() << ENDL;
      
      ColorImage* orig;
      try {
        orig = readEncoder.read(filename);
      } catch (gip::InvalidFormat&) {
        ferr << MESSAGE("Invalid or unsupported BMP format") << ENDL;
        setExitCode(EXIT_CODE_ERROR);
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
      setExitCode(EXIT_CODE_ERROR);
      return;
    }
  }
};

APPLICATION_STUB(BMPEncoderApplication);
