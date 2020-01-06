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
#include <gip/transformation/StraightLineHoughTransformation.h>
#include <gip/ArrayImage.h>
#include <gip/transformation/Convert.h>
#include <gip/analysis/MinimumMaximum.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>
#include <base/string/InvalidFormat.h>
#include <base/Integer.h>

using namespace com::azure::dev::gip;

class HoughApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  HoughApplication() noexcept
    : Application(MESSAGE("Hough")) {
  }

  void houghTransformation(const String& inputFile, const String& outputFile) noexcept {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    GrayImage grayOriginalImage(image->getDimension());
    {
      Convert<GrayImage, ColorImage, RGBToGray> transform(&grayOriginalImage, image, RGBToGray());
      transform();
    }
    delete image;
    
    FloatImage houghImage(grayOriginalImage.getDimension());
    
    {
      StraightLineHoughTransformation transform(&houghImage, &grayOriginalImage);
      fout << MESSAGE("Transforming image: ") << ' ' << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for scale: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }
    
    double maximum = 0;
    {
      MinimumMaximum<double> minimumMaximum;
      forEach(houghImage, minimumMaximum);
      maximum = minimumMaximum.getMaximum();
    }
    
    GrayImage finalImage(houghImage.getDimension());
    {
      Convert<GrayImage, FloatImage, FloatToGrayWithScale> transform(&finalImage, &houghImage, FloatToGrayWithScale(PixelTraits<GrayPixel>::MAXIMUM/maximum));
      transform();
    }
    
    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.writeGray(outputFile, &finalImage);
  }
  
  void main() noexcept {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL << ENDL;

    String inputFile;
    String outputFile;
    
    const Array<String> arguments = getArguments();
    switch (arguments.getSize()) {
    case 0:
      fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" input output") << ENDL;
      return;
    case 2:
      inputFile = arguments[0]; // the file name of the source image
      outputFile = arguments[1]; // the file name of the destination image
      break;
    default:
      ferr << MESSAGE("Error: Invalid argument(s)") << ENDL;
      fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" input output") << ENDL;
      setExitCode(EXIT_CODE_ERROR);
      return;
    }
    
    houghTransformation(inputFile, outputFile);
  }
};

APPLICATION_STUB(HoughApplication);
