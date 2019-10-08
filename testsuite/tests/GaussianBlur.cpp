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

using namespace dk::sdu::mip::gip;

class Gaussian : public BinaryOperation<long double, long double, long double> {
private:
  
  const long double denomX;
  const long double denomY;
public:
  
  inline Gaussian(long double deviationX, long double deviationY) throw()
    : denomX(-1/(2 * deviationX * deviationX)),
      denomY(-1/(2 * deviationY * deviationY)) {
  }
  
  inline long double operator()(long double x, long double y) const throw() {
    return Math::exp(x * x * denomX + y * y * denomY);
  }
};

class RealToGray : public UnaryOperation<Complex, GrayPixel> {
private:

   const long double scale;
public:

  inline RealToGray(const Dimension& dimension) throw()
    : scale(1.0/dimension.getSize()) {
  }
  
  //inline RealToGray(const Dimension& dimension) throw() : scale(255) {
  //}

  inline GrayPixel operator()(const Complex& value) const throw() {
    return clamp(0, static_cast<GrayPixel>(value.getReal() * scale), 255);
  }
};

class GaussianBlur : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:
  
  GaussianBlur(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("GaussianBlur"), numberOfArguments, arguments, environment) {
  }
  
  void blur(const String& inputFile, const String& outputFile) throw() {
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
    
    /*ArrayImage<long double>*/ ComplexImage filterImage(fourierImage.getDimension());
    {
      Gaussian gaussian(
        8192/512*filterImage.getDimension().getWidth(),
        8192/512*filterImage.getDimension().getHeight()
      );
      //typedef ArrayImage<long double> DestinationImage;
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
          long double temp = gaussian(x++, y++);
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
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("https://dev.azure.com/renefonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002-2019 by Rene Moeller Fonseca") << EOL
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

STUB(GaussianBlur);
