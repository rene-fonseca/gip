/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/ArrayImage.h>
#include <gip/Point.h>
#include <base/string/String.h>

namespace gip {

//  class Font : public Object {
//  public:
//  };
//
//  class BitmapFont : public Font {
//  public:
//  };

  /**
    This class supports the drawing of primitives onto an image.

    @short Canvas
    @version 1.0
  */

  class Canvas {
  private:

    /** The pixel type of the canvas. */
    typedef ColorImage::Pixel Pixel;

    /** The image to draw onto. */
    ColorImage* canvas;
    /** The dimension of the canvas. */
    Dimension dimension;
    /** The rows . */
    ColorImage::Rows rows;
    /** The name of the active font. */
    String fontName;

    /** Draws a clipped line onto the canvas. */
    void lineClipped(const Point& p1, const Point& p2, Pixel color) throw();
    /** Draws a clipped line onto the canvas with anti-aliasing enabled. */
    void lineClippedAntiAliased(const Point& p1, const Point& p2, Pixel color) throw();

    inline void pixelInternal(const Point& point, Pixel color) throw() {
      if ((point.getX() >= 0) && (point.getY() >= 0) &&
          (point.getX() < dimension.getWidth()) &&
          (point.getY() < dimension.getHeight())) {
        rows[point.getY()][point.getX()] = color;
      }
    }

    inline Pixel getPixelInternal(const Point& point) const throw() {
      if ((point.getX() >= 0) && (point.getY() >= 0) &&
          (point.getX() < dimension.getWidth()) &&
          (point.getY() < dimension.getHeight())) {
        return rows[point.getY()][point.getX()];
      }
    }
  public:

    /** Drawing options. */
    enum Option {
      /** Enables filling. */
      FILL = 1,
      /** Enables anti-aliasing. */
      ANTIALIASING = 2
    };

    /**
      Initializes the canvas for drawing onto the specified color image.

      @param canvas The image to draw onto.
    */
    Canvas(ColorImage* canvas) throw();

    /**
      Returns the dimension of the canvas.
    */
    Dimension getDimension() const throw();

    /**
      Clips the line specified by the end points such that it fits onto the canvas.

      @param p1 The first end point.
      @param p2 The second end point.
      @return False if line was rejected.
    */
    bool clip(Point& p1, Point& p2) const throw();

    /**
      Draws a image onto the canvas at the specified offset.

      @param offset The offset of the image.
      @param image The image to be drawn.
    */
    void image(const Point& offset, const ColorImage& image) throw();

    /**
      Draws a circle. This methods accepts the options FILL and ANTIALIASING.

      @param center The center of the circle.
      @param radius The radius of the circle.
      @param color The color of the circle.
      @param options The options. The default is 0.
    */
    void circle(
      const Point& center,
      unsigned int radius,
      Pixel color,
      unsigned int options = 0) throw();

    /**
      Draws a disk filled with the specified color. This methods accepts the
      ANTIALIASING option. This method simply invokes the circle method with
      FILL enabled.

      @param center The center of the circle.
      @param radius The radius of the circle.
      @param color The color of the circle.
      @param options The options. The default is 0.
    */
    inline void disk(const Point& center, unsigned int radius, Pixel color, unsigned int options = 0) throw() {
      circle(center, radius, color, options | FILL);
    }

    /**
      Draws a ring. This methods accepts the option ANTIALIASING.

      @param center The center of the circle.
      @param innerRadius The radius of the inner circle.
      @param outerRadius The radius of the outer circle.
      @param color The color of the circle.
      @param options The options. The default is 0.
    */
    void ring(
      const Point& center,
      unsigned int innerRadius,
      unsigned int outerRadius,
      Pixel color,
      unsigned int options = 0) throw();

    /**
      Draws an ellipse. This methods accepts the options FILL and ANTIALIASING.

      @param center The center of the ellipse.
      @param dimension The half dimension of the ellipse.
      @param color The color of the ellipse.
      @param options The options. The default is 0.
    */
    void ellipse(
      const Point& center,
      const Dimension& dimension,
      Pixel color,
      unsigned int options = 0) throw();

    /**
      Draws a rectangle. This methods accepts the option FILL.

      @param p1 The first corner.
      @param p2 The second corner.
      @param color The fill color.
      @param options Drawing options. The default is 0.
    */
    void rectangle(
      const Point& p1,
      const Point& p2,
      Pixel color,
      unsigned int options = 0) throw();

    /**
      Draws a rectangular box filled with the specified color. This method
      simply invokes the rectangle method with FILL enabled.

      @param p1 The first corner.
      @param p2 The second corner.
      @param color The fill color.
      @param options Drawing options. The default is 0.
    */
    inline void fill(
      const Point& p1,
      const Point& p2,
      Pixel color,
      unsigned int options = 0) throw() {
      rectangle(p1, p2, color, options | FILL);
    }

    /**
      Draw pixel at the specified point.

      @param point The point.
      @param color The color of the pixel.
    */
    void pixel(const Point& point, Pixel color) throw();

    /**
      Returns the pixel at the specified point.

      @param point The position of the point.
      @return The value is unspecified if the point is outside the dimension of the canvas.
    */
    Pixel getPixel(const Point& point) const throw();

    /**
      Draw line for p1 to p2 with the specified color. This methods accepts the
      options FILL and ANTIALIASING.

      @param p1 The first end point.
      @param p2 The second end point.
      @param color The color of the line.
      @param options Drawing options. The default is 0.
    */
    void line(
      const Point& p1,
      const Point& p2,
      Pixel color,
      unsigned int options = 0) throw();
    
    void line(
      const Point& p1,
      const Point& p2,
      ColorAlphaPixel color,
      unsigned int options = 0) throw();

    /**
      Select the font for used when writing text.

      @param name The identifier of the font.
    */
    void setFont(const String& name) throw();

    /**
      Returns the name of the current font.
    */
    String getFont() const throw();
    
    /**
      Returns the dimension of the specified message using the active font.
      Nothing is written to the canvas.
      
      @param message The message.
    */
    Dimension getDimensionOfText(const String& message) const throw();
    
    /**
      Writes the message to the canvas at the current raster position.

      @param message The message to be written.
    */
//    void write(const String& message) throw();
    
    /**
      Writes the message to the canvas at the specified raster position.

      @param position Specifies where to write the message.
      @param message The message to be written.
    */
    void write(const Point& position, const String& message) throw();
  };

}; // end of gip namespace
