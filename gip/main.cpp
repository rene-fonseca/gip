/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/media/AVIEncoder.h>
#include <gip/io/BMPEncoder.h>
#include <gip/io/PCXEncoder.h>
#include <gip/io/GIFEncoder.h>
#include <gip/transformation/Scale.h>
#include <gip/transformation/LinearScale.h>
#include <gip/transformation/Test.h>
#include <gip/transformation/Gradient.h>
#include <gip/transformation/FourierTransformation.h>
#include <gip/transformation/WalshTransformation.h>
#include <gip/transformation/Convert.h>
#include <gip/transformation/DrawRectangle.h>
#include <gip/transformation/MinMax.h>
#include <gip/transformation/Flip.h>
#include <gip/transformation/Mirror.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/string/StringOutputStream.h>
#include <base/Functor.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>
#include <math.h>

using namespace gip;
using namespace base;

class InvertGray : public UnaryOperation<GrayPixel, GrayPixel> {
public:

  inline Result operator()(const Argument& value) throw() {
    GrayPixel result;
    result = MAXIMUM_INTENSITY - value;
    return result;
  }
};

class InvertColor : public UnaryOperation<ColorPixel, ColorPixel> {
public:

  inline Result operator()(const Argument& value) throw() {
    ColorPixel result;
    result.blue = MAXIMUM_INTENSITY - value.blue;
    result.green = MAXIMUM_INTENSITY - value.green;
    result.red = MAXIMUM_INTENSITY - value.red;
    return result;
  }
};

class SqrSum : public BinaryOperation<Complex, Complex, void> {
private:

  double result;
public:

  inline SqrSum() throw() : result(0) {}

  inline void operator()(const FirstArgument& left, const SecondArgument& right) throw() {
    Complex temp = left - right;
    result += temp.getSqrModulus();
  }

  inline double getResult() throw() {
    return result;
  }
};

// need outfix time with message

template<class ENCODER>
void writeColorImage(const String& name, ColorImage* image) throw(IOException) {
  ENCODER encoder(name);
  encoder.write(image);
}

template<class ENCODER>
void writeGrayImage(const String& name, GrayImage* image) throw(IOException) {
  ENCODER encoder(name);
  encoder.writeGray(image);
}

class SumGray : public UnaryOperation<ColorPixel, void> {
protected:
  double result;
public:
  inline SumGray() throw() : result(0) {}
  inline Result operator()(const Argument& value) throw() {RGBToGray opr; result += opr(value);}
  inline double getResult() const throw() {return result;}
};

class SumReal : public UnaryOperation<ComplexPixel, void> {
protected:
  double result;
public:
  inline SumReal() throw() : result(0) {}
  inline Result operator()(const Argument& value) throw() {result += value.getReal();}
  inline double getResult() const throw() {return result;}
};

ImageEncoder* readEncoder;
ImageEncoder* writeEncoder;
ColorImage* image;
ColorImage* destImage;

void testAVI() {
  fout << MESSAGE("Testing the AVI encoder...") << ENDL;
  AVIEncoder encoder("clock.avi");
  encoder.getInfo(fout);

  try {
    AVIReader reader("clock.avi");

    fout << MESSAGE("Valid movie: ") << reader.isValid() << EOL
         << MESSAGE("Dimension of movie: ") << reader.getDimension() << EOL
         << MESSAGE("Number of frames: ") << reader.getNumberOfFrames() << ENDL;
    ColorImage frame(reader.getDimension());

    for (unsigned int i = 0; i < reader.getNumberOfFrames(); ++i) {
      reader.getFrame(frame);
      StringOutputStream filename;
      filename << MESSAGE("frame") << setWidth(3) << ZEROPAD << i << MESSAGE(".bmp") << FLUSH;
      BMPEncoder(filename.getString()).write(&frame);
    }
  } catch (Exception& e) {
    ferr << MESSAGE("Exception: ") << e.getMessage() << ENDL;
    return;
  }
}



void testFourierTransfomation(ColorImage* image) {
  fout << MESSAGE("Testing the Fast Fourier transfomation...") << ENDL;
}



class WalshToGray : public UnaryOperation<FloatPixel, GrayPixel> {
private:

  long double scale;
public:

  inline WalshToGray(const Dimension& dimension) throw() : scale(1./dimension.getSize()) {}

  inline Result operator()(const Argument& value) {
    long double temp = MAXIMUM_INTENSITY * log(1 + value * scale);
    if (temp < MINIMUM_INTENSITY) {
      return MINIMUM_INTENSITY;
    } else if (temp > MAXIMUM_INTENSITY) {
      return MAXIMUM_INTENSITY;
    } else {
      return GrayPixel(static_cast<Intensity>(temp));
    }
  }
};

void testWalshTransformation(ColorImage* image) {
  fout << MESSAGE("Testing the Walsh transfomation...") << ENDL;

  FloatImage spaceImage(image->getDimension());
  {
    Convert<FloatImage, ColorImage, RGBToFloat> transform(&spaceImage, image, RGBToFloat());
    fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
    transform();
  }

  FloatImage walshImage(spaceImage.getDimension());
  {
    WalshTransformation transform(&walshImage, &spaceImage);
    fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
    timeScope();
    transform();
  }
  fout << MESSAGE("  0Hz=") << *walshImage.getElements() << ENDL;

  {
    fout << MESSAGE("Filtering...") << ENDL;
    FloatPixel* elements = walshImage.getElements();
    unsigned int rows = walshImage.getDimension().getHeight();
    unsigned int columns = walshImage.getDimension().getWidth();
    for (unsigned int row = 0; row < rows; ++row) {
      for (unsigned int column = 0; column < columns; ++column) {
        if ((row + column) > 256) {
          elements[row * columns + column] = 0;
        }
      }
    }
  }

  {
    WalshTransformation transform(&spaceImage, &walshImage);
    fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
    timeScope();
    transform();
  }

  GrayImage grayImage(spaceImage.getDimension());
  {
    long double scale = 1./walshImage.getDimension().getSize();
    Convert<GrayImage, FloatImage, FloatToGrayWithScale> transform(&grayImage, &spaceImage, FloatToGrayWithScale(scale));
    fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
    transform();
  }

  BMPEncoder walshEncoder("walsh.bmp");
  walshEncoder.writeGray(&grayImage);
}

void test(const String& input, const String& output) {

  readEncoder = new GIFEncoder(input);
  writeEncoder = new BMPEncoder(output);

  fout << MESSAGE("Information:") << ENDL;
  readEncoder->getInfo(fout);

  fout << MESSAGE("Checking whether file is valid...") << ENDL;
  if (readEncoder->isValid()) {
    fout << MESSAGE("Importing image with encoder: ") << readEncoder->getDescription() << ENDL;
    image = readEncoder->read();

    fout << MESSAGE("Exporting image with encoder: ") << writeEncoder->getDescription() << ENDL;
    writeEncoder->write(image);
    return;

    ColorImage* scaledImage = new ColorImage(Dimension(512, 512));
    {
      LinearScale transform(scaledImage, image);
      fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

    {
      SumGray sum;
      fout << MESSAGE("Calculating the average... ") << ENDL;
      forEach(*scaledImage, sum);
      fout << MESSAGE("  average=") << sum.getResult()/scaledImage->getDimension().getSize() << ENDL;
    }

//    {
//      ColorPixel color;
//      color.blue = 0x00;
//      color.green = 0x00;
//      color.red = 0x00;
//      Region region(Coordinate(0, 0), Dimension(512, 512));
//      DrawRectangle transform(scaledImage, region, color);
//      fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//      transform();
//
//      color.blue = 0xff;
//      color.green = 0xff;
//      color.red = 0xff;
//      Region region2(Coordinate((512 - 64)/2, 64/2), Dimension(64, 512 - 64));
//      DrawRectangle transform2(scaledImage, region2, color);
//      fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//      transform2();
//    }

    ComplexImage* spaceImage = new ComplexImage(scaledImage->getDimension());
    {
      Convert<ComplexImage, ColorImage, RGBToComplex> transform(spaceImage, scaledImage, 1./scaledImage->getDimension().getSize());
      fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

    {
      SumReal sum;
      fout << MESSAGE("Calculating the average... ") << ENDL;
      forEach(*spaceImage, sum);
      fout << MESSAGE("  average=") << sum.getResult()/spaceImage->getDimension().getSize() << ENDL;
    }

    testWalshTransformation(scaledImage);

    ComplexImage* frequencyImage = new ComplexImage(spaceImage->getDimension());
    {
      FourierTransformation transform(frequencyImage, spaceImage);
      fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      timeScope();
      transform();
    }
    fout << MESSAGE("  0Hz=") << *frequencyImage->getElements() << ENDL;

//    double scale = frequencyImage->getDimension().getSize();
//    {
//      FindMaximum analysis(frequencyImage);
//      fout << MESSAGE("Analysing image... ") << '(' << TypeInfo::getTypename(analysis) << ')' << ENDL;
//      scale = MAXIMUM_INTENSITY/analysis();
//      fout << MESSAGE("  scale=") << scale << ENDL;
//    }

    {
      FourierTransformation transform(spaceImage, frequencyImage, false);
      fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      timeScope();
      transform();
    }

    {
      SumReal sum;
      fout << MESSAGE("Calculating the average... ") << ENDL;
      forEach(*spaceImage, sum);
      fout << MESSAGE("  average=") << sum.getResult()/spaceImage->getDimension().getSize() << ENDL;
    }

    ColorImage* finalImage = scaledImage;
    {
      Convert<ColorImage, ComplexImage, ComplexToRGB> transform(finalImage, spaceImage, 1);
      fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
      transform();
    }

//    {
//      InvertColor opr;
//      transform(*finalImage, opr);
//    }

//    Flip<ColorImage> flip(finalImage);
//    flip();
//    Mirror<ColorImage> mirror(finalImage);
//    mirror();

//    {
//      Gradient transform(finalImage, scaledImage);
//      fout << MESSAGE("Transforming image... ") << '(' << TypeInfo::getTypename(transform) << ')' << ENDL;
//      transform();
//    }

//    Scale<ColorImage, ColorImage> transform(destImage, image);
//    Test2 transform(destImage);

    fout << MESSAGE("Exporting image with encoder: ") << writeEncoder->getDescription() << ENDL;
    writeEncoder->write(finalImage);
    delete writeEncoder;
  } else {
    fout << MESSAGE("File format not supported by image encoder") << ENDL;
  }
}

void pixelInformation() {
  GrayPixel grayArray[1024];
  ColorPixel colorArray[1024];
  FloatPixel floatArray[1024];
  ComplexPixel complexArray[1024];

  fout << MESSAGE("Pixel Information") << EOL
       << MESSAGE("  GrayPixel: pixel=") << sizeof(GrayPixel) << MESSAGE(" array=") << sizeof(grayArray) << EOL
       << MESSAGE("  ColorPixel: pixel=") << sizeof(ColorPixel) << MESSAGE(" array=") << sizeof(colorArray) << EOL
       << MESSAGE("  FloatPixel: pixel=") << sizeof(FloatPixel) << MESSAGE(" array=") << sizeof(floatArray) << EOL
       << MESSAGE("  ComplexPixel: pixel=") << sizeof(ComplexPixel) << MESSAGE(" array=") << sizeof(complexArray)<< EOL
       << EOL;
}

class GIPApplication : public Application {
public:

  GIPApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("gip"), numberOfArguments, arguments, environment) {
  }
  
  void main() throw() {
    fout << MESSAGE("Generic Image Processing Framework") << EOL << EOL;
    
    String input;
    String output;

    const Array<String> arguments = getArguments();
    switch (arguments.getSize()) {
    case 3:
      input = arguments[1]; // the file name of the image
      output = arguments[2]; // the file name of the image
      break;
    default:
      fout << MESSAGE("gip [input] [output]") << ENDL;
      return; // stop
    }
    
//    testAVI();
//    test(input, output);
    Timer timer;
    while (timer.getLiveMicroseconds() < 30000000);
  }
};

int main(int argc, const char* argv[], const char* env[]) {
  GIPApplication application(argc, argv, env);
  try {
    application.main();
  } catch(Exception& e) {
    return Application::getApplication()->exceptionHandler(e);
  } catch(...) {
    return Application::getApplication()->exceptionHandler();
  }
  return Application::getApplication()->getExitCode();
}
