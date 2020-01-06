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
#include <gip/transformation/FourierTransformation.h>
#include <gip/transformation/Convert.h>
#include <gip/operation/HeatColorMap.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>
#include <gip/Functor.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/string/StringOutputStream.h>
#include <base/Functor.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>

using namespace com::azure::dev::gip;

class Gaussian : public BinaryOperation<double, double, double> {
private:
  
  const double denomX = 0;
  const double denomY = 0;
public:
  
  inline Gaussian(double deviationX, double deviationY) noexcept
    : denomX(-1/(2 * deviationX * deviationX)),
      denomY(-1/(2 * deviationY * deviationY)) {
  }
  
  inline double operator()(double x, double y) const noexcept {
    return Math::exp(x * x * denomX + y * y * denomY);
  }
};

class RealToGray : public UnaryOperation<Complex, GrayPixel> {
private:

   const double scale = 0;
public:

  inline RealToGray(const Dimension& dimension) noexcept
    : scale(1.0/dimension.getSize()) {
  }
  
  //inline RealToGray(const Dimension& dimension) noexcept : scale(255) {
  //}

  inline GrayPixel operator()(const Complex& value) const noexcept {
    return clamp(0, static_cast<GrayPixel>(value.getReal() * scale), 255);
  }
};

class GaussianBlur : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:
  
  GaussianBlur() noexcept
    : Application(MESSAGE("GaussianBlur")) {
  }
  
  void blur(const String& inputFile, const String& outputFile) noexcept {
    BMPEncoder encoder;
    
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
      fout << MESSAGE("Scaling image: ") << originalImage.getDimension()
           << MESSAGE("->") << scaledImage.getDimension() << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    
    ComplexImage spatialImage(scaledImage.getDimension());
    {
      Convert<ComplexImage, ColorImage, RGBToFloat> transform(
        &spatialImage,
        &scaledImage,
        RGBToFloat()
      );
      fout << MESSAGE("Converting image: ColorImage->ComplexImage") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    
    ComplexImage fourierImage(spatialImage.getDimension());
    {
      FourierTransformation transform(&fourierImage, &spatialImage);
      fout << MESSAGE("Transforming image: Spatial->Fourier") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for transformation: ")
           << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }
    
    /*ArrayImage<double>*/ ComplexImage filterImage(fourierImage.getDimension());
    {
      Gaussian gaussian(
        8192/512*filterImage.getDimension().getWidth(),
        8192/512*filterImage.getDimension().getHeight()
      );
      //typedef ArrayImage<double> DestinationImage;
      typedef ComplexImage DestinationImage;
      
      const unsigned int rows = filterImage.getDimension().getHeight()/2;
      const unsigned int columns = filterImage.getDimension().getWidth()/2;
      DestinationImage::Rows rowsLookup = filterImage.getRows();
      DestinationImage::Rows::RowIterator top = rowsLookup.getFirst();
      DestinationImage::Rows::RowIterator bottom = rowsLookup.getEnd();
      unsigned int y = 0;
      while (top < bottom) {
        --bottom;
        DestinationImage::Rows::RowIterator::ElementIterator c0 = top.getFirst();
        DestinationImage::Rows::RowIterator::ElementIterator c1 = top.getEnd();
        DestinationImage::Rows::RowIterator::ElementIterator c2 = bottom.getFirst();
        DestinationImage::Rows::RowIterator::ElementIterator c3 = bottom.getEnd();
        unsigned int x = 0;
        while (c0 < c1) {
          double temp = gaussian(x++, y++);
          *c0++ = temp;
          *--c1 = temp;
          *c2++ = temp;
          *--c3 = temp;
        }
        ++top;
      }
    }
    
    // multiply images
    Multiply<Complex> multiply;
    transform(fourierImage, filterImage, multiply);
    
    {
      FourierTransformation transform(&spatialImage, &fourierImage, false);
      fout << MESSAGE("Transforming image: Fourier->Spatial") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      fout << MESSAGE("Time elapsed for transformation: ")
           << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }
    
    GrayImage grayImage(spatialImage.getDimension());
    {
      Convert<GrayImage, ComplexImage, RealToGray> transform(
        &grayImage,
        &spatialImage,
        RealToGray(spatialImage.getDimension())
      );
      fout << MESSAGE("Converting image: ComplexImage->GrayImage") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    
    encoder.writeGray(outputFile, &grayImage);
  }
  
  void main() noexcept {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << ENDL;
    
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
    
    blur(inputFile, outputFile);
  }
};

APPLICATION_STUB(GaussianBlur);
