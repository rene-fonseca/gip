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
#include <gip/transformation/ContrastStretch.h>
#include <gip/transformation/Convert.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>

using namespace com::azure::dev::gip;

class ContrastStretchApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  ContrastStretchApplication() noexcept
    : Application(MESSAGE("ContrastStretch")) {
  }

  void contrastStretchTransformation(const String& inputFile, const String& outputFile) noexcept {
    BMPEncoder encoder;

    fout << MESSAGE("Information:") << ENDL;
    ArrayMap<String, AnyValue> information = encoder.getInformation(inputFile);
    fout << information << ENDL;

    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;

//    GrayImage grayOriginalImage(originalImage.getDimension());
//    {
//      Convert<GrayImage, ColorImage, RGBToGray> transform(&grayOriginalImage, &originalImage, RGBToGray());
//      fout << MESSAGE("Converting image: ColorImage->GrayImage") << ' '
//           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//      transform();
//    }
//
//    GrayImage finalImage(grayOriginalImage.getDimension());
    ColorImage finalImage(originalImage.getDimension());
    {
//      ContrastStretch<GrayImage, GrayImage> transform(&finalImage, &grayOriginalImage);
      ContrastStretch<ColorImage, ColorImage> transform(&finalImage, &originalImage);
      fout << MESSAGE("Transforming image: ") << ' ' << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for ContrastStretch transformation: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.write(outputFile, &finalImage);
//    encoder.writeGray(outputFile, &finalImage);
  }
  
  void main() noexcept {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL << ENDL;
    
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
    
    contrastStretchTransformation(inputFile, outputFile);
  }
};

APPLICATION_STUB(ContrastStretchApplication);
