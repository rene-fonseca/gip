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
#include <gip/transformation/HaarTransformation.h>
#include <gip/transformation/Convert.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/string/StringOutputStream.h>
#include <base/Functor.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>
#include <base/mathematics/Random.h>

using namespace dk::sdu::mip::gip;

class HaarToGray : public UnaryOperation<float, GrayPixel> {
public:

  inline GrayPixel operator()(const float& value) const throw() {
    long double temp = value/2 + 128; // -255 <= value <= 255
    if (temp < 0x00) {
      return 0x00;
    } else if (temp > 0xff) {
      return 0xff;
    } else {
      return static_cast<GrayPixel>(temp);
    }
  }
};

class HaarToGray2 : public UnaryOperation<GrayPixel, GrayPixel> {
public:

  inline GrayPixel operator()(const GrayPixel& value) const throw() {
    GrayPixel temp = value + 128; // -255 <= value <= 255
    return static_cast<GrayPixel>(temp);
//     if (temp < 0x00) {
//       return 0x00;
//     } else if (temp > 0xff) {
//       return 0xff;
//     } else {
//       return static_cast<GrayPixel>(temp);
//     }
  }
};

class HaarApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  HaarApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("HaarTransformation"), numberOfArguments, arguments, environment) {
  }

  void haarTransformation(const String& inputFile, const String& outputFile) throw() {    


//     Random random;
//     for (long long i = 0; i < 32*256*256*256; ++i) {
//       int origo = random.getInteger()%256;
//       int orige = random.getInteger()%256;
//       int diff = origo - orige;
//       int avg = (origo + orige)/2;
// //      int oo = (2 * avg + 1 + diff)/2;
// //      int ee = oo - diff;

//       int ee = (2 * avg + 1 - diff)/2;
//       int oo = diff + ee;
      
//       if ((ee != orige) || (oo != origo)) {
//         fout << "1:  ee:" << ee << "  oo:" << oo << "  avg:" << avg << "  diff:" << diff << "  orige:" << orige << "  origo:" << origo << ENDL;
//       }
//     }
    
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    ColorImage originalImage(*image);
    delete image;
    
    Dimension dimension(
      Math::getPowerOf2(originalImage.getDimension().getWidth()),
      Math::getPowerOf2(originalImage.getDimension().getHeight())
    );
    // dimension = Dimension(128, 128); // TAG: debug
    
    ColorImage scaledImage(dimension);
    {
      LinearScale transform(&scaledImage, &originalImage);
      fout << MESSAGE("Scaling image: ") << originalImage.getDimension() << MESSAGE("->") << scaledImage.getDimension() << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    
    FloatImage spatialImage(scaledImage.getDimension());
    {
      Convert<FloatImage, ColorImage, RGBToFloat> transform(&spatialImage, &scaledImage, RGBToFloat());
      fout << MESSAGE("Converting image: ") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    
    FloatImage& haarImage = spatialImage;
    {
      HaarTransformation<FloatImage> transform(&haarImage);
      fout << MESSAGE("Transforming image: Spatial->Haar") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      Timer timer;
      transform();
      // transform.inverse();
      fout << MESSAGE("Time elapsed for Haar transformation: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
    }

    GrayImage grayImage(spatialImage.getDimension());
    {
      Convert<GrayImage, FloatImage, HaarToGray> transform(&grayImage, &haarImage, HaarToGray());
      fout << MESSAGE("Converting image: ") << ' '
           << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }
    
//     {
//       unsigned long long sum = 0;
//       const GrayPixel* src = spatialImage.getElements();
//       for (unsigned int i = spatialImage.getDimension().getSize(); i > 0; --i) {
//         sum += *src++;
//       }
//       fout << "calc avg:" << sum/(1. * spatialImage.getDimension().getSize()) << ENDL;
//     }

//     GrayImage spatialImage(scaledImage.getDimension());
//     {
//       Convert<GrayImage, ColorImage, RGBToGray> transform(&spatialImage, &scaledImage, RGBToGray());
//       fout << MESSAGE("Converting image: ") << ' '
//            << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//       transform();
//     }
    
//     GrayImage& haarImage = spatialImage;
//     {
//       HaarTransformation<GrayImage> transform(&haarImage);
//       fout << MESSAGE("Transforming image: Spatial->Haar") << ' '
//            << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//       Timer timer;
//       transform();
//       fout << "avg: " << haarImage.getElements()[0] << ENDL;
      
//       transform.inverse();
//       fout << MESSAGE("Time elapsed for Haar transformation: ") << timer.getLiveMicroseconds() << MESSAGE(" microseconds") << EOL;
//     }

//    GrayImage& grayImage = spatialImage;
//     {
//       Convert<GrayImage, GrayImage, HaarToGray2> transform(&grayImage, &haarImage, HaarToGray2());
//       fout << MESSAGE("Converting image: ") << ' '
//            << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//       transform();
//     }

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.writeGray(outputFile, &grayImage);
  }
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("https://dev.azure.com/renefonseca/gip") << EOL
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
    
    haarTransformation(inputFile, outputFile);
  }
};

STUB(HaarApplication);
