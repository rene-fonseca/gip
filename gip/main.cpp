/***************************************************************************
    begin                : Mon Apr 23 17:18:58 CEST 2001
    copyright            : (C) 2001 by René Møller Fonseca
    email                : fonseca@mip.sdu.dk
 ***************************************************************************/

#include <gip/io/BMPEncoder.h>
#include <gip/transformation/Scale.h>
#include <gip/transformation/LinearScale.h>
#include <gip/transformation/Test.h>
#include <gip/transformation/Gradient.h>
#include <gip/transformation/FourierTransformation.h>
#include <gip/transformation/Convert.h>
#include <gip/transformation/DrawRectangle.h>
#include <gip/transformation/MinMax.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>
#include <base/string/FormatOutputStream.h>
#include <base/Functor.h>
#include <base/Timer.h>
#include <typeinfo>
#include <math.h>
#include <sys/time.h>

using namespace gip;
using namespace base;

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
  inline Result operator()(const Argument& value) throw() {result += RGBToGray(value);}
  inline double getResult() const throw() {return result;}
};

class SumReal : public UnaryOperation<ComplexPixel, void> {
protected:
  double result;
public:
  inline SumReal() throw() : result(0) {}
  inline Result operator()(const Argument& value) throw() {result += value.real;}
  inline double getResult() const throw() {return result;}
};

ImageEncoder* readEncoder;
ImageEncoder* writeEncoder;
ColorImage* image;
ColorImage* destImage;

void test(const String& input, const String& output) {

  fout << "Input encoder: BMPEncoder" << ENDL;
  readEncoder = new BMPEncoder(input);

  fout << "Output encoder: BMPEncoder" << ENDL;
  writeEncoder = new BMPEncoder(output);

  fout << "Using encoder: " << readEncoder->getDescription();

  fout << "Information:" << ENDL;
  readEncoder->getInfo(fout);

  fout << "Checking whether file is valid..." << ENDL;
  if (readEncoder->isValid()) {
    fout << "Reading image..." << ENDL;
    image = readEncoder->read();

    ColorImage* scaledImage = new ColorImage(Dimension(512, 512));
    {
      LinearScale transform(scaledImage, image);
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
      transform();
    }
//    writeColorImage<BMPEncoder>("gip1.bmp", scaledImage);

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

    ComplexImage* spaceImage = new ComplexImage(Dimension(512, 512));
    {
      Convert<ComplexImage, ColorImage> transform(spaceImage, scaledImage, 1./scaledImage->getDimension().getSize());
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

    ComplexImage* frequencyImage = new ComplexImage(Dimension(512, 512));
    {
      FourierTransformation transform(frequencyImage, spaceImage);
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
      timeScope();
      for (unsigned int i = 0; i < 16; ++i) {
        transform();
      }
    }

    fout << "  0Hz (real)=" << frequencyImage->getElements()[0].real << ENDL;
    fout << "  0Hz (imaginary)=" << frequencyImage->getElements()[0].imaginary << ENDL;

    double scale = 1;
//    {
//      FindMaximum analysis(frequencyImage);
//      fout << "Analysing image... " << typeid(analysis).name() << ENDL;
//      scale = MAXIMUM_INTENSITY/analysis();
//      fout << "  scale=" << scale << ENDL;
//    }

    ComplexImage* respacedImage = new ComplexImage(Dimension(512, 512));
    {
      FourierTransformation transform(respacedImage, frequencyImage, false);
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
      transform();
    }

    {
      SumReal sum;
      fout << "Calculating the average... " << ENDL;
      forEach(*respacedImage, sum);
      fout << "  average=" << sum.getResult()/respacedImage->getDimension().getSize() << ENDL;
    }

//    {
//      FourierTransformation transform(frequencyImage, respacedImage);
//      fout << "Transforming image... " << typeid(transform).name() << ENDL;
//      transform();
//    }
//    {
//      FourierTransformation transform(respacedImage, frequencyImage);
//      fout << "Transforming image... " << typeid(transform).name() << ENDL;
//      transform();
//    }
//    scale = scale/scaledImage->getDimension().getSize();

    {
      Convert<ColorImage, ComplexImage> transform(scaledImage, respacedImage, scale);
      fout << "Transforming image... " << typeid(transform).name() << ENDL;
      transform();
    }

    ColorImage* finalImage = scaledImage;

//    {
//      Gradient transform(finalImage, scaledImage);
//      fout << "Transforming image... " << typeid(transform).name() << ENDL;
//      transform();
//    }

//    Scale<ColorImage, ColorImage> transform(destImage, image);
//    Test2 transform(destImage);

    fout << "Writing image..." << ENDL;
    writeEncoder->write(finalImage);
  } else {
    fout << "File format not supported by image encoder" << ENDL;
  }
}

int main(int argc, char* argv[]) {
  fout << "Generic Image Processing Framework" << ENDL;

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
    test(input, output);
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
