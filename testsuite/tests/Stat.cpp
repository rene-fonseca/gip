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
#include <gip/analysis/Statistic.h>
#include <gip/transformation/Convert.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>

using namespace dk::sdu::mip::gip;

class StatApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  StatApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("Stat"), numberOfArguments, arguments, environment) {
  }

  void stat(const String& inputFile) throw() {
    BMPEncoder encoder;
    
    ColorImage* image = encoder.read(inputFile);
    GrayImage grayOriginalImage(image->getDimension());
    {
      Convert<GrayImage, ColorImage, RGBToGray> transform(&grayOriginalImage, image, RGBToGray());
      transform();
    }
    delete image;
    
    Timer timer;
    timer.start();
    Statistic<GrayImage> statistic(grayOriginalImage);
    timer.stop();
    fout << MESSAGE("Time elapsed: ") << timer.getMicroseconds() << MESSAGE(" microseconds") << EOL;

    fout << MESSAGE("Dimension: ") << grayOriginalImage.getDimension() << EOL
         << MESSAGE("Number of samples: ") << statistic.getNumberOfSamples() << EOL
         << MESSAGE("Minimum value: ") << statistic.getMinimum() << EOL
         << MESSAGE("Maximum value: ") << statistic.getMaximum() << EOL
         << MESSAGE("Dynamic range: ") << statistic.getDynamicRange() << EOL
         << MESSAGE("Number of used values: ") << statistic.getUsedValues() << EOL
         << MESSAGE("Minimum frequency: ") << statistic.getMinimumFrequency() << EOL
         << MESSAGE("Maximum frequency: ") << statistic.getMaximumFrequency() << EOL
         << MESSAGE("Mean value: ") << setPrecision(3) << statistic.getMean() << EOL
         << MESSAGE("Median value: ") << statistic.getMedian() << EOL
         << MESSAGE("Mode value: ") << statistic.getMode() << EOL
         << MESSAGE("Variance: ") << setPrecision(3) << statistic.getVariance() << EOL
         << MESSAGE("Standard deviation: ") << setPrecision(3) << statistic.getDeviation() << EOL
         << MESSAGE("Coefficient of variation: ") << setPrecision(3) << statistic.getCoefficientOfVariation() << EOL
         << MESSAGE("Entropy: ") << setPrecision(3) << statistic.getEntropy() << EOL
         << ENDL;
  }

  void help() throw() {
    fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" input") << EOL
         << ENDL;
  }
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("http://www.mip.sdu.dk/~fonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>") << EOL << ENDL;
    
    const Array<String> arguments = getArguments();
    if (arguments.getSize() == 1) {
      String inputFile = arguments[0]; // the file name of the source image
      stat(inputFile);
    } else {
      help();
    }
  }
};

STUB(StatApplication);
