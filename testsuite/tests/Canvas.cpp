/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/io/BMPEncoder.h>
#include <gip/canvas/Canvas.h>
#include <gip/transformation/Convert.h>
#include <gip/ArrayImage.h>
#include <base/Application.h>
#include <base/string/FormatOutputStream.h>
#include <base/Timer.h>
#include <base/TypeInfo.h>
#include <base/mathematics/Random.h>
#include <base/mathematics/Constants.h>

using namespace dk::sdu::mip::gip;

class CanvasApplication : public Application {
private:

  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:

  CanvasApplication(int numberOfArguments, const char* arguments[], const char* environment[]) throw()
    : Application(MESSAGE("Canvas"), numberOfArguments, arguments, environment) {
  }

  Point getPoint(const Dimension& dimension) throw() {
    return Point(
      Random::getInteger() % dimension.getWidth(),
      Random::getInteger() % dimension.getHeight()
    );
  }

  Point getPoint2(const Dimension& dimension) throw() {
    return Point(
      Random::getInteger() % 10000 - 5000,
      Random::getInteger() % 10000 - 5000
    );
  }

  void canvasTransformation(const String& inputFile, const String& outputFile) throw() {
    BMPEncoder encoder;
    
    fout << MESSAGE("Importing image with encoder: ") << encoder.getDescription() << ENDL;
    ColorImage* image = encoder.read(inputFile);
    
    Canvas canvas(image);

    ColorImage minor(Dimension(100, 100));
    Canvas canvas2(&minor);

    canvas2.rectangle(Point(0, 0), Point(99, 99), makeColorPixel(255, 0, 0), Canvas::FILL);
    canvas.image(Point(100, 100), minor);
    canvas2.rectangle(Point(0, 0), Point(99, 99), makeColorPixel(0, 255, 0), Canvas::FILL);
    canvas.image(Point(0, 200), minor);
    canvas2.rectangle(Point(0, 0), Point(99, 99), makeColorPixel(0, 0, 255), Canvas::FILL);
    canvas.image(Point(-50, 300), minor);

    canvas.image(Point(-100+1, -100+1), minor);
    canvas.image(Point(-100+1, canvas.getDimension().getHeight()-1), minor);
    canvas.image(Point(canvas.getDimension().getWidth()-1, canvas.getDimension().getHeight()-1), minor);

    canvas.fill(Point(0, 0), Point(canvas.getDimension().getWidth()-1, canvas.getDimension().getHeight()-1), makeColorPixel(0, 0, 0));

//    canvas.circle(Point(30, 30), 10, makeColorPixel(255, 0, 0), Canvas::ANTIALIASING + Canvas::FILL);
//    canvas.circle(Point(80, 30), 10, makeColorPixel(0, 255, 0), Canvas::ANTIALIASING + Canvas::FILL);
//    canvas.circle(Point(130, 30), 10, makeColorPixel(0, 0, 255), Canvas::ANTIALIASING + Canvas::FILL);
//
//    canvas.circle(Point(30+25, 50), 10, makeColorPixel(255, 255, 0), Canvas::ANTIALIASING + Canvas::FILL);
//    canvas.circle(Point(80+25, 50), 10, makeColorPixel(0, 255, 255), Canvas::ANTIALIASING + Canvas::FILL);
//    canvas.circle(Point(130+25, 50), 10, makeColorPixel(255, 0, 255), Canvas::ANTIALIASING + Canvas::FILL);

    canvas.circle(Point(400, 300), 50, makeColorPixel(255, 255, 255), Canvas::ANTIALIASING);
    canvas.circle(Point(400 - 10, 300 - 10), 50, makeColorPixel(255, 255, 255));

    canvas.ellipse(Point(290, 200), Dimension(150, 100), makeColorPixel(128, 64, 192));

//    canvas.ring(Point(300, 200), 200, 230, makeColorPixel(32, 128, 64), Canvas::ANTIALIASING);

    canvas.line(Point(17, 13), Point(354, 237), makeColorPixel(123, 132, 234), Canvas::ANTIALIASING);
    canvas.line(Point(17, 13+10), Point(354, 237+10), makeColorPixel(255, 255, 255), Canvas::ANTIALIASING);
    canvas.line(Point(17, 13+20), Point(354, 237+20), makeColorPixel(0, 0, 0), Canvas::ANTIALIASING);

    // test rectangle
    canvas.fill(Point(200 - 10 * 5, 200 - 10 * 5), Point(200 + 10 * 5, 200 + 10 * 5), makeColorPixel(64, 192, 128));
    for (int i = 0; i < 10; ++i) {
      canvas.rectangle(Point(200 - i * 5, 200 - i * 5), Point(200 + i * 5, 200 + i * 5), makeColorPixel(200 - i * 10, i * 20, i * 10));
    }

    // test circle
    canvas.disk(Point(200, 300), 10 * 7, makeColorPixel(0, 0, 255), Canvas::ANTIALIASING);
    for (int radius = 0; radius < 10; ++radius) {
      canvas.circle(Point(200, 300), radius * 7, makeColorPixel(255, 255, 255), Canvas::ANTIALIASING);
    }

    // test ellipse
    canvas.ellipse(Point(400, 300), Dimension(10 * 13, 10 * 7), makeColorPixel(192, 128, 128), Canvas::ANTIALIASING + Canvas::FILL);
    for (int radius = 1; radius < 10; ++radius) {
      canvas.ellipse(Point(400, 300), Dimension(radius * 13, radius * 7), makeColorPixel(255, 255, 255), Canvas::ANTIALIASING);
    }

    // test line
    for (int i = 0; i < 60; ++i) {
      Point p1(100, 400);
      Point p2(
        static_cast<int>(50 * Math::cos(2*constant::PI/60*i) + 100),
        static_cast<int>(50 * Math::sin(2*constant::PI/60*i) + 400)
      );
      canvas.line(p1, p2, makeColorPixel((i * 255 + 15)/60, 128, 128 - i * 3), Canvas::ANTIALIASING);
    }
    
    // test ring
    for (int radius = 4; radius < 15; ++radius) {
      canvas.ring(
        Point(canvas.getDimension().getWidth() - 50, canvas.getDimension().getHeight() - 50),
        radius*radius*radius/5-2*radius/3+10, radius*radius*radius/5+2*radius/3+10,
        makeColorPixel(128 - radius * 10, 128, 128 + radius * 10),
        Canvas::ANTIALIASING
      );
    }
    
    for (int i = 0; i < canvas.getDimension().getWidth() + 45; i += 5) {
      canvas.line(Point(-45 + i, 0), Point(i, 45), makeColorPixel(255, 0, 0), Canvas::ANTIALIASING);
      canvas.line(Point(-45 + i, 45), Point(i, 0), makeColorPixel(0, 0, 255), Canvas::ANTIALIASING);
    }

    static const ColorPixel primaryColors[] = {
      makeColorPixel(255, 0, 0),
      makeColorPixel(255, 255, 0),
      makeColorPixel(0, 255, 0),
      makeColorPixel(0, 255, 255),
      makeColorPixel(0, 0, 255),
      makeColorPixel(255, 0, 255)
    };

    canvas.ring(Point(100, 100), 62, 38, makeColorPixel(32, 128, 64), Canvas::ANTIALIASING);
    for (int i = 0; i < 6; ++i) {
      canvas.circle(
        Point(
          static_cast<int>(50 * Math::cos(constant::PI/3*i) + 100),
          static_cast<int>(50 * Math::sin(constant::PI/3*i) + 100)
        ),
        10,
        primaryColors[i],
        Canvas::ANTIALIASING + Canvas::FILL
      );
    }

    fout << MESSAGE("Dimension of test: ") << canvas.getDimensionOfText(MESSAGE("Hello, World!")) << ENDL;
    canvas.write(Point(150, 200), MESSAGE("Hello, World!"));

    fout << MESSAGE("Exporting image with encoder: ") << encoder.getDescription() << ENDL;
    encoder.write(outputFile, image);
    delete image;
  }
  
  void main() throw() {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("http://www.mip.sdu.dk/~fonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>") << EOL << ENDL;
    
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
    
    canvasTransformation(inputFile, outputFile);
  }
};

STUB(CanvasApplication);
