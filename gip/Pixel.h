/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__PIXEL_H
#define _DK_SDU_MIP__GIP__PIXEL_H

#include <gip/features.h>
#include <base/Functor.h>
#include <base/mathematics/Complex.h>

namespace gip {

  /**
    The default intensity used by image elements (8 bit).

    @short Image element.
  */
  typedef unsigned char Intensity; // TAG: close to stupid

  static const Intensity MINIMUM_INTENSITY = 0x00;
  static const Intensity MAXIMUM_INTENSITY = 0xff;

  /**
    Image element wrapper for a single intensity (e.g. gray level) with the
    intensity represented by one byte.

    @short Image element.
  */
  typedef Intensity __attribute__ ((aligned (4) )) GrayPixel; // TAG: should this be unaligned

  /**
    Image element wrapper for red, green, and blue intensities with each
    intensity represented by one byte. Pixel is stored as a 32 bit integer.

    @short Image element.
  */
  union RGBPixel {
    unsigned int rgb;
    struct {
      Intensity red;
      Intensity green;
      Intensity blue;
    } __attribute__ ((packed));
  } __attribute__ ((packed));

  inline RGBPixel makeRGBPixel(Intensity blue, Intensity green, Intensity red) throw() {
    RGBPixel result;
    result.red = red;
    result.green = green;
    result.blue = blue;
    return result;
  }

  static const RGBPixel RGB_PIXEL_BLACK = {0x000000};
  static const RGBPixel RGB_PIXEL_WHITE = {0xffffff};

  /**
    Image element wrapper for red, green, blue, and alpha intensities with each
    intensity represented by one byte. Pixel is storage as a 32 bit integer. The
    alpha element is ordinarily used to specify the transparency level of the
    pixel.

    @short RGB pixel with transparency element.
  */
  union RGBAPixel {
    unsigned int rgba;
    struct {
      Intensity red;
      Intensity green;
      Intensity blue;
      Intensity alpha;
    } __attribute__ ((packed));
  } __attribute__ ((packed));

  static const Intensity ALPHA_TRANSPARENT_INTENSITY = MINIMUM_INTENSITY;
  static const Intensity ALPHA_OPAQUE_INTENSITY = MAXIMUM_INTENSITY;

  inline RGBAPixel makeRGBAPixel(Intensity red, Intensity green, Intensity blue, Intensity alpha) throw() {
    RGBAPixel result;
    result.red = red;
    result.green = green;
    result.blue = blue;
    result.alpha = alpha;
    return result;
  }

  typedef float FloatPixel; // TAG: stupid typedef
  
  typedef double DoublePixel; // TAG: stupid typedef

  /**
    Pixel with red, green, and blue components of any type.
  */
  template<class TYPE>
  struct AnyRGBPixel {
    TYPE red;
    TYPE green;
    TYPE blue;
  };
  
  /**
    Pixel with red, green, blue, and alpha components of any type.
  */
  template<class TYPE>
  struct AnyRGBAPixel {
    TYPE red;
    TYPE green;
    TYPE blue;
    TYPE alpha;
  };

//   template<>
//   struct AnyRGBPixel<unsigned char> {
//     union {
//       unsigned int rgb;
//       struct {
//         unsigned char red;
//         unsigned char green;
//         unsigned char blue;
//         unsigned char unused;
//       } __attribute__ ((packed));
//     } __attribute__ ((packed));
//   };

//   typedef AnyRGBPixel<unsigned char> RGBPixel;
  
  /** (R, G, B) triple pixel with each element stored as an unsigned integer. */
  typedef AnyRGBPixel<unsigned int> RGBPixel96;

  /** Single precision RGB pixel. */
  typedef AnyRGBPixel<float> FloatRGBPixel;

  /** Double precision RGB pixel. */
  typedef AnyRGBPixel<double> DoubleRGBPixel;

  /** Long double precision RGB pixel. */
  typedef AnyRGBPixel<long double> LongDoubleRGBPixel;

  /** (R, G, B, A) quadruple pixel with each element stored as an unsigned integer. */
  typedef AnyRGBAPixel<unsigned int> RGBAPixel128;

  typedef Complex ComplexPixel; // TAG: stupid typedef

  template<class DEST, class SRC>
  inline DEST convertPixel(const SRC& value) throw() {return value;}

  template<>
  inline ComplexPixel convertPixel<ComplexPixel, FloatPixel>(const FloatPixel& value) throw() {
    return ComplexPixel(value);
  }

  template<>
  inline ComplexPixel convertPixel<ComplexPixel, DoublePixel>(const DoublePixel& value) throw() {
    return ComplexPixel(value);
  }

  template<>
  inline FloatPixel convertPixel<FloatPixel, GrayPixel>(const GrayPixel& value) throw() {
    return FloatPixel(value);
  }

  template<>
  inline GrayPixel convertPixel<GrayPixel, RGBPixel>(const RGBPixel& value) throw() {
    return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3}; // looses information
  }

  template<>
  inline GrayPixel convertPixel<GrayPixel, RGBAPixel>(const RGBAPixel& value) throw() {
    return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3}; // looses information
  }

  template<>
  inline FloatPixel convertPixel<FloatPixel, ComplexPixel>(const ComplexPixel& value) throw() {
    return FloatPixel(value.getReal()); // looses information
  }

  template<>
  inline FloatRGBPixel convertPixel<FloatRGBPixel, RGBPixel>(const RGBPixel& value) throw() {
    FloatRGBPixel result;
    result.red = value.red;
    result.green = value.green;
    result.blue = value.blue;
    return result;
  }

  template<>
  inline FloatRGBPixel convertPixel<FloatRGBPixel, RGBAPixel>(const RGBAPixel& value) throw() {
    FloatRGBPixel result;
    result.red = value.red;
    result.green = value.green;
    result.blue = value.blue;
    return result;
  }

  template<>
  inline DoubleRGBPixel convertPixel<DoubleRGBPixel, RGBPixel>(const RGBPixel& value) throw() {
    DoubleRGBPixel result;
    result.red = value.red;
    result.green = value.green;
    result.blue = value.blue;
    return result;
  }

  template<>
  inline DoubleRGBPixel convertPixel<DoubleRGBPixel, RGBAPixel>(const RGBAPixel& value) throw() {
    DoubleRGBPixel result;
    result.red = value.red;
    result.green = value.green;
    result.blue = value.blue;
    return result;
  }

  template<>
  inline DoubleRGBPixel convertPixel<DoubleRGBPixel, FloatRGBPixel>(const FloatRGBPixel& value) throw() {
    DoubleRGBPixel result;
    result.red = value.red;
    result.green = value.green;
    result.blue = value.blue;
    return result;
  }

  template<>
  inline GrayPixel convertPixel<GrayPixel, FloatPixel>(const FloatPixel& value) throw() {
    return GrayPixel(static_cast<Intensity>(value)); // looses information
  }

  template<>
  inline RGBPixel convertPixel<RGBPixel, GrayPixel>(const GrayPixel& value) throw() {
    RGBPixel result;
    result.red = value;
    result.green = value;
    result.blue = value;
    return result;
  }

  template<>
  inline RGBAPixel convertPixel<RGBAPixel, GrayPixel>(const GrayPixel& value) throw() {
    RGBAPixel result;
    result.red = value;
    result.green = value;
    result.blue = value;
    result.alpha = ALPHA_OPAQUE_INTENSITY;
    return result;
  }

  template<>
  inline RGBAPixel convertPixel<RGBAPixel, RGBPixel>(const RGBPixel& value) throw() {
    RGBAPixel result;
    result.rgba = value.rgb;
    result.alpha = ALPHA_OPAQUE_INTENSITY;
    return result;
  }

  typedef RGBPixel ColorPixel;
  
  template<class DEST, class SRC>
  class ConvertPixel {
  };

  template<class SRC>
  class ConvertPixel<RGBPixel, SRC> {
  public:

    inline RGBPixel operator()(const SRC& value) const throw() {
      return makeRGBPixel(value, value, value);
    }
  };

  template<>
  class ConvertPixel<RGBPixel, GrayPixel> {
  public:

    inline RGBPixel operator()(const GrayPixel& value) const throw() {
      return makeRGBPixel(value, value, value);
    }
  };
  
  template<>
  class ConvertPixel<RGBAPixel, GrayPixel> {
  private:

    const Intensity alpha;
  public:

    inline ConvertPixel(Intensity _alpha = ALPHA_OPAQUE_INTENSITY) throw() : alpha(_alpha) {
    }
    
    inline RGBAPixel operator()(const GrayPixel& value) const throw() {
      return makeRGBAPixel(value, value, value, alpha); // value * 0x010101 + alpha * 0x01000000
    }
  };

  template<>
  class ConvertPixel<RGBAPixel, RGBPixel> {
  private:

    const Intensity alpha;
  public:

    inline ConvertPixel(Intensity _alpha = ALPHA_OPAQUE_INTENSITY) throw() : alpha(_alpha) {
    }
    
    inline RGBAPixel operator()(const RGBPixel& value) const throw() {
      RGBAPixel result;
      result.rgba = value.rgb;
      result.alpha = alpha;
      return result;
    }
  };


  
  template<class DEST, class SRC>
  class ForcedConvertPixel {
  };

  template<>
  class ForcedConvertPixel<GrayPixel, long double> {
  public:

    /** Hint. */
    enum Policy {CLAMP, DIRECT};
  private:

    Policy policy;
  public:

    inline ForcedConvertPixel(Policy _policy = DIRECT) throw() : policy(_policy) {
    }

    inline GrayPixel operator()(long double value) const throw() {
      switch (policy) {
      case CLAMP:
        return (GrayPixel){static_cast<Intensity>(255 * value)}; // [0.0; 1.0] => [0; 255] // TAG: round to nearest or trunc
      case DIRECT:
        return (GrayPixel){static_cast<Intensity>(value)}; // [0.0; 255.0] => [0; 255] // TAG: round to nearest or trunc
      }
    }
  };

  template<>
  class ForcedConvertPixel<GrayPixel, RGBPixel> {
  public:

    /** Hint. */
    enum Policy {FASTEST, NEAREST};
  private:

    /** Hint. */
    Policy policy;
  public:

    inline ForcedConvertPixel(Policy _policy = NEAREST) throw() : policy(_policy) {
    }
    
    inline GrayPixel operator()(const RGBPixel& value) const throw() {
      switch (policy) {
      case FASTEST:
        return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue)/3};
      case NEAREST:
        return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3};
      }
    }
  };

  template<>
  class ForcedConvertPixel<GrayPixel, RGBAPixel> {
  public:

    /** Hint. */
    enum Policy {FASTEST, NEAREST};
  private:

    Policy policy;
  public:

    inline ForcedConvertPixel(Policy _policy = NEAREST) throw() : policy(_policy) {
    }
    
    inline GrayPixel operator()(const RGBAPixel& value) const throw() {
      switch (policy) {
      case FASTEST:
        return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue)/3};
      case NEAREST:
        return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3};
      }
    }
  };
  
  template<>
  class ForcedConvertPixel<long double, RGBPixel> {
  public:

    /** Hint. */
    enum Policy {FLOATING_INTENSITY, HONOUR_INTENSITY};
  private:

    Policy policy;
  public:

    inline ForcedConvertPixel(Policy _policy = HONOUR_INTENSITY) throw() : policy(_policy) {
    }
    
    inline long double operator()(const RGBPixel& value) const throw() {
      switch (policy) {
      case FLOATING_INTENSITY:
        return static_cast<unsigned int>(value.red) + value.green + value.blue;
      case HONOUR_INTENSITY:
        return (static_cast<unsigned int>(value.red) + value.green + value.blue)/3.0;
      }
    }
  };
  
  template<>
  class ForcedConvertPixel<long double, RGBAPixel> {
  public:

    /** Hint. */
    enum Policy {FLOATING_INTENSITY, HONOUR_INTENSITY};
  private:

    Policy policy;
  public:

    inline ForcedConvertPixel(Policy _policy = HONOUR_INTENSITY) throw() : policy(_policy) {
    }
    
    inline long double operator()(const RGBAPixel& value) const throw() {
      switch (policy) {
      case FLOATING_INTENSITY:
        return static_cast<unsigned int>(value.red) + value.green + value.blue;
      case HONOUR_INTENSITY:
        return (static_cast<unsigned int>(value.red) + value.green + value.blue)/3.0;
      }
    }
  };
}; // end of gip namespace

namespace base {

  template<>
  class Relocateable<gip::RGBPixel> {
  public:
    static const bool IS_RELOCATEABLE = Relocateable<unsigned int>::IS_RELOCATEABLE && Relocateable<gip::Intensity>::IS_RELOCATEABLE;
  };
  
  template<>
  class Relocateable<gip::RGBAPixel> {
  public:
    static const bool IS_RELOCATEABLE = Relocateable<unsigned int>::IS_RELOCATEABLE && Relocateable<gip::Intensity>::IS_RELOCATEABLE;
  };
  
  template<class TYPE>
  class Relocateable<gip::AnyRGBPixel<TYPE> > {
  public:
    static const bool IS_RELOCATEABLE = Relocateable<TYPE>::IS_RELOCATEABLE;
  };

  template<class TYPE>
  class Relocateable<gip::AnyRGBAPixel<TYPE> > {
  public:
    static const bool IS_RELOCATEABLE = Relocateable<TYPE>::IS_RELOCATEABLE;
  };
  
}; // end of base namespace

#endif
