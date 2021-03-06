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

//  class _COM_AZURE_DEV__GIP__API Font : public Object {
//  public:
//  };
//
//  class _COM_AZURE_DEV__GIP__API BitmapFont : public Font {
//  public:
//  };

  /**
    This class supports the drawing of primitives onto an image.

    @short Canvas
    @version 1.0
  */

  class _COM_AZURE_DEV__GIP__API Canvas {
  private:

    /** The pixel type of the canvas. */
    typedef ColorImage::Pixel Pixel;

    /** The image to draw onto. */
    ColorImage* canvas = nullptr;
    /** The dimension of the canvas. */
    Dimension dimension;
    /** The rows. */
    ColorImage::Rows rows;
    /** The name of the active font. */
    String fontName;

    /** Draws a clipped line onto the canvas. */
    void lineClipped(const Point& p1, const Point& p2, Pixel color) noexcept;
    /** Draws a clipped line onto the canvas with anti-aliasing enabled. */
    void lineClippedAntiAliased(const Point& p1, const Point& p2, Pixel color) noexcept;

    /** Returns true if the point is inside the rectangle from (0, 0) and to Dimension. */
    static inline bool isPointInsideDimension(const Point& point, const Dimension& dimension) noexcept {
      return (point.getX() >= 0) && (point.getY() >= 0) &&
        (static_cast<unsigned int>(point.getX()) < dimension.getWidth()) &&
        (static_cast<unsigned int>(point.getY()) < dimension.getHeight());
    }

    inline void pixelInternal(const Point& point, Pixel color) noexcept {
      if (isPointInsideDimension(point, dimension)) {
        rows[point.getY()][point.getX()] = color;
      }
    }

    inline Pixel getPixelInternal(const Point& point) const noexcept {
      if (isPointInsideDimension(point, dimension)) {
        return rows[point.getY()][point.getX()];
      }
      return Pixel();
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
    Canvas(ColorImage* canvas) noexcept;

    /**
      Returns the dimension of the canvas.
    */
    Dimension getDimension() const noexcept;

    /**
      Clips the line specified by the end points such that it fits onto the canvas.

      @param p1 The first end point.
      @param p2 The second end point.
      @return False if line was rejected.
    */
    bool clip(Point& p1, Point& p2) const noexcept;

    /**
      Draws a image onto the canvas at the specified offset.

      @param offset The offset of the image.
      @param image The image to be drawn.
    */
    void image(const Point& offset, const ColorImage& image) noexcept;

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
      unsigned int options = 0) noexcept;

    /**
      Draws a disk filled with the specified color. This methods accepts the
      ANTIALIASING option. This method simply invokes the circle method with
      FILL enabled.

      @param center The center of the circle.
      @param radius The radius of the circle.
      @param color The color of the circle.
      @param options The options. The default is 0.
    */
    inline void disk(const Point& center, unsigned int radius, Pixel color, unsigned int options = 0) noexcept {
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
      unsigned int options = 0) noexcept;

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
      unsigned int options = 0) noexcept;

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
      unsigned int options = 0) noexcept;

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
      unsigned int options = 0) noexcept {
      rectangle(p1, p2, color, options | FILL);
    }

    /**
      Draw pixel at the specified point.

      @param point The point.
      @param color The color of the pixel.
    */
    void pixel(const Point& point, Pixel color) noexcept;

    /**
      Returns the pixel at the specified point.

      @param point The position of the point.
      @return The value is unspecified if the point is outside the dimension of the canvas.
    */
    Pixel getPixel(const Point& point) const noexcept;

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
      unsigned int options = 0) noexcept;
    
    void line(
      const Point& p1,
      const Point& p2,
      ColorAlphaPixel color,
      unsigned int options = 0) noexcept;

    /**
      Select the font for used when writing text.

      @param name The identifier of the font.
    */
    void setFont(const String& name) noexcept;

    /**
      Returns the name of the current font.
    */
    String getFont() const noexcept;
    
    /**
      Returns the dimension of the specified message using the active font.
      Nothing is written to the canvas.
      
      @param message The message.
    */
    Dimension getDimensionOfText(const String& message) const noexcept;
    
    /**
      Writes the message to the canvas at the current raster position.

      @param message The message to be written.
    */
//    void write(const String& message) noexcept;
    
    /**
      Writes the message to the canvas at the specified raster position.

      @param position Specifies where to write the message.
      @param message The message to be written.
    */
    void write(const Point& position, const String& message) noexcept;
  };

}; // end of gip namespace
