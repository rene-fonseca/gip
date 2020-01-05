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
#include <gip/transformation/LinearScale.h>
#include <gip/transformation/DiscreteCosineTransformation.h>
#include <gip/transformation/Convert.h>
#include <gip/operation/HeatColorMap.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>
#include <gip/analysis/MinimumMaximum.h>
#include <gip/Functor.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/string/StringOutputStream.h>
#include <base/Functor.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>
#include <base/math/Constants.h>

using namespace com::azure::dev::gip;

class CosineToGray : public UnaryOperation<float, GrayPixel> {
private:

   long double scale;
public:

  inline CosineToGray(long double _scale) noexcept
    : scale(PixelTraits<GrayPixel>::MAXIMUM * _scale) {
  }
  
  inline CosineToGray(const Dimension& dimension) noexcept
    : scale(1.0/dimension.getSize()) {
  }
  
  inline GrayPixel operator()(const float& value) const noexcept {
    return clamp<GrayPixel>(
      0,
      static_cast<GrayPixel>(scale * Math::ln(1 + value)),
      PixelTraits<GrayPixel>::MAXIMUM
    );
  }
};

class MapToHue : public UnaryOperation<ColorPixel, long double> {
private:

  long double scale;
  HeatColorMap map;
public:

  MapToHue(long double _scale) noexcept : scale(_scale) {
  }

  inline ColorPixel operator()(const long double& value) const noexcept {
    RGBPixel<long double> temp = map(Math::sqrt(value * scale));
    ColorPixel result;
    result.red = static_cast<PixelTraits<ColorPixel>::Component>(PixelTraits<ColorPixel>::MAXIMUM * temp.red + 0.5);
    result.green = static_cast<PixelTraits<ColorPixel>::Component>(PixelTraits<ColorPixel>::MAXIMUM * temp.green + 0.5);
    result.blue = static_cast<PixelTraits<ColorPixel>::Component>(PixelTraits<ColorPixel>::MAXIMUM * temp.blue + 0.5);
    return result;
  }
};

class DCTApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  DCTApplication() noexcept
    : Application(MESSAGE("DiscreteCosineTransformation")) {
  }
  
  void dct(const String& inputFile, const String& outputFile) noexcept {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;
    
    Dimension dimension(
      Math::getPowerOf2(originalImage.getDimension().getWidth()),
      Math::getPowerOf2(originalImage.getDimension().getHeight())
    );
    
    ColorImage scaledImage(dimension);
    {
      LinearScale transform(&scaledImage, &originalImage);
      fout << MESSAGE("Scaling image: ") << originalImage.getDimension() << MESSAGE("->") << scaledImage.getDimension() << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    FloatImage spaceImage(scaledImage.getDimension());
    {
      Convert<FloatImage, ColorImage, RGBToFloat> transform(&spaceImage, &scaledImage, RGBToFloat());
      fout << MESSAGE("Converting image: ") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

    FloatImage cosineImage(spaceImage.getDimension());
    {
      DiscreteCosineTransformation transform(&cosineImage, &spaceImage);
      fout << MESSAGE("Transforming image: ") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for transformation: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    long double maximum;
    {
      MinimumMaximum<long double> minimumMaximum;
      forEach(cosineImage, minimumMaximum);
      maximum = minimumMaximum.getMaximum();
    }
    fout << MESSAGE("Maximum: ") << maximum << ENDL;
    
    GrayImage finalImage(cosineImage.getDimension());
    {
      Convert<GrayImage, FloatImage, CosineToGray> transform(
        &finalImage,
        &cosineImage,
        CosineToGray(1/Math::ln(1 + maximum))
      );
      transform();
    }
    
//     ArrayImage<long double> modulusImage(cosineImage.getDimension());
//     long double maximumModulus;
//     {
//       Convert<ArrayImage<long double>, FloatImage, FourierToLogModulus> transform(&modulusImage, &cosineImage, FourierToLogModulus(cosineImage.getDimension()));
//       fout << MESSAGE("Converting image: ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//       transform();
//       maximumModulus = transform.getResult().getMaximum();
//     }

//     ColorImage finalImage(modulusImage.getDimension());
//     MapToHue map(1.0/maximumModulus);
//     fillWithUnary(finalImage, modulusImage, map);

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
    case 2:
      inputFile = arguments[0]; // the file name of the source image
      outputFile = arguments[1]; // the file name of the destination image
      break;
    default:
      fout << MESSAGE("Usage: ") << getFormalName() << MESSAGE(" input output") << ENDL;
      return; // stop
    }
    
    dct(inputFile, outputFile);
  }
};

APPLICATION_STUB(DCTApplication);
