/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <base/Application.h>
#include <base/Version.h>
#include <base/math/Vector3D.h>
#include <base/math/Matrix4x4.h>
#include <base/opengl/OpenGLContext.h>
#include <base/string/FormatOutputStream.h>
#include <base/string/StringOutputStream.h>
#include <base/ui/MessageDialog.h>
#include <base/ui/OpenFileDialog.h>
#include <base/ui/SaveFileDialog.h>
#include <gip/Version.h>
#include <gip/io/BMPEncoder.h>
#include <gip/io/JPEGEncoder.h>
#include <gip/io/PCXEncoder.h>
#include <gip/io/PNGEncoder.h>
#include <gip/io/TGAEncoder.h>
#include <gip/transformation/Convert.h>
#include <gip/transformation/LinearScale.h>

// TAG: remove onIdle from WindowImpl
// TAG: need IdleListener class for use with dispatch?
// TAG: need TimerListener?

using namespace com::azure::dev::gip;

/** Verbosity levels. */
class Verbosity {
public:
      
  enum Value {
    NO_INFORMATION,
    WARNINGS,
    NORMAL,
    COMMANDS,
    ACTIVE_MOUSE_EVENTS,
    ALL_MOUSE_EVENTS,
    EVERYTHING,
    DEFAULT = EVERYTHING // NORMAL
  };
};

/** Internal commands. */
class Command {
public:

  enum {
    NOTHING,
        
    SELECT_VERBOSITY_NO_INFORMATION,
    SELECT_VERBOSITY_WARNINGS,
    SELECT_VERBOSITY_NORMAL,
    SELECT_VERBOSITY_COMMANDS,
    SELECT_VERBOSITY_ACTIVE_MOUSE_EVENTS,
    SELECT_VERBOSITY_ALL_MOUSE_EVENTS,
    SELECT_VERBOSITY_EVERYTHING,
    
    OPEN_IMAGE,
    SAVE_IMAGE,
    
    RENDER_DEFAULT,
    RENDER_INTENSITY,
    RENDER_RGB_CLOUD,
    RESET_VIEW_PARAMETERS,
    SHOW_OPENGL_INFORMATION, /** Show OpenGL specific information. */
    HELP, /** Show help information. */
    ABOUT, /**< Show information about the application. */
    QUIT, /**< Quit the application. */
    
    SELECT_LOW_DETAIL,
    SELECT_MEDIUM_DETAIL,
    SELECT_HIGH_DETAIL,
        
    SHOW_FRAME_RATE,
        
    CYCLE_DISPLAY_MODE,
        
    CYCLE_SHADING_MODEL,
    SELECT_SHADING_MODEL_FLAT,
    SELECT_SHADING_MODEL_SMOOTH,
        
    CYCLE_POLYGON_MODE,
    SELECT_POLYGON_MODE_FILL,
    SELECT_POLYGON_MODE_LINE,
    SELECT_POLYGON_MODE_POINT,
        
    BLENDING_ENABLE,
    BLENDING_DISABLE,
        
    LIGHTING_ENABLE,
    LIGHTING_DISABLE,
        
    ROTATE_AROUND_X_AXIS_NEG,
    ROTATE_AROUND_X_AXIS_POS,
    ROTATE_AROUND_Y_AXIS_NEG,
    ROTATE_AROUND_Y_AXIS_POS,
    ROTATE_AROUND_Z_AXIS_NEG,
    ROTATE_AROUND_Z_AXIS_POS,
        
    TRANSLATE_ALONG_X_NEG,
    TRANSLATE_ALONG_X_POS,
    TRANSLATE_ALONG_Y_NEG,
    TRANSLATE_ALONG_Y_POS,
    TRANSLATE_ALONG_Z_NEG,
    TRANSLATE_ALONG_Z_POS,

    SELECT_MODE_DEFAULT,
    SELECT_MODE_SYSTEM,
    SELECT_MODE_FLOOR,
    SELECT_MODE_CUBE,
    SELECT_MODE_CONE,
    SELECT_MODE_TORUS,
    SELECT_MODE_INTENSITY,
    SELECT_MODE_RGB_CLOUD,

    SELECT_QUALITY_WORST,
    SELECT_QUALITY_NORMAL,
    SELECT_QUALITY_BEST
  };
};
    
/** The supported shading models. */
class ShadingModel {
public:
      
  enum Model {
    FLAT,
    SMOOTH,
    FIRST_MODEL = FLAT,
    LAST_MODEL = SMOOTH,
    DEFAULT = FLAT
  };
};
    
/** The supported polygon modes. */
class PolygonMode {
public:

  enum Mode {
    FILL,
    LINE,
    POINT,
    FIRST_MODE = FILL,
    LAST_MODE = POINT,
    DEFAULT = FILL
  };
};

/** The rendering quality. */
enum Quality {
  QUALITY_WORST,
  QUALITY_NORMAL,
  QUALITY_BEST
};

/** The rendering mode. */
enum Mode {
  MODE_BLACKNESS,
  MODE_SYSTEM,
  MODE_FLOOR,
  MODE_CUBE,
  MODE_CONE,
  MODE_TORUS,
  MODE_INTENSITY,
  MODE_RGB_CLOUD,
  MODE_DEFAULT = MODE_SYSTEM /**< The default mode. */
};

/** The display mode. */
enum DisplayMode {
  DISPLAY_MODE_WINDOW,
  DISPLAY_MODE_FULL_SCREEN,
  DISPLAY_MODE_FIRST = DISPLAY_MODE_WINDOW,
  DISPLAY_MODE_LAST = DISPLAY_MODE_FULL_SCREEN
};

class MyMenu : public Menu {
public:
  
  class PolygonModeMenu : public Menu {
  public:
    
    PolygonModeMenu() throw(UserInterfaceException) {
      append(MESSAGE("&Fill\tf"), Command::SELECT_POLYGON_MODE_FILL);
      append(MESSAGE("&Line\tl"), Command::SELECT_POLYGON_MODE_LINE);
      append(MESSAGE("&Point\tp"), Command::SELECT_POLYGON_MODE_POINT);
    }
  };
  
  class ShadingModelMenu : public Menu {
  public:
    
    ShadingModelMenu() throw(UserInterfaceException) {
      append(MESSAGE("&Flat\tf"), Command::SELECT_SHADING_MODEL_FLAT);
      append(MESSAGE("&Smooth\ts"), Command::SELECT_SHADING_MODEL_SMOOTH);
    }
  };
  
  class ModeMenu : public Menu {
  public:
    
    ModeMenu() throw(UserInterfaceException) {
      append(MESSAGE("&System"), Command::SELECT_MODE_SYSTEM);
      append(MESSAGE("&Floor"), Command::SELECT_MODE_FLOOR);
      append(MESSAGE("&Cube"), Command::SELECT_MODE_CUBE);
      append(MESSAGE("C&one"), Command::SELECT_MODE_CONE);
      append(MESSAGE("&Torus"), Command::SELECT_MODE_TORUS);
      append(MESSAGE("&Image"), Command::SELECT_MODE_INTENSITY);
      append(MESSAGE("&RGB Cloud"), Command::SELECT_MODE_RGB_CLOUD);
    }
  };
  
  class QualityMenu : public Menu {
  public:
    
    QualityMenu() throw(UserInterfaceException) {
      append(MESSAGE("&Worst"), Command::SELECT_QUALITY_WORST);
      append(MESSAGE("&Normal"), Command::SELECT_QUALITY_NORMAL);
      append(MESSAGE("&Best"), Command::SELECT_QUALITY_BEST);
    }
  };
  
  class VerbosityMenu : public Menu {
  public:
    
    VerbosityMenu() throw(UserInterfaceException) {
      append(MESSAGE("&No information"), Command::SELECT_VERBOSITY_NO_INFORMATION);
      append(MESSAGE("&Warnings"), Command::SELECT_VERBOSITY_WARNINGS);
      append(MESSAGE("&Normal"), Command::SELECT_VERBOSITY_NORMAL);
      append(MESSAGE("&Commands"), Command::SELECT_VERBOSITY_COMMANDS);
      append(MESSAGE("&Active mouse events"), Command::SELECT_VERBOSITY_ACTIVE_MOUSE_EVENTS);
      append(MESSAGE("&All mouse events"), Command::SELECT_VERBOSITY_ALL_MOUSE_EVENTS);
      append(MESSAGE("&Everything"), Command::SELECT_VERBOSITY_EVERYTHING);
    }
  };
private:
  
  PolygonModeMenu polygonModeMenu;
  ShadingModelMenu shadingModelMenu;
  ModeMenu modeMenu;
  QualityMenu qualityMenu;
  VerbosityMenu verbosityMenu;
public:
  
  MyMenu() throw(UserInterfaceException) {
    append(MESSAGE("&Open image...\t(C-o)"), Command::OPEN_IMAGE);
    append(MESSAGE("&Save image...\t(C-s)"), Command::SAVE_IMAGE);
    appendMenu(MESSAGE("S&hading model"), shadingModelMenu);
    appendMenu(MESSAGE("&Polygon mode"), polygonModeMenu);
    appendMenu(MESSAGE("&Mode"), modeMenu);
    appendMenu(MESSAGE("&Quality"), qualityMenu);
    appendMenu(MESSAGE("&Verbosity"), verbosityMenu);
    append(MESSAGE("OpenGL information\t(C-a)"), Command::SHOW_OPENGL_INFORMATION);
    append(MESSAGE("&About\t(C-a)"), Command::ABOUT);
    append(MESSAGE("&Reset\t(C-r)"), Command::RESET_VIEW_PARAMETERS);
    append(MESSAGE("&Quit\t(C-q)"), Command::QUIT);
  }
};

/** 3D view specification (orientation, translation, ...). */
class View {
public:
    
  Vector3D<long double> translation;
  Vector3D<long double> rotation;
  long double scale;
public:
    
  inline View() noexcept {
    resetParameters();
  }
      
  inline void resetParameters() noexcept {
    setTranslation(Vector3D<long double>(0, 0, 0));
    setRotation(Vector3D<long double>(0, 0, 0));
    setScale(1);
  }
    
  inline Vector3D<long double> getTranslation() const noexcept {
    return translation;
  }
    
  inline void setTranslation(const Vector3D<long double>& translation) noexcept {
    this->translation = translation;
  }
    
  inline Vector3D<long double> getRotation() const noexcept {
    return rotation;
  }
    
  inline void setRotation(const Vector3D<long double>& rotation) noexcept {
    this->rotation = rotation;
  }
    
  inline long double getScale() const noexcept {
    return scale;
  }
      
  inline void setScale(long double scale) noexcept {
    this->scale = scale;
  }
};

class EncoderRegistry : public Object {
private:
    
  Map<String, ImageEncoder*> encoders;
  Map<String, String> filters;
    
  BMPEncoder bmp;
  JPEGEncoder jpeg;
  PCXEncoder pcx;
  PNGEncoder png;
  TGAEncoder tga;
public:
      
  void addEncoder(ImageEncoder* encoder) noexcept {
    StringOutputStream stream;
    Array<String> extensions = encoder->getExtensions();
    Array<String>::ReadEnumerator enu = extensions.getReadEnumerator();
    while (enu.hasNext()) {
      const String* extension = enu.next();
      if (!encoders.hasKey(*extension)) {
        encoders[*extension] = encoder;
        stream << MESSAGE("*.") << *extension << ';' << FLUSH;
      }
    }
    String filter = stream.getString();
    filter.removeFrom(filter.getLength() - 1); // remove last ';'
    filters[filter] = encoder->getDescription() + MESSAGE(" (") + filter + MESSAGE(")");
  }
      
  EncoderRegistry() noexcept {
    filters[MESSAGE("*.*")] = MESSAGE("All Files (*.*)"); // TAG: dialog window should support this
    addEncoder(&bmp);
    addEncoder(&jpeg);
    addEncoder(&pcx);
    addEncoder(&png);
    addEncoder(&tga);
  }
      
  Map<String, String> getFilters() const noexcept {
    return filters;
  }
      
  ImageEncoder* getEncoderByName(const String& extension) noexcept {
    if (!encoders.hasKey(extension)) {
      return 0;
    }
    ImageEncoder* encoder = encoders[extension];
    return encoder;
  }
      
  ImageEncoder* getEncoder(const String& filename) noexcept {
    int index = filename.lastIndexOf('.');
    if (index < 0) {
      return 0;
    }
    String extension = filename.substring(index + 1);
    if (!encoders.hasKey(extension)) {
      return 0;
    }
    ImageEncoder* encoder = encoders[extension];
    return encoder;
  }
};

/** An OpenGL renderable object. */
class Renderable {
public:
  
  virtual void update(const View& view) noexcept = 0;
  
  virtual ~Renderable() noexcept {
  }
};

class RenderIntensity : public Renderable {
private:

  OpenGL& openGL;
  ColorImage image;
  OpenGL::ReserveDisplayLists displayLists;
  Dimension dimension;
  long double amplitude;
  bool mono;
  unsigned int objectOffset;
  
  class Object {
  public:
    
    enum {
      INTENSITY
    };
  };
public:

  RenderIntensity(OpenGL& _openGL, const ColorImage& _image) noexcept
    : openGL(_openGL),
      image(_image),
      displayLists(_openGL, 1) {
    dimension = image.getDimension();
    amplitude = 1.0;
    mono = false;
    
//     openGL.glEnable(OpenGL::RESCALE_NORMAL);
//     openGL.glEnable(OpenGL::NORMALIZE);

    makeIntensity();
  }

  void makeIntensity() noexcept {
    const Dimension dimension = image.getDimension();
    // ColorImage* scaled = new ColorImage(dimension);
    // LinearScale scale(scaled, &image);
    // scale();
    
    // GrayImage* gray = new GrayImage(scaled->getDimension());
    // Convert<GrayImage, ColorImage, RGBToGray> transform(gray, scaled, RGBToGray());
    // transform();
    
    const ColorPixel* src = image.getElements();
    const ColorPixel* srcNextRow = src + dimension.getWidth();
    const double xOffset = -0.5 * dimension.getWidth();
    const double yOffset = -0.5 * dimension.getHeight();
    {
      OpenGL::DisplayList displayList(openGL, displayLists.getOffset() + Object::INTENSITY);
      
      if (dimension.getHeight() == 0) {
        return;
      }
      
      for (unsigned int y = 0; y < (dimension.getHeight() - 1); ++y) {
        OpenGL::Block block(openGL, OpenGL::TRIANGLE_STRIP);
        for (unsigned int x = 0; x < dimension.getWidth(); ++x) {
          Vector3D<long double> ac(
            0,
            1,
            convertPixel<GrayPixel, ColorPixel>(*srcNextRow)/16.0 -
            convertPixel<GrayPixel, ColorPixel>(*src)/16.0
          );
          Vector3D<long double> bc(
            1,
            0,
            convertPixel<GrayPixel, ColorPixel>(src[1])/16.0 -
            convertPixel<GrayPixel, ColorPixel>(*src)/16.0
          );
          Vector3D<long double> normal = ac.cross(bc);
          normal /= normal.getModulus();
          normal = Vector3D<long double>(0, 0, 1); // TAG: temp fix
          openGL.glNormal3f(normal.getX(), normal.getY(), normal.getZ());
          
          openGL.glColor4ub(
            src->red,
            src->green,
            src->blue,
            convertPixel<GrayPixel, ColorPixel>(*src)
          );
          openGL.glVertex3f(
            x + xOffset,
            y + yOffset,
            convertPixel<GrayPixel, ColorPixel>(*src)/16.0
          );
          openGL.glColor4ub(
            srcNextRow->red,
            srcNextRow->green,
            srcNextRow->blue,
            convertPixel<GrayPixel, ColorPixel>(*srcNextRow)
          );
          openGL.glVertex3f(
            x + xOffset,
            y+1 + yOffset,
            convertPixel<GrayPixel, ColorPixel>(*srcNextRow)/16.0
          );
          ++src;
          openGL.glColor4ub(
            src->red,
            src->green,
            src->blue,
            convertPixel<GrayPixel, ColorPixel>(*src)
          );
          openGL.glVertex3f(
            x+1 + xOffset,
            y + yOffset,
            convertPixel<GrayPixel, ColorPixel>(*src)/16.0
          );
          
          Vector3D<long double> ac2(
            -1,
            0,
            convertPixel<GrayPixel, ColorPixel>(srcNextRow[1])/16.0 -
            convertPixel<GrayPixel, ColorPixel>(*srcNextRow)/16.0
          );
          Vector3D<long double> bc2(
            -1,
            -1,
            convertPixel<GrayPixel, ColorPixel>(srcNextRow[1])/16.0 -
            convertPixel<GrayPixel, ColorPixel>(*src)/16.0
          );
          Vector3D<long double> normal2 = ac2.cross(bc2);
          normal2 /= normal2.getModulus();
          normal2 = Vector3D<long double>(0, 0, 1); // TAG: temp fix
          openGL.glNormal3f(normal2.getX(), normal2.getY(), normal2.getZ());
          
          openGL.glColor4ub(
            srcNextRow->red,
            srcNextRow->green,
            srcNextRow->blue,
            convertPixel<GrayPixel, ColorPixel>(*srcNextRow)
          );
          openGL.glVertex3f(
            x + xOffset,
            y+1 + yOffset,
            convertPixel<GrayPixel, ColorPixel>(*srcNextRow)/16.0
          );
          ++srcNextRow;
          openGL.glColor4ub(
            src->red,
            src->green,
            src->blue,
            convertPixel<GrayPixel, ColorPixel>(*src)
          );
          openGL.glVertex3f(
            x+1 + xOffset,
            y + yOffset,
            convertPixel<GrayPixel, ColorPixel>(*src)/16.0
          );
          openGL.glColor4ub(
            srcNextRow->red,
            srcNextRow->green,
            srcNextRow->blue,
            convertPixel<GrayPixel, ColorPixel>(*srcNextRow)
          );
          openGL.glVertex3f(
            x+1 + xOffset,
            y+1 + yOffset,
            convertPixel<GrayPixel, ColorPixel>(*srcNextRow)/16.0
          );
        }
      }
    }
  }

  void setDimension(const Dimension& dimension) noexcept {
    this->dimension = dimension;
    makeIntensity();
  }
  
  void setAmplitude(long double amplitude) noexcept {
    this->amplitude = amplitude;
    makeIntensity();
  }
  
  void setMode() noexcept {
    mono = true;
    makeIntensity();
  }
  
  void update(const View& view) noexcept {
    openGL.glClearColor(0.0, 0.0, 0.0, 1.0);
    openGL.glClear(OpenGL::COLOR_BUFFER_BIT | OpenGL::DEPTH_BUFFER_BIT);
    
    openGL.glMatrixMode(OpenGL::MODELVIEW);
    openGL.glLoadIdentity();
    Vector3D<long double> translation = view.getTranslation();
    openGL.glTranslatef(translation.getX(), translation.getY(), translation.getZ());
    Vector3D<long double> rotation = view.getRotation();
    openGL.glRotatef(rotation.getX(), 0.0, 1.0, 0.0);
    openGL.glRotatef(rotation.getY(), 1.0, 0.0, 0.0);
    openGL.glRotatef(rotation.getZ(), 0.0, 0.0, 1.0);
    long double scale = amplitude * view.getScale() * 0.5;
    openGL.glScalef(scale, scale, scale);
    
    // displayLists.execute();
    openGL.glCallList(displayLists.getOffset() + Object::INTENSITY);
  }
  
  ~RenderIntensity() noexcept {
  }
};

class RenderRGBCloud : public Renderable {
private:

  OpenGL& openGL;
  ColorImage image;
  OpenGL::ReserveDisplayLists displayLists;
  
  class Object {
  public:
    
    enum {
      RGB_CUBE,
      RGB_CLOUD
    };
  };
public:

  RenderRGBCloud(OpenGL& _openGL, const ColorImage& _image) noexcept
    : openGL(_openGL),
      image(_image),
      displayLists(_openGL, 2) {
    
    openGL.glLineWidth(1.5);
    openGL.glPointSize(4.0);
    openGL.glPolygonMode(OpenGL::FRONT, OpenGL::LINE);
    openGL.glPolygonMode(OpenGL::BACK, OpenGL::LINE);
    openGL.glShadeModel(OpenGL::SMOOTH);
    openGL.glBlendFunc(OpenGL::SRC_ALPHA, OpenGL::ONE_MINUS_SRC_ALPHA);
    
    buildCube();
    buildCloud();
  }

  void buildCube() noexcept {
    OpenGL::DisplayList displayList(openGL, displayLists.getOffset() + Object::RGB_CUBE);
    
    // RGB cube
    openGL.glBegin(OpenGL::LINE_LOOP);
    openGL.glColor3ub(0, 0, 0);
    openGL.glVertex3f(-1, -1, -1);
    openGL.glColor3ub(255,0,0);
    openGL.glVertex3f(1, -1, -1);
    openGL.glColor3ub(255, 255, 0);
    openGL.glVertex3f(1, 1, -1);
    openGL.glColor3ub(0, 255, 0);
    openGL.glVertex3f(-1, 1, -1);
    openGL.glEnd();
    
    openGL.glBegin(OpenGL::LINE_LOOP);
    openGL.glColor3ub(0, 0, 255);
    openGL.glVertex3f(-1, -1, 1);
    openGL.glColor3ub(255, 0, 255);
    openGL.glVertex3f(1, -1, 1);
    openGL.glColor3ub(255, 255, 255);
    openGL.glVertex3f(1, 1, 1);
    openGL.glColor3ub(0, 255, 255);
    openGL.glVertex3f(-1, 1, 1);
    openGL.glEnd();
    
    openGL.glBegin(OpenGL::LINES);
    openGL.glColor3ub(0, 0, 0);
    openGL.glVertex3f(-1, -1, -1);
    openGL.glColor3ub(0, 0, 255);
    openGL.glVertex3f(-1, -1, 1);
    openGL.glColor3ub(255, 0, 0);
    openGL.glVertex3f(1, -1, -1);
    openGL.glColor3ub(255, 0, 255);
    openGL.glVertex3f(1, -1, 1);
    openGL.glColor3ub(255, 255, 0);
    openGL.glVertex3f(1, 1, -1);
    openGL.glColor3ub(255, 255, 255);
    openGL.glVertex3f(1, 1, 1);
    openGL.glColor3ub(0, 255, 0);
    openGL.glVertex3f(-1, 1, -1);
    openGL.glColor3ub(0, 255, 255);
    openGL.glVertex3f(-1, 1, 1);
    openGL.glEnd();
  }

  class CloudOperation {
  private:
    
    OpenGL& openGL;
    Allocator<unsigned long> lookup;
    unsigned long* fastLookup;
  public:
    
    CloudOperation(OpenGL& _openGL) noexcept
      : openGL(_openGL),
        lookup(256 * 256 * 256/sizeof(unsigned long)) {      
      fill<unsigned long>(lookup.getElements(), 256 * 256 * 256/sizeof(unsigned long), 0);
      fastLookup = lookup.getElements();
    }
    
    inline void operator()(const ColorPixel& value) noexcept {
      unsigned int colorIndex = value.rgb & 0xffffff;
      
      unsigned long* colorGroup = fastLookup + colorIndex/sizeof(unsigned long);
      unsigned long colorMask = 1 << (colorIndex % sizeof(unsigned long));
      
      if ((*colorGroup & colorMask) == 0) {
        *colorGroup |= colorMask;
        openGL.glColor4ub(
          value.red,
          value.green,
          value.blue,
          128 // (static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3
        );
        openGL.glVertex3f(
          -1 + value.red * 2.0/255.0,
          -1 + value.green * 2.0/255.0,
          -1 + value.blue * 2.0/255.0
        );
        // TAG: add noise < 0.5
      }
    }
  };
  
  void buildCloud() noexcept {
    CloudOperation cloudOperation(openGL);
    OpenGL::DisplayList displayList(openGL, displayLists.getOffset() + Object::RGB_CLOUD);
    
    // OpenGL::Block(openGL, OpenGL::POINTS);
    openGL.glBegin(OpenGL::POINTS);      
    forEach(image, cloudOperation);  
    openGL.glEnd();
  }

  void onCommand(unsigned int command) noexcept {
    // TAG: fixme
  }
  
  void update(const View& view) noexcept {
    openGL.glClearColor(0.3f, 0.3f, 0.6f, 1.0f);
    openGL.glClear(OpenGL::COLOR_BUFFER_BIT | OpenGL::DEPTH_BUFFER_BIT);
    
    openGL.glMatrixMode(OpenGL::MODELVIEW);
    openGL.glLoadIdentity();
    Vector3D<long double> translation = view.getTranslation();
    openGL.glTranslatef(translation.getX(), translation.getY(), translation.getZ());
    Vector3D<long double> rotation = view.getRotation();
    openGL.glRotatef(rotation.getX(), 0.0, 1.0, 0.0);
    openGL.glRotatef(rotation.getY(), 1.0, 0.0, 0.0);
    openGL.glRotatef(rotation.getZ(), 0.0, 0.0, 1.0);
    openGL.glScalef(view.getScale() * 0.5, view.getScale() * 0.5, view.getScale() * 0.5);
    
    openGL.glDisable(OpenGL::BLEND);
    openGL.glCallList(displayLists.getOffset() + Object::RGB_CLOUD); // render cloud
    openGL.glEnable(OpenGL::BLEND);
    
    openGL.glDepthMask(false);
    openGL.glCallList(displayLists.getOffset() + Object::RGB_CUBE); // render cube
    openGL.glDepthMask(true);
  }
  
  ~RenderRGBCloud() noexcept {
  }
};

class ViewerApplication : public Application {
private:
  
  static const unsigned int MAJOR_VERSION = 1;
  static const unsigned int MINOR_VERSION = 0;
public:
  
  ViewerApplication() noexcept
    : Application(MESSAGE("OpenGL Viewer")) {
  }
  
  class MyOpenGLContext : public OpenGLContext {
  private:

    /** Specifies the level of verbosity. */
    Verbosity::Value verbosity;
    MyMenu menu;
    DisplayMode displayMode;
    Mode mode;
    EncoderRegistry encoderRegistry;
    OpenFileDialog openFile;
    SaveFileDialog saveFile;
    ColorImage image;
    Renderable* renderable;
    View view;
    
    ShadingModel::Model shadingModel;
    PolygonMode::Mode polygonMode;
    bool blending;
    bool lighting;
    
    Vector3D<long double> translationBegin;
    Vector3D<long double> rotationBegin;
    long double scaleBegin;    
    
    long double orthoLeft;
    long double orthoRight;
    long double orthoBottom;
    long double orthoTop;
    long double orthoNear;
    long double orthoFar;
    
    Position mouseButtonPosition;
    bool mouseLeftButtonPressed;
    bool mouseMiddleButtonPressed;
    bool mouseRightButtonPressed;
    
    Vector3D<long double> drag;
    Matrix4x4<OpenGL::GLdouble> modelViewMatrix;
    Matrix4x4<OpenGL::GLdouble> invertedModelViewMatrix;
  public:

    /** Predefined objects. */
    enum Object {
      OBJECT_SYSTEM = 1,
      OBJECT_FLOOR,
      OBJECT_CUBE,
      OBJECT_CONE,
      OBJECT_TORUS
    };
    
    MyOpenGLContext(
      const String& title,
      const Position& position,
      const Dimension& dimension,
      const Format& format) throw(UserInterfaceException)
      : OpenGLContext(position, dimension, format),
        verbosity(Verbosity::DEFAULT),
        mode(MODE_DEFAULT),
        renderable(0) {
      
      setTitle(title);
      setIconTitle(title);
      
      openFile.setFilters(encoderRegistry.getFilters());
      openFile.setTitle(MESSAGE("Open image..."));
      saveFile.setFilters(encoderRegistry.getFilters());
      saveFile.setTitle(MESSAGE("Save image..."));

      // clear initial image
      
      openGL.glPolygonMode(OpenGL::FRONT_AND_BACK, OpenGL::FILL);
      
      static const OpenGL::GLfloat materialAmbient[] = {1.0, 1.0, 1.0, 1.0};
      static const OpenGL::GLfloat materialSpecular[] = {0.25, 0.25, 0.25, 0.25};
      static const OpenGL::GLfloat materialShininess[] = {10.0};
      static const OpenGL::GLfloat materialEmission[] = {1.0, 0.0, 0.0, 0.0};
        
      openGL.glMaterialfv(OpenGL::FRONT_AND_BACK, OpenGL::AMBIENT, materialAmbient);
      openGL.glMaterialfv(OpenGL::FRONT_AND_BACK, OpenGL::SPECULAR, materialSpecular);
      openGL.glMaterialfv(OpenGL::FRONT_AND_BACK, OpenGL::SHININESS, materialShininess);
      openGL.glMaterialfv(OpenGL::FRONT_AND_BACK, OpenGL::EMISSION, materialEmission);
      
      static const OpenGL::GLfloat lightModelAmbient[] = {0.0, 0.0, 0.0, 0.0};
      
      openGL.glLightModelfv(OpenGL::LIGHT_MODEL_AMBIENT, lightModelAmbient);
      openGL.glLightModeli(OpenGL::LIGHT_MODEL_LOCAL_VIEWER, false);
      openGL.glLightModeli(OpenGL::LIGHT_MODEL_TWO_SIDE, false);
      
      static const OpenGL::GLfloat lightAmbient[] = {0.5, 0.5, 0.5, 0.0};
      static const OpenGL::GLfloat lightDiffuse[] = {0.25, 0.25, 0.25, 1.0};
      static const OpenGL::GLfloat lightSpecular[] = {0.25, 0.25, 0.25, 1.0};
      static const OpenGL::GLfloat lightPosition[] = {10.0, 10.0, 10.0, 0.0};
      
      openGL.glLightfv(OpenGL::LIGHT0, OpenGL::AMBIENT, lightAmbient);
      openGL.glLightfv(OpenGL::LIGHT0, OpenGL::DIFFUSE, lightDiffuse);
      openGL.glLightfv(OpenGL::LIGHT0, OpenGL::SPECULAR, lightSpecular);
      openGL.glLightfv(OpenGL::LIGHT0, OpenGL::POSITION, lightPosition); // uses current model-view matrix
      openGL.glEnable(OpenGL::LIGHT0);

      openGL.glColorMaterial(OpenGL::BACK, OpenGL::AMBIENT);
      openGL.glColorMaterial(OpenGL::FRONT, OpenGL::EMISSION);
      openGL.glEnable(OpenGL::COLOR_MATERIAL);

      //openGL.glEnable(OpenGL::MULTISAMPLE); // OpenGL 1.3
      
      openGL.glEnable(OpenGL::DEPTH_TEST);
      // openGL.glDepthFunc(OpenGL::LEQUAL); // TAG: fixme

      mode = MODE_TORUS;
      setQuality(QUALITY_NORMAL);
      shadingModel = ShadingModel::FLAT;
      openGL.glShadeModel(OpenGL::FLAT);
      polygonMode = PolygonMode::FILL;
      openGL.glPolygonMode(OpenGL::FRONT_AND_BACK, OpenGL::FILL);
      blending = false;
      openGL.glDisable(OpenGL::BLEND);
      lighting = true;
      openGL.glEnable(OpenGL::LIGHTING);
      
      drag = Vector3D<long double>(0, 0, 0);
      
      mouseLeftButtonPressed = false;
      mouseMiddleButtonPressed = false;
      mouseRightButtonPressed = false;

      makeSystem();
      makeFloor();
      makeCube();
      makeCone();
      makeTorus();
    }

    ~MyOpenGLContext() noexcept {
    }
    
    void setQuality(Quality quality) noexcept {
      // TAG: need attribute
      switch (quality) {
      case QUALITY_WORST:
        // fastest
        openGL.glHint(OpenGL::PERSPECTIVE_CORRECTION_HINT, OpenGL::FASTEST);
        openGL.glHint(OpenGL::POINT_SMOOTH_HINT, OpenGL::FASTEST);
        openGL.glHint(OpenGL::LINE_SMOOTH_HINT, OpenGL::FASTEST);
        openGL.glHint(OpenGL::POLYGON_SMOOTH_HINT, OpenGL::FASTEST);
        openGL.glDisable(OpenGL::POINT_SMOOTH);
        openGL.glDisable(OpenGL::LINE_SMOOTH);
        openGL.glDisable(OpenGL::POLYGON_SMOOTH);
        break;
      case QUALITY_NORMAL:
        // ok
        openGL.glHint(OpenGL::PERSPECTIVE_CORRECTION_HINT, OpenGL::DONT_CARE);
        openGL.glHint(OpenGL::POINT_SMOOTH_HINT, OpenGL::DONT_CARE);
        openGL.glHint(OpenGL::LINE_SMOOTH_HINT, OpenGL::DONT_CARE);
        openGL.glHint(OpenGL::POLYGON_SMOOTH_HINT, OpenGL::DONT_CARE);
        openGL.glDisable(OpenGL::POINT_SMOOTH);
        openGL.glDisable(OpenGL::LINE_SMOOTH);
        openGL.glDisable(OpenGL::POLYGON_SMOOTH);
        break;
      case QUALITY_BEST:
        openGL.glHint(OpenGL::PERSPECTIVE_CORRECTION_HINT, OpenGL::NICEST);
        openGL.glHint(OpenGL::POINT_SMOOTH_HINT, OpenGL::NICEST);
        openGL.glHint(OpenGL::LINE_SMOOTH_HINT, OpenGL::NICEST);
        openGL.glHint(OpenGL::POLYGON_SMOOTH_HINT, OpenGL::NICEST);
        openGL.glEnable(OpenGL::POINT_SMOOTH);
        openGL.glEnable(OpenGL::LINE_SMOOTH);
        openGL.glEnable(OpenGL::POLYGON_SMOOTH);
        break;
      }
      invalidate();
    }

    inline void setVerbosity(Verbosity::Value verbosity) noexcept {
      this->verbosity = verbosity;
    }
    
    void setTranslation(const Vector3D<long double>& translation) noexcept {
      view.setTranslation(translation);
      invalidate();
    }
    
    void setRotation(const Vector3D<long double>& rotation) noexcept {
      view.setRotation(rotation);
      invalidate();
    }
      
    void setScale(long double scale) noexcept {
      view.setScale(scale);
      invalidate();
    }
    
    void resetViewParameters() noexcept {
      view.setTranslation(Vector3D<long double>(0, 0, 0));
      view.setRotation(Vector3D<long double>(0, 0, 0));
      view.setScale(1);
      invalidate();
    }
    
    void setShadingModel(ShadingModel::Model shadingModel) noexcept {
      if (shadingModel != this->shadingModel) {
        this->shadingModel = shadingModel;
        switch (shadingModel) {
        case ShadingModel::SMOOTH:
          openGL.glShadeModel(OpenGL::SMOOTH);
          break;
        case ShadingModel::FLAT:
          openGL.glShadeModel(OpenGL::FLAT);
          break;
        }
        invalidate();
      }
    }
    
    void setPolygonMode(PolygonMode::Mode polygonMode) noexcept {
      if (polygonMode != this->polygonMode) {
        this->polygonMode = polygonMode;
        switch (polygonMode) {
        case PolygonMode::FILL:
          openGL.glPolygonMode(OpenGL::FRONT_AND_BACK, OpenGL::FILL);
          break;
        case PolygonMode::LINE:
          openGL.glPolygonMode(OpenGL::FRONT_AND_BACK, OpenGL::LINE);
          break;
        case PolygonMode::POINT:
          openGL.glPolygonMode(OpenGL::FRONT_AND_BACK, OpenGL::POINT);
          break;
        }
        invalidate();
      }
    }
    
    void setBlending(bool blending) noexcept {
      if (blending != this->blending) {
        this->blending = blending;
        if (blending) {
          openGL.glEnable(OpenGL::BLEND);
          openGL.glBlendColor(1.0, 1.0, 1.0, 0.5);
          openGL.glBlendFunc(OpenGL::SRC_ALPHA, OpenGL::ONE);
        } else {
          openGL.glDisable(OpenGL::BLEND);
        }
        invalidate();
      }
    }
    
    void setLighting(bool lighting) noexcept {
      if (lighting != this->lighting) {
        this->lighting = lighting;
        if (lighting) {
          openGL.glEnable(OpenGL::LIGHTING);
        } else {
          openGL.glDisable(OpenGL::LIGHTING);
        }
        invalidate();
      }
    }
    
    void setMode(Mode mode) noexcept {
      if (renderable) {
        delete renderable;
      }
      renderable = 0;
      switch (mode) {
      case MODE_INTENSITY:
        setRenderable(new RenderIntensity(openGL, image));
        break;
      case MODE_RGB_CLOUD:
        setRenderable(new RenderRGBCloud(openGL, image));
        break;
      default:
        break;
      }
      this->mode = mode;
      invalidate();
    }
    
    /** Maps the (x,y)-position into world coordinates. */
    Vector3D<long double> mapXYToWorld(const Position& position, const int viewPort[4]) const noexcept {
      long double tempX =
        static_cast<long double>(position.getX() - viewPort[0])/
        static_cast<long double>(viewPort[2]);
      long double tempY =
        static_cast<long double>(position.getY() - viewPort[1])/
        static_cast<long double>(viewPort[3]);
      return Vector3D<long double>(
        orthoLeft + tempX * (orthoRight - orthoLeft),
        orthoTop + tempY * (orthoBottom - orthoTop),
        orthoNear
      );
    }
    
    void makeSystem() noexcept {
      OpenGL::DisplayList displayList(openGL, OBJECT_SYSTEM);

      openGL.glColorMaterial(OpenGL::BACK, OpenGL::AMBIENT);
      openGL.glColorMaterial(OpenGL::FRONT, OpenGL::EMISSION);
      openGL.glEnable(OpenGL::COLOR_MATERIAL);      
      
      openGL.glScalef(10 * 0.5, 10 * 0.5, 10 * 0.5);
      openGL.glColor4f(1.0, 0.0, 0.0, 1.0);
      openGL.cylinder(0.25, 0.25, 2, 16, 1);
      openGL.glTranslatef(0, 0, 2);
      openGL.cone(0.5, 1.0, 16, 1);
      openGL.glTranslatef(0, 0, -2);
      openGL.glRotatef(90, 1.0, 0.0, 0.0); // x axis
      openGL.glColor4f(0.0, 1.0, 0.0, 1.0);
      openGL.cylinder(0.25, 0.25, 2, 16, 1);
      openGL.glTranslatef(0, 0, 2);
      openGL.cone(0.5, 1.0, 16, 1);
      openGL.glTranslatef(0, 0, -2);
      openGL.glRotatef(90, 0.0, 1.0, 0.0); // y axis
      openGL.glColor4f(0.0, 0.0, 1.0, 1.0);
      openGL.cylinder(0.25, 0.25, 2, 16, 1);
      openGL.glTranslatef(0, 0, 2);
      openGL.cone(0.5, 1.0, 16, 1);

      openGL.glDisable(OpenGL::COLOR_MATERIAL);
    }
    
    void makeFloor() noexcept {
      OpenGL::DisplayList displayList(openGL, OBJECT_FLOOR);
    }
    
    void makeCube() noexcept {
      OpenGL::DisplayList displayList(openGL, OBJECT_CUBE);
      
      openGL.glColorMaterial(OpenGL::FRONT, OpenGL::EMISSION);
      openGL.glEnable(OpenGL::COLOR_MATERIAL);
      openGL.glColor4f(0.0, 1.0, 0.0, 0.75);
      
      {
        OpenGL::Block block(openGL, OpenGL::QUAD_STRIP); // draw the sides of the cube
        
        // Normal A
        openGL.glNormal3f(0.0, 0.0, -1.0);
        
        openGL.glVertex3i(3, 3, -3); // vertex 1
        openGL.glVertex3i(3, -3, -3); // vertex 2
        openGL.glVertex3i(-3, 3, -3); // vertex 3
        openGL.glVertex3i(-3, -3, -3); // vertex 4
        
        // Normal B
        openGL.glNormal3f(-1.0, 0.0, 0.0);
        openGL.glVertex3i(-3, 3, 3); // vertex 5
        openGL.glVertex3i(-3, -3, 3); // vertex 6
        
        // Normal C
        openGL.glNormal3f(0.0, 0.0, 1.0);
        openGL.glVertex3i(3, 3, 3); // vertex 7
        openGL.glVertex3i(3, -3, 3); // vertex 8
        
        // Normal D
        openGL.glNormal3f(1.0, 0.0, 0.0);
        openGL.glVertex3i(3, 3, -3); // vertex 9
        openGL.glVertex3i(3, -3, -3); // vertex 10
      }
      
      openGL.glDisable(OpenGL::COLOR_MATERIAL);
    }
    
    void makeCone() noexcept {
      OpenGL::DisplayList displayList(openGL, OBJECT_CONE);
      openGL.glColorMaterial(OpenGL::FRONT, OpenGL::EMISSION);
      openGL.glEnable(OpenGL::COLOR_MATERIAL);
      openGL.glColor4f(0.25, 0.5, 0.75, 0.5);
      openGL.cone(6.0, 12.0, 8, 8);
      openGL.glDisable(OpenGL::COLOR_MATERIAL);
    }
    
    void makeTorus() noexcept {
      OpenGL::DisplayList displayList(openGL, OBJECT_TORUS);
      openGL.glColorMaterial(OpenGL::FRONT, OpenGL::EMISSION);
      openGL.glEnable(OpenGL::COLOR_MATERIAL);
      openGL.glColor4f(0.25, 0.5, 0.75, 0.5);
      openGL.torus(4.0, 8.0, 64, 16);
      openGL.glDisable(OpenGL::COLOR_MATERIAL);
    }

    void displayBlackness() noexcept {
      openGL.glClearColor(0.0, 0.0, 0.0, 1.0);
      openGL.glClear(OpenGL::COLOR_BUFFER_BIT);
    }
    
    void displayObject(unsigned int object) noexcept {
      openGL.glClearColor(0.0, 0.0, 0.0, 1.0);
      openGL.glClear(OpenGL::COLOR_BUFFER_BIT | OpenGL::DEPTH_BUFFER_BIT);
      
      openGL.glMatrixMode(OpenGL::MODELVIEW);
      openGL.glLoadIdentity();
      Vector3D<long double> translation = view.getTranslation();
      openGL.glTranslatef(translation.getX(), translation.getY(), translation.getZ());
      Vector3D<long double> rotation = view.getRotation();
      openGL.glRotatef(rotation.getX(), 0.0, 1.0, 0.0);
      openGL.glRotatef(rotation.getY(), 1.0, 0.0, 0.0);
      openGL.glRotatef(rotation.getZ(), 0.0, 0.0, 1.0);
      openGL.glScalef(view.getScale() * 0.05, view.getScale() * 0.05, view.getScale() * 0.05);

      openGL.glCallList(object);
    }
    
    void setRenderable(Renderable* renderable) noexcept {
      if (this->renderable) {
        delete this->renderable;
      }
      this->renderable = renderable;
    }
    
    void onDisplay() noexcept {
      if (renderable) {
        renderable->update(view);
      } else {
        switch (mode) {
        case MODE_BLACKNESS:
          displayBlackness();
          break;
        case MODE_SYSTEM:
          displayObject(OBJECT_SYSTEM);
          break;
        case MODE_FLOOR:
          displayObject(OBJECT_FLOOR);
          break;
        case MODE_CUBE:
          displayObject(OBJECT_CUBE);
          break;
        case MODE_CONE:
          displayObject(OBJECT_CONE);
          break;
        case MODE_TORUS:
          displayObject(OBJECT_TORUS);
          break;
        default:
          break;
        }
      }
      openGL.glFlush();
      swap();
    }
    
    void onMove(const Position& position) noexcept {
      if (verbosity >= Verbosity::ALL_MOUSE_EVENTS) {
        fout << MESSAGE("Window move event: ") << position << ENDL;
      }
    }
    
    void onResize(const Dimension& dimension) noexcept {
      if (verbosity >= Verbosity::ALL_MOUSE_EVENTS) {
        fout << MESSAGE("Resize event: ") << dimension << ENDL;
      }
      
      openGL.glViewport(0, 0, dimension.getWidth(), dimension.getHeight());

      // TAG: fixme
      // openGL.glMatrixMode(OpenGL::MODELVIEW);
      openGL.glMatrixMode(OpenGL::PROJECTION);
      openGL.glLoadIdentity();
      double aspectRatio = static_cast<double>(dimension.getWidth())/
        static_cast<double>(dimension.getHeight());
      openGL.perspective(0 * 45.0, aspectRatio, 0.01, 100.0);
      
      // openGL.glMatrixMode(OpenGL::PROJECTION); // TAG: fixme
      // openGL.glLoadIdentity();
      // orthoTop = 1.0;
      // orthoBottom = -1.0;
      // orthoRight = static_cast<double>(dimension.getWidth())/static_cast<double>(dimension.getHeight());
      // orthoLeft = -orthoRight;
      // openGL.glOrtho(orthoLeft, orthoRight, orthoBottom, orthoTop, orthoNear, orthoFar);
      
      invalidate();
    }
    
    struct Flag {
      unsigned int mask;
      Literal literal;
    };
    
    void onMouseMove(const Position& position, unsigned int state) noexcept {
      const Position difference = position - mouseButtonPosition;
      if (verbosity >= Verbosity::ALL_MOUSE_EVENTS) {
        fout << MESSAGE("Mouse motion event: ") << position << ENDL;
      }
      setCursor(OpenGLContext::HAND); // TAG: remove

      if ((mouseMiddleButtonPressed) || (mouseLeftButtonPressed && mouseRightButtonPressed)) {
        if (state & Key::CONTROL) {
          int viewPort[4];
          openGL.glGetIntegerv(OpenGL::VIEWPORT, viewPort);
          Vector3D<long double> position3D = mapXYToWorld(position, viewPort);
          view.translation.setX(translationBegin.getX() + position3D.getX() - drag.getX());
          view.translation.setY(translationBegin.getY() + position3D.getY() - drag.getY());
        } else {
          long double scale = scaleBegin * Math::exp(static_cast<long double>(difference.getY() * 0.01));
          if (scale < 0.00001) {
            scale = 0.0001;
          } else if (scale >= 10.0) {
            scale = 10.0;
          }
          view.setScale(scale);
        }
        invalidate();
      } else if (mouseLeftButtonPressed) {
        // TAG: fixme
        if (state & Key::CONTROL) {
          view.rotation.setZ(
            rotationBegin.getZ() + difference.getX()/(256/*dimension.getWidth()*//180.0)
          );
        } else {
          view.rotation.setX(
            rotationBegin.getX() + difference.getX()/(256/*dimension.getHeight()*//180.0)
          );
          view.rotation.setY(
            rotationBegin.getY() + difference.getY()/(256/*dimension.getWidth()*//180.0)
          );
          
          /*long double ax = difference.getY();
            long double ay = difference.getX();
            long double az = 0;
            long double angle = 180.0 * Math::sqrt(ax*ax + ay*ay + az*az)/static_cast<long double>(dimension.getWidth());
          */
          // use inverse matrix to determine local axis of rotation
          //openGL.glGetDoublev(OpenGL::MODELVIEW_MATRIX, modelViewMatrix);
          //invertMatrix(modelViewMatrix, invertedModelViewMatrix);
            
          //long double axisX = invertedModelViewMatrix[0] * ax + invertedModelViewMatrix[4] * ay + invertedModelViewMatrix[7] * az;
          //long double axisY = invertedModelViewMatrix[1] * ax + invertedModelViewMatrix[5] * ay + invertedModelViewMatrix[8] * az;
          //long double axisZ = invertedModelViewMatrix[2] * ax + invertedModelViewMatrix[6] * ay + invertedModelViewMatrix[9] * az;
          //openGL.glRotatef(angle, axisX, axisY, axisZ);
        }
        invalidate();
      }
      //         if (buttons != 0) {
      //           fout << MESSAGE("Mouse move: ") << position << ' ' << MESSAGE("ACTIVE") << ENDL;
      //         } else {
      //           fout << MESSAGE("Mouse move: ") << position << ' ' << MESSAGE("PASSIVE") << ENDL;
      //         }
    }

    void onMouseScope(bool scope) noexcept {
      fout << MESSAGE("Event: mouse scope ")
           << (scope ? MESSAGE("INSIDE SCOPE") : MESSAGE("OUT OF SCOPE")) << ENDL;
    }
    
    void onMouseButton(const Position& position, Mouse::Button button, Mouse::Event event, unsigned int state) noexcept {
      static const Flag STATES[] = {
        {Mouse::LEFT, MESSAGE("LEFT")},
        {Mouse::MIDDLE, MESSAGE("MIDDLE")},
        {Mouse::RIGHT, MESSAGE("RIGHT")},
        {Mouse::WHEEL, MESSAGE("WHEEL")},
        {Mouse::EXTRA, MESSAGE("EXTRA")},
        {Mouse::EXTRA2, MESSAGE("EXTRA2")},
        {Key::LEFT_CONTROL, MESSAGE("L-CTRL")},
        {Key::RIGHT_CONTROL, MESSAGE("R-CTRL")},
        {Key::LEFT_SHIFT, MESSAGE("L-SHFT")},
        {Key::RIGHT_SHIFT, MESSAGE("R-SHFT")},
        {Key::LEFT_ALT, MESSAGE("L-ALT")},
        {Key::RIGHT_ALT, MESSAGE("R-ALT")},
        {Key::NUM_LOCK_TOGGLED, MESSAGE("NUM")},
        {Key::CAPS_LOCK_TOGGLED, MESSAGE("CAPS")},
        {Key::SCROLL_LOCK_TOGGLED, MESSAGE("SCROLL")},
        {Key::INSERT_TOGGLED, MESSAGE("INSERT")}
      };
      
      static const Literal EVENT_STRING[] = {
        MESSAGE("PRESSED"),
        MESSAGE("RELEASED"),
        MESSAGE("DOUBLE CLICKED"),
        MESSAGE("TURNED")
      };
      
      if (verbosity >= Verbosity::ACTIVE_MOUSE_EVENTS) {
        fout << MESSAGE("Mouse button event: ")
             << getMouseButtonName(button) << ' ';
        
        if (static_cast<unsigned int>(event) < getArraySize(EVENT_STRING)) {
          fout << EVENT_STRING[event];
        } else {
          fout << MESSAGE("[UNNAMED EVENT]") << ' ' << static_cast<unsigned int>(event);
        }
        fout << ' ';
        
        for (unsigned int i = 0; i < getArraySize(STATES); ++i) {
          if (state & STATES[i].mask) {
            fout << STATES[i].literal << ' ';
          }
        }
        
        fout << position << ENDL;
      }

      switch (button) {
      case Mouse::LEFT:
        mouseLeftButtonPressed = event == Mouse::PRESSED;
        setCapture(mouseLeftButtonPressed);
        break;
      case Mouse::MIDDLE:
        mouseMiddleButtonPressed = event == Mouse::PRESSED;
        setCapture(mouseMiddleButtonPressed);
        break;
      case Mouse::RIGHT:        
        mouseRightButtonPressed = event == Mouse::PRESSED;
        setCapture(false);
        if (event == Mouse::PRESSED) {
          displayMenu(position, menu);
        }
        break;
      default:
        break;
      }
      mouseButtonPosition = position;
      
      if (event == Mouse::PRESSED) {
        // command: TRANSLATE_IN_XY_PLANE
        // command: ROTATE_AROUND_X_AND_Y
        // command: SCALE
        // command: MEASURE_DISTANCE
        translationBegin = view.getTranslation();
        rotationBegin = view.getRotation();
        scaleBegin = view.getScale();
      }
      
      int viewPort[4];
      openGL.glGetIntegerv(OpenGL::VIEWPORT, viewPort);
      Vector3D<long double> drag = mapXYToWorld(position, viewPort);    
      drag.setZ(0);
    }
    
    void onMouseWheel(const Position& position, int delta, unsigned int buttons) noexcept {
      if (verbosity >= Verbosity::ACTIVE_MOUSE_EVENTS) {
        fout << MESSAGE("Mouse wheel") << ENDL;
      }
      setTranslation(view.getTranslation() + Vector3D<long double>(0, 0, 0.1 * delta/120));
      invalidate();
    }
    
    void onKey(unsigned int key, unsigned int flags, unsigned int modifiers) noexcept {
      if (flags & Key::PRESSED) {
        if (flags & Key::DEAD) {
          return;
        }

        if ((flags & Key::ASCII) && ASCIITraits::isGraph(key)) {
          fout << MESSAGE("Key: ") << PREFIX << HEX << key << ' '
               << '\'' << static_cast<char>(key) << '\''
               << ' ' << MESSAGE("PRESSED");
        } else {
          fout << MESSAGE("Key: ") << PREFIX << HEX << key << ' ' << MESSAGE("PRESSED");
        }
        if (flags & Key::FIRST_TIME) {
          fout << ' ' << MESSAGE("FIRST");
        }
        fout << ENDL;
      } else {
        fout << MESSAGE("Key: ") << PREFIX << HEX << key << ' ' << MESSAGE("RELEASED") << ENDL;
      }

      if (flags & Key::PRESSED) {
        if (flags & Key::DEAD) {
          return;
        }
          
        unsigned int command = Command::NOTHING;
        switch (key) {
        case Key::LEFT:
          command = Command::ROTATE_AROUND_X_AXIS_NEG;
          break;
        case Key::RIGHT:
          command = Command::ROTATE_AROUND_X_AXIS_POS;
          break;
        case Key::UP:
          command = Command::ROTATE_AROUND_Y_AXIS_NEG;
          break;
        case Key::DOWN:
          command = Command::ROTATE_AROUND_Y_AXIS_POS;
          break;
        case Key::PRIOR:
          if (modifiers & Key::SHIFT) {
            command = Command::TRANSLATE_ALONG_X_NEG;
          } else if (modifiers & Key::CONTROL) {
            command = Command::TRANSLATE_ALONG_Y_NEG;
          } else if (modifiers & Key::ALT) {
          } else {
            command = Command::TRANSLATE_ALONG_Z_NEG;
          }
          break;
        case Key::NEXT:
          if (modifiers & Key::SHIFT) {
            command = Command::TRANSLATE_ALONG_X_POS;
          } else if (modifiers & Key::CONTROL) {
            command = Command::TRANSLATE_ALONG_Y_POS;
          } else if (modifiers & Key::ALT) {
          } else {
            command = Command::TRANSLATE_ALONG_Z_POS;
          }
          break;
        case Key::HOME:
          command = Command::RESET_VIEW_PARAMETERS;
          break;
        case Key::END:
          break;
        case Key::F1:
          command = Command::SHOW_OPENGL_INFORMATION;
          break;
        case Key::F12:
          command = Command::CYCLE_DISPLAY_MODE;
          break;
        case ' ':
          command = Command::RENDER_DEFAULT;
          break;
        case 'q':
          command = Command::QUIT;
          break;
        case 'a':
          setAutorepeat(!getAutorepeat());
          break;
        case 's':
          command = Command::CYCLE_SHADING_MODEL;
          break;
        case 'b':
          command = blending ? Command::BLENDING_DISABLE : Command::BLENDING_ENABLE;
          break;
        case 'm':
          command = Command::SELECT_SHADING_MODEL_FLAT;
          break;
        case 'L':
          command = lighting ? Command::LIGHTING_DISABLE : Command::LIGHTING_ENABLE;
          break;
        case 'f':
          command = Command::SELECT_POLYGON_MODE_FILL;
          break;
        case 'l':
          command = Command::SELECT_POLYGON_MODE_LINE;
          break;
        case 'p':
          command = Command::SELECT_POLYGON_MODE_POINT;
          break;
        case 'r':
          command = Command::RESET_VIEW_PARAMETERS;
          break;
        }
        onCommand(command);
      }
    }
    
    void onIdle() noexcept {
      invalidate();
    }
    
    bool onClose() noexcept {
      fout << MESSAGE("Event: close ") << ENDL;
      MessageDialog dialog(
        MESSAGE("Quit"),
        MESSAGE("Do you really wan't to quit?"),
        MessageDialog::QUESTION
      );
      dialog.execute();
      if (dialog.getAnswer() == MessageDialog::YES) {
        Application::getApplication()->terminate();
      }
      return dialog.getAnswer() == MessageDialog::YES;
    }
    
    void onVisibility(Visibility visibility) noexcept {
      if (verbosity >= Verbosity::ACTIVE_MOUSE_EVENTS) {
        fout << MESSAGE("Visibility event: ")
             << ((visibility == VISIBLE) ? MESSAGE("VISIBLE") : MESSAGE("INVISIBLE"))
             << ENDL;
      }
    }
    
    void onFocus(Focus focus) noexcept {
      if (verbosity >= Verbosity::ACTIVE_MOUSE_EVENTS) {
        fout << MESSAGE("Focus event: ")
             << ((focus == ACQUIRED_FOCUS) ? MESSAGE("ACQUIRED FOCUS") : MESSAGE("LOST FOCUS"))
             << ENDL;
      }
    }

    void dumpOpenGLInformation() noexcept {
      fout << MESSAGE("OpenGL context information: ") << EOL
           << indent(2) << MESSAGE("client vendor: ") << getGLClientVendor() << EOL
           << indent(2) << MESSAGE("client release: ") << getGLClientRelease() << EOL
           << indent(2) << MESSAGE("client extensions: ") << getGLClientExtensions() << EOL
           << indent(2) << MESSAGE("server vendor: ") << getGLServerVendor() << EOL
           << indent(2) << MESSAGE("server release: ") << getGLServerRelease() << EOL
           << indent(2) << MESSAGE("server extensions: ") << getGLServerExtensions() << EOL
           << indent(2) << MESSAGE("direct context: ") << isDirect() << EOL
           << ENDL;
      fout << MESSAGE("Vendor: ") << openGL.getVendor() << EOL
           << MESSAGE("Renderer: ") << openGL.getRenderer() << EOL
           << MESSAGE("Version: ") << openGL.getVersion() << EOL
           << MESSAGE("Extensions: ") << openGL.getExtensions() << EOL
           << EOL
           << MESSAGE("Rendering context:") << EOL
           << indent(2) << MESSAGE("red bits: ") << redBits << EOL
           << indent(2) << MESSAGE("green bits: ") << greenBits << EOL
           << indent(2) << MESSAGE("blue bits: ") << blueBits << EOL
           << indent(2) << MESSAGE("alpha bits: ") << alphaBits << EOL
           << indent(2) << MESSAGE("accumulator red bits: ") << accumulatorRedBits << EOL
           << indent(2) << MESSAGE("accumulator green bits: ") << accumulatorGreenBits << EOL
           << indent(2) << MESSAGE("accumulator blue bits: ") << accumulatorBlueBits << EOL
           << indent(2) << MESSAGE("accumulator alpha bits: ") << accumulatorAlphaBits << EOL
           << indent(2) << MESSAGE("depth bits: ") << depthBits << EOL
           << indent(2) << MESSAGE("stencil bits: ") << stencilBits << EOL
           << indent(2) << MESSAGE("aux buffers: ") << auxBuffers << EOL
           << indent(2) << MESSAGE("overlay planes: ") << numberOfOverlayPlanes << EOL
           << indent(2) << MESSAGE("underlay planes: ") << numberOfUnderlayPlanes << EOL
           << indent(2) << MESSAGE("double buffered: ") << isDoubleBuffered() << EOL
           << indent(2) << MESSAGE("stereoscopic: ") << isStereoscopic() << EOL
           << indent(2) << MESSAGE("direct: ") << isDirect() << EOL
           << indent(2) << MESSAGE("generic: ") << isGeneric() << EOL
           << ENDL;
    }
    
    void openImage() noexcept {
      try {
        if (!openFile.execute()) {
          return; // canceled
        }
      } catch (UserInterfaceException&) {
        MessageDialog dialog(
          MESSAGE("Error"),
          MESSAGE("Unable to open dialog."),
          MessageDialog::ERROR
        );
        dialog.execute();
      }
      ImageEncoder* encoder = encoderRegistry.getEncoder(openFile.getFilename());
      if (!encoder) {
        MessageDialog dialog(
          MESSAGE("Error"),
          MESSAGE("Unable to resolve image encoder."),
          MessageDialog::ERROR
        );
        dialog.execute();
        return;
      }
      if (verbosity >= Verbosity::EVERYTHING) {
        fout << MESSAGE("Encoder: ") << encoder->getDescription() << ENDL;
      }
      ColorImage* frame = nullptr;
      try {
        frame = encoder->read(openFile.getFilename());
      } catch (IOException) {
        MessageDialog dialog(
          MESSAGE("Error"),
          MESSAGE("Unable to open image."),
          MessageDialog::ERROR
        );
        dialog.execute();
        return;
      }
      image = *frame;
      delete frame;
    }
      
    void saveFrameBuffer() noexcept {
      try {
        if (!saveFile.execute()) {
          return; // canceled
        }
      } catch (UserInterfaceException&) {
        MessageDialog dialog(
          MESSAGE("Error"),
          MESSAGE("Unable to open dialog."),
          MessageDialog::ERROR
        );
        dialog.execute();
      }
      ImageEncoder* encoder = encoderRegistry.getEncoder(saveFile.getFilename());
      if (!encoder) {
        MessageDialog dialog(
          MESSAGE("Error"),
          MESSAGE("Unable to resolve image encoder."),
          MessageDialog::ERROR
        );
        dialog.execute();
        return;
      }
      if (verbosity >= Verbosity::EVERYTHING) {
        fout << MESSAGE("Encoder: ") << encoder->getDescription() << ENDL;
      }
      OpenGL::GLint viewPort[4];
      openGL.glGetIntegerv(OpenGL::VIEWPORT, viewPort);
      // ColorImage frame(Dimension(viewPort[2], viewPort[3])); // TAG: fixme
      ColorImage* frame = new ColorImage(Dimension(viewPort[2], viewPort[3]));
      openGL.glPixelStorei(OpenGL::PACK_ALIGNMENT, 4);
      // TAG: need atomic access to frame
      openGL.glReadPixels(
        0,
        0,
        viewPort[2],
        viewPort[3],
        OpenGL::RGBA,
        OpenGL::UNSIGNED_BYTE,
        frame->getElements()
      );
      try {
        encoder->write(saveFile.getFilename(), frame);
      } catch (IOException) {
        MessageDialog dialog(MESSAGE("Error"), MESSAGE("Unable to save image."), MessageDialog::ERROR);
        dialog.execute();
      }
      delete frame;
    }

    void setDisplayMode(DisplayMode displayMode) noexcept {
      if (displayMode != this->displayMode) {
        try {
          // TAG: fixme
          this->displayMode = displayMode;
        } catch (UserInterfaceException&) {
          MessageDialog dialog(
            MESSAGE("Error"),
            MESSAGE("Unable to switch display mode."),
            MessageDialog::ERROR
          );
        }
      }
    }
    
    void dumpCommand(const Literal& description) noexcept {
      if (verbosity >= Verbosity::COMMANDS) {
        fout << MESSAGE("Command: ") << description << ENDL;
      }
    }
     
    void onCommand(unsigned int identifier) noexcept {
      switch (identifier) {
      case Command::SELECT_VERBOSITY_NO_INFORMATION:
        dumpCommand(MESSAGE("Set verbosity level to NO_INFORMATION"));
        setVerbosity(Verbosity::NO_INFORMATION);
        break;
      case Command::SELECT_VERBOSITY_WARNINGS:
        dumpCommand(MESSAGE("Set verbosity level to WARNINGS"));
        setVerbosity(Verbosity::WARNINGS);
        break;
      case Command::SELECT_VERBOSITY_NORMAL:
        dumpCommand(MESSAGE("Set verbosity level to NORMAL"));
        setVerbosity(Verbosity::NORMAL);
        break;
      case Command::SELECT_VERBOSITY_COMMANDS:
        dumpCommand(MESSAGE("Set verbosity level to COMMANDS"));
        setVerbosity(Verbosity::COMMANDS);
        break;
      case Command::SELECT_VERBOSITY_ACTIVE_MOUSE_EVENTS:
        dumpCommand(MESSAGE("Set verbosity level to ACTIVE_MOUSE_EVENTS"));
        setVerbosity(Verbosity::ACTIVE_MOUSE_EVENTS);
        break;
      case Command::SELECT_VERBOSITY_ALL_MOUSE_EVENTS:
        dumpCommand(MESSAGE("Set verbosity level to ALL_MOUSE_EVENTS"));
        setVerbosity(Verbosity::ALL_MOUSE_EVENTS);
        break;
      case Command::SELECT_VERBOSITY_EVERYTHING:
        dumpCommand(MESSAGE("Set verbosity level to EVERYTHING"));
        setVerbosity(Verbosity::EVERYTHING);
        break;
      case Command::OPEN_IMAGE:
        dumpCommand(MESSAGE("Open image"));
        openImage();
        if ((mode != MODE_INTENSITY) && (mode != MODE_RGB_CLOUD)) {
          setMode(MODE_INTENSITY); // TAG: use last image mode
        } else {
          setMode(mode);
        }
        break;
      case Command::SAVE_IMAGE:
        dumpCommand(MESSAGE("Save image"));
        saveFrameBuffer();
        break;
      case Command::HELP:
        dumpCommand(MESSAGE("Help"));
        fout << MESSAGE("Help") << EOL << ENDL; // TAG: fixme
        break;
      case Command::SHOW_OPENGL_INFORMATION:
        dumpCommand(MESSAGE("Show OpenGL information"));
        dumpOpenGLInformation();
        break;
      case Command::ABOUT:
        dumpCommand(MESSAGE("About"));
        fout << base::Version().getBanner() << EOL << ENDL;
        fout << gip::Version().getBanner() << EOL << ENDL;
        break;
      case Command::CYCLE_DISPLAY_MODE:
        {
          dumpCommand(MESSAGE("Cycle display mode"));
          unsigned int displayMode = static_cast<unsigned int>(this->displayMode) + 1;
          if (displayMode > static_cast<unsigned int>(DISPLAY_MODE_LAST)) {
            displayMode = static_cast<unsigned int>(DISPLAY_MODE_FIRST);
          }
          setDisplayMode(static_cast<DisplayMode>(displayMode));
        }
        break;
      case Command::QUIT:
        dumpCommand(MESSAGE("Quit"));
        if (onClose()) {
          exit();
        }
        break;
      case Command::CYCLE_SHADING_MODEL:
        {
          dumpCommand(MESSAGE("Cycle shading model"));
          unsigned int shadingModel = static_cast<unsigned int>(this->shadingModel) + 1;
          if (shadingModel > static_cast<unsigned int>(ShadingModel::LAST_MODEL)) {
            shadingModel = static_cast<unsigned int>(ShadingModel::FIRST_MODEL);
          }
          setShadingModel(static_cast<ShadingModel::Model>(shadingModel));
        }
        break;
      case Command::SELECT_SHADING_MODEL_FLAT:
        dumpCommand(MESSAGE("Select shading model: flat"));
        setShadingModel(ShadingModel::FLAT);
        break;
      case Command::SELECT_SHADING_MODEL_SMOOTH:
        dumpCommand(MESSAGE("Select shading model: smooth"));
        setShadingModel(ShadingModel::SMOOTH);
        break;
      case Command::CYCLE_POLYGON_MODE:
        {
          dumpCommand(MESSAGE("Cycle polygon mode"));
          unsigned int polygonMode = static_cast<unsigned int>(this->polygonMode) + 1;
          if (polygonMode > static_cast<unsigned int>(PolygonMode::LAST_MODE)) {
            polygonMode = static_cast<unsigned int>(PolygonMode::FIRST_MODE);
          }
          setPolygonMode(static_cast<PolygonMode::Mode>(polygonMode));
        }
        break;
      case Command::SELECT_POLYGON_MODE_FILL:
        dumpCommand(MESSAGE("Select polygon mode: fill"));
        setPolygonMode(PolygonMode::FILL);
        break;
      case Command::SELECT_POLYGON_MODE_LINE:
        dumpCommand(MESSAGE("Select polygon mode: line"));
        setPolygonMode(PolygonMode::LINE);
        break;
      case Command::SELECT_POLYGON_MODE_POINT:
        dumpCommand(MESSAGE("Select polygon mode: point"));
        setPolygonMode(PolygonMode::POINT);
        break;
      case Command::BLENDING_ENABLE:
        dumpCommand(MESSAGE("Enable blending"));
        setBlending(true);
        break;
      case Command::BLENDING_DISABLE:
        dumpCommand(MESSAGE("Disable blending"));
        setBlending(false);
        break;
      case Command::LIGHTING_ENABLE:
        dumpCommand(MESSAGE("Enable lighting"));
        setLighting(true);
        break;
      case Command::LIGHTING_DISABLE:
        dumpCommand(MESSAGE("Disable lighting"));
        setLighting(false);
        break;
      case Command::RESET_VIEW_PARAMETERS:
        dumpCommand(MESSAGE("Reset view parameters"));
        resetViewParameters();
        break;
      case Command::ROTATE_AROUND_X_AXIS_NEG:
        dumpCommand(MESSAGE("Rotate around X axis (neg)"));
        setRotation(view.getRotation() + Vector3D<long double>(-1.0, 0, 0));
        break;
      case Command::ROTATE_AROUND_X_AXIS_POS:
        dumpCommand(MESSAGE("Rotate around X axis (pos)"));
        setRotation(view.getRotation() + Vector3D<long double>(1.0, 0, 0));
        break;
      case Command::ROTATE_AROUND_Y_AXIS_NEG:
        dumpCommand(MESSAGE("Rotate around Y axis (neg)"));
        setRotation(view.getRotation() + Vector3D<long double>(0, -1.0, 0));
        break;
      case Command::ROTATE_AROUND_Y_AXIS_POS:
        dumpCommand(MESSAGE("Rotate around Y axis (pos)"));
        setRotation(view.getRotation() + Vector3D<long double>(0, 1.0, 0));
        break;
      case Command::ROTATE_AROUND_Z_AXIS_NEG:
        dumpCommand(MESSAGE("Rotate around Z axis (neg)"));
        setRotation(view.getRotation() + Vector3D<long double>(0, 0, -1.0));
        break;
      case Command::ROTATE_AROUND_Z_AXIS_POS:
        dumpCommand(MESSAGE("Rotate around Z axis (pos)"));
        setRotation(view.getRotation() + Vector3D<long double>(0, 0, 1.0));
        break;
      case Command::TRANSLATE_ALONG_X_NEG:
        dumpCommand(MESSAGE("Translate along X axis (neg)"));
        setTranslation(view.getTranslation() + Vector3D<long double>(-0.1, 0, 0));
        break;
      case Command::TRANSLATE_ALONG_X_POS:
        dumpCommand(MESSAGE("Translate along X axis (pos)"));
        setTranslation(view.getTranslation() + Vector3D<long double>(0.1, 0, 0));
        break;
      case Command::TRANSLATE_ALONG_Y_NEG:
        dumpCommand(MESSAGE("Translate along Y axis (neg)"));
        setTranslation(view.getTranslation() + Vector3D<long double>(0, -0.1, 0));
        break;
      case Command::TRANSLATE_ALONG_Y_POS:
        dumpCommand(MESSAGE("Translate along Y axis (pos)"));
        setTranslation(view.getTranslation() + Vector3D<long double>(0, 0.1, 0));
        break;
      case Command::TRANSLATE_ALONG_Z_NEG:
        dumpCommand(MESSAGE("Translate along Z axis (neg)"));
        setTranslation(view.getTranslation() + Vector3D<long double>(0, 0, -0.1));
        break;
      case Command::TRANSLATE_ALONG_Z_POS:
        dumpCommand(MESSAGE("Translate along Z axis (pos)"));
        setTranslation(view.getTranslation() + Vector3D<long double>(0, 0, 0.1));
        break;
      case Command::SELECT_MODE_DEFAULT:
        dumpCommand(MESSAGE("Select view mode: default"));
        setMode(MODE_DEFAULT);
        break;
      case Command::SELECT_MODE_SYSTEM:
        dumpCommand(MESSAGE("Select view mode: system"));
        setMode(MODE_SYSTEM);
        break;
      case Command::SELECT_MODE_FLOOR:
        dumpCommand(MESSAGE("Select view mode: floor"));
        setMode(MODE_FLOOR);
        break;
      case Command::SELECT_MODE_CUBE:
        dumpCommand(MESSAGE("Select view mode: cube"));
        setMode(MODE_CUBE);
        break;
      case Command::SELECT_MODE_CONE:
        dumpCommand(MESSAGE("Select view mode: cone"));
        setMode(MODE_SYSTEM);
        break;
      case Command::SELECT_MODE_TORUS:
        dumpCommand(MESSAGE("Select view mode: torus"));
        setMode(MODE_TORUS);
        break;
      case Command::SELECT_MODE_INTENSITY:
        dumpCommand(MESSAGE("Select view mode: image"));
        setMode(MODE_INTENSITY);
        break;
      case Command::SELECT_MODE_RGB_CLOUD:
        dumpCommand(MESSAGE("Select view mode: RGB cloud"));
        setMode(MODE_RGB_CLOUD);
        break;
      case Command::SELECT_QUALITY_WORST:
        dumpCommand(MESSAGE("Select quality: worst"));          
        setQuality(QUALITY_WORST);
        break;
      case Command::SELECT_QUALITY_NORMAL:
        dumpCommand(MESSAGE("Select quality: normal"));
        setQuality(QUALITY_NORMAL);
        break;
      case Command::SELECT_QUALITY_BEST:
        dumpCommand(MESSAGE("Select quality: best"));
        setQuality(QUALITY_BEST);
        break;
      default:
        if (verbosity >= Verbosity::WARNINGS) {
          fout << MESSAGE("Command '") << identifier << MESSAGE("' is not supported") << ENDL;
        }
      }
    }
  };
  
  void main() noexcept {
    fout << getFormalName() << MESSAGE(" version ") << MAJOR_VERSION << '.' << MINOR_VERSION << EOL
         << MESSAGE("Generic Image Processing Framework (Test Suite)") << EOL
         << MESSAGE("https://dev.azure.com/renefonseca/gip") << EOL
         << MESSAGE("Copyright (C) 2002-2019 by Rene Moeller Fonseca") << EOL
         << ENDL;
    
    MyOpenGLContext::Format desiredFormat;
    int formatId = -1;
    
    Array<MyOpenGLContext::Format> formats = MyOpenGLContext::getFormats(
      OpenGLContext::RGB |
      OpenGLContext::DOUBLE_BUFFERED |
      OpenGLContext::DEPTH |
      OpenGLContext::DIRECT
    );
    Array<MyOpenGLContext::Format>::ReadEnumerator enu = formats.getReadEnumerator();
    
    fout << MESSAGE("Available formats:") << ENDL;
    for (unsigned int i = 0; enu.hasNext(); ++i) {
      const MyOpenGLContext::Format* format = enu.next();
      if (false) {
        fout << indent(2) << MESSAGE("Format: ") << i << EOL
             << indent(4) << MESSAGE("color indexed: ")
             << ((format->flags & MyOpenGLContext::COLOR_INDEXED) != 0) << EOL
             << indent(4) << MESSAGE("rgb: ")
             << ((format->flags & MyOpenGLContext::RGB) != 0) << EOL
             << indent(4) << MESSAGE("double buffered: ")
             << ((format->flags & MyOpenGLContext::DOUBLE_BUFFERED) != 0) << EOL
             << indent(4) << MESSAGE("stereoscopic: ")
             << ((format->flags & MyOpenGLContext::STEREO) != 0) << EOL
             << indent(4) << MESSAGE("generic: ")
             << ((format->flags & MyOpenGLContext::GENERIC) != 0) << EOL
             << indent(4) << MESSAGE("color bits: ") << format->colorBits << EOL
             << indent(6) << MESSAGE("red bits: ") << format->redBits << EOL
             << indent(6) << MESSAGE("green bits: ") << format->greenBits << EOL
             << indent(6) << MESSAGE("blue bits: ") << format->blueBits << EOL
             << indent(6) << MESSAGE("alpha bits: ") << format->alphaBits << EOL
             << indent(4) << MESSAGE("accumulator bits: ") << format->accumulatorBits << EOL
             << indent(6) << MESSAGE("accumulator red bits: ") << format->accumulatorRedBits << EOL
             << indent(6) << MESSAGE("accumulator green bits: ") << format->accumulatorGreenBits << EOL
             << indent(6) << MESSAGE("accumulator blue bits: ") << format->accumulatorBlueBits << EOL
             << indent(6) << MESSAGE("accumulator alpha bits: ") << format->accumulatorAlphaBits << EOL
             << indent(4) << MESSAGE("depthBits: ") << format->depthBits << EOL
             << indent(4) << MESSAGE("stencilBits: ") << format->stencilBits << EOL
             << indent(4) << MESSAGE("auxBuffers: ") << format->auxBuffers << EOL
             << ENDL;
      }
      if ((formatId == -1) || (format->alphaBits && (!desiredFormat.alphaBits))) {
        formatId = i;
        desiredFormat = *format;
      }
    }
    
    if (formatId == -1) {
      ferr << MESSAGE("Format not available") << ENDL;
      setExitCode(EXIT_CODE_ERROR);
      return;
    }
    
    MyOpenGLContext myOpenGLContext(
      Application::getApplication()->getFormalName(),
      Position(64, 64),
      Dimension(256, 256),
      desiredFormat
    );
    
    myOpenGLContext.show();
    myOpenGLContext.raise();
    myOpenGLContext.dispatch();
  }
};

APPLICATION_STUB(ViewerApplication);
