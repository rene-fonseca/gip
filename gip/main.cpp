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
#include <base/string/FormatOutputStream.h>
#include <base/string/StringOutputStream.h>
#include <base/Functor.h>
#include <base/Timer.h>
#include <typeinfo>
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
  fout << "Testing the AVI encoder..." << ENDL;
  AVIEncoder encoder("clock.avi");
  encoder.getInfo(fout);

  try {
    AVIReader reader("clock.avi");

    fout << "Valid movie: " << reader.isValid() << ENDL;
    fout << "Dimension of movie: " << reader.getDimension() << ENDL;
    fout << "Number of frames: " << reader.getNumberOfFrames() << ENDL;
    ColorImage frame(reader.getDimension());

    for (unsigned int i = 0; i < reader.getNumberOfFrames(); ++i) {
      reader.getFrame(frame);
      StringOutputStream filename;
      filename << "frame" << setWidth(3) << ZEROPAD << i << ".bmp" << FLUSH;
      BMPEncoder(filename.getString()).write(&frame);
    }
  } catch (Exception& e) {
    ferr << "Exception: " << e.getMessage() << ENDL;
    return;
  }
}



void testFourierTransfomation(ColorImage* image) {
  fout << "Testing the Fast Fourier transfomation..." << ENDL;
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
  fout << "Testing the Walsh transfomation..." << ENDL;

  FloatImage spaceImage(image->getDimension());
  {
    Convert<FloatImage, ColorImage, RGBToFloat> transform(&spaceImage, image, RGBToFloat());
    fout << "Transforming image... " << typeid(transform).name() << ENDL;
    transform();
  }

  FloatImage walshImage(spaceImage.getDimension());
  {
    WalshTransformation transform(&walshImage, &spaceImage);
    fout << "Transforming image... " << typeid(transform).name() << ENDL;
    timeScope();
    transform();
  }
  fout << "  0Hz=" << *walshImage.getElements() << ENDL;

  {
    fout << "Filtering..." << ENDL;
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
    fout << "Transforming image... " << typeid(transform).name() << ENDL;
    timeScope();
    transform();
  }

  GrayImage grayImage(spaceImage.getDimension());
  {
    long double scale = 1./walshImage.getDimension().getSize();
    Convert<GrayImage, FloatImage, FloatToGrayWithScale> transform(&grayImage, &spaceImage, FloatToGrayWithScale(scale));
    fout << "Transforming image... " << typeid(transform).name() << ENDL;
    transform();
  }

  BMPEncoder walshEncoder("walsh.bmp");
  walshEncoder.writeGray(&grayImage);
}

void test(const String& input, const String& output) {

  readEncoder = new GIFEncoder(input);
  writeEncoder = new BMPEncoder(output);

  fout << "Information:" << ENDL;
  readEncoder->getInfo(fout);

  fout << "Checking whether file is valid..." << ENDL;
  if (readEncoder->isValid()) {
    fout << "Importing image with encoder: " << readEncoder->getDescription() << ENDL;
    image = readEncoder->read();

    fout << "Exporting image with encoder: " << writeEncoder->getDescription() << ENDL;
    writeEncoder->write(image);
    return;

    ColorImage* scaledImage = new ColorImage(Dimension(512, 512));
    {
      LinearScale transform(scaledImage, image);
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
      transform();
    }

    {
      SumGray sum;
      fout << "Calculating the average... " << ENDL;
      forEach(*scaledImage, sum);
      fout << "  average=" << sum.getResult()/scaledImage->getDimension().getSize() << ENDL;
    }

//    {
//      ColorPixel color;
//      color.blue = 0x00;
//      color.green = 0x00;
//      color.red = 0x00;
//      Region region(Coordinate(0, 0), Dimension(512, 512));
//      DrawRectangle transform(scaledImage, region, color);
//      fout << "Transforming image... " << typeid(transform).name() << ENDL;
//      transform();
//
//      color.blue = 0xff;
//      color.green = 0xff;
//      color.red = 0xff;
//      Region region2(Coordinate((512 - 64)/2, 64/2), Dimension(64, 512 - 64));
//      DrawRectangle transform2(scaledImage, region2, color);
//      fout << "Transforming image... " << typeid(transform).name() << ENDL;
//      transform2();
//    }

    ComplexImage* spaceImage = new ComplexImage(scaledImage->getDimension());
    {
      Convert<ComplexImage, ColorImage, RGBToComplex> transform(spaceImage, scaledImage, 1./scaledImage->getDimension().getSize());
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
      transform();
    }

    {
      SumReal sum;
      fout << "Calculating the average... " << ENDL;
      forEach(*spaceImage, sum);
      fout << "  average=" << sum.getResult()/spaceImage->getDimension().getSize() << ENDL;
    }

    testWalshTransformation(scaledImage);

    ComplexImage* frequencyImage = new ComplexImage(spaceImage->getDimension());
    {
      FourierTransformation transform(frequencyImage, spaceImage);
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
      timeScope();
      transform();
    }
    fout << "  0Hz=" << *frequencyImage->getElements() << ENDL;

//    double scale = frequencyImage->getDimension().getSize();
//    {
//      FindMaximum analysis(frequencyImage);
//      fout << "Analysing image... " << typeid(analysis).name() << ENDL;
//      scale = MAXIMUM_INTENSITY/analysis();
//      fout << "  scale=" << scale << ENDL;
//    }

    {
      FourierTransformation transform(spaceImage, frequencyImage, false);
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
      timeScope();
      transform();
    }

    {
      SumReal sum;
      fout << "Calculating the average... " << ENDL;
      forEach(*spaceImage, sum);
      fout << "  average=" << sum.getResult()/spaceImage->getDimension().getSize() << ENDL;
    }

    ColorImage* finalImage = scaledImage;
    {
      Convert<ColorImage, ComplexImage, ComplexToRGB> transform(finalImage, spaceImage, 1);
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
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
//      fout << "Transforming image... " << typeid(transform).name() << ENDL;
//      transform();
//    }

//    Scale<ColorImage, ColorImage> transform(destImage, image);
//    Test2 transform(destImage);

    fout << "Exporting image with encoder: " << writeEncoder->getDescription() << ENDL;
    writeEncoder->write(finalImage);
    delete writeEncoder;
  } else {
    fout << "File format not supported by image encoder" << ENDL;
  }
}

void pixelInformation() {
  GrayPixel grayArray[1024];
  ColorPixel colorArray[1024];
  FloatPixel floatArray[1024];
  ComplexPixel complexArray[1024];

  fout << "Pixel Information" << EOL;
  fout << "  GrayPixel: pixel=" << sizeof(GrayPixel) << " array=" << sizeof(grayArray) << EOL;
  fout << "  ColorPixel: pixel=" << sizeof(ColorPixel) << " array=" << sizeof(colorArray) << EOL;
  fout << "  FloatPixel: pixel=" << sizeof(FloatPixel) << " array=" << sizeof(floatArray) << EOL;
  fout << "  ComplexPixel: pixel=" << sizeof(ComplexPixel) << " array=" << sizeof(complexArray)<< EOL;
  fout << EOL;
}

int main(int argc, char* argv[]) {
  fout << "Generic Image Processing Framework" << EOL << EOL;

  String input;
  String output;

  switch (argc) {
  case 3:
    input = argv[1]; // the file name of the image
    output = argv[2]; // the file name of the image
    break;
  default:
    fout << "gip [input] [output]" << ENDL;
    return 0; // stop
  }

  try {
//    testAVI();
//    test(input, output);
  } catch(Exception& e) {
    ferr << typeid(e).name() << ": "<< e.getMessage() << ENDL;
    return 1;
  } catch(...) {
    ferr << "Unknown exception" << ENDL;
    return 1;
  }
  fout << "Completed" << ENDL;
  return 0;
}
