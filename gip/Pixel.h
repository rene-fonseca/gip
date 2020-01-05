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

#include <gip/PixelTraits.h>
#include <gip/GrayAlphaPixel.h>
#include <gip/RGBPixel.h>
#include <gip/RGBAPixel.h>
#include <base/Functor.h>
#include <base/math/Complex.h>

namespace gip {
  
  /**
    @defgroup pixels Pixels
  */

  /**
    Image element wrapper for a single intensity (e.g. gray level) with the
    intensity represented by one byte.

    @short Achromatic image element
    @ingroup pixels
  */
  typedef int GrayPixel;
  
  template<>
  class PixelTraits<GrayPixel> {
  public:
    
    typedef unsigned char Component;
    typedef GrayPixel Pixel;
    typedef PixelComponent<Component>::Arithmetic Arithmetic;
    
    enum {
      MINIMUM = 0x00,
      MAXIMUM = 0xff
    };

    class GetOrder : public UnaryOperation<Pixel, Arithmetic> {
    public:
      
      inline Arithmetic operator()(const Pixel& pixel) const noexcept {
        return pixel;
      }
    };
  };



  /**
    Blends two pixels according to the specified level of opacity.

    @param back The pixel in the background.
    @param front The pixel in the foreground.
    @param opaque The level indicating a fully opaque pixel.
    @param opacity The opacity level of the new pixel. This must be in the range [0; opaque].
  */
  template<class PIXEL>
  inline PIXEL blend(PIXEL back, PIXEL front, unsigned int opaque, unsigned int opacity) noexcept {
    return ((opaque - opacity) * static_cast<typename PixelTraits<PIXEL>::Arithmetic>(back) + opacity * static_cast<typename PixelTraits<PIXEL>::Arithmetic>(front))/opaque;
  }



  template<>
  inline Complex::Type mapToOneDimension<Complex, Complex::Type>(const Complex& value) noexcept {
    return value.getSqrModulus();
  }



  template<class DEST, class SRC>
  inline DEST convertPixel(const SRC& value) noexcept {
    return value;
  }

  template<>
  inline Complex convertPixel<Complex, float>(const float& value) noexcept {
    return value;
  }

  template<>
  inline Complex convertPixel<Complex, double>(const double& value) noexcept {
    return Complex(value);
  }

  template<>
  inline float convertPixel<float, GrayPixel>(const GrayPixel& value) noexcept {
    return float(value);
  }

  template<>
  inline GrayPixel convertPixel<GrayPixel, ColorPixel>(const ColorPixel& value) noexcept {
    return (GrayPixel)((static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3); // looses information
  }

//   template<>
//   inline GrayPixel convertPixel<GrayPixel, ColorAlphaPixel>(const ColorAlphaPixel& value) noexcept {
//     return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3}; // looses information
//   }

//   template<>
//   inline float convertPixel<float, Complex>(const Complex& value) noexcept {
//     return float(value.getReal()); // looses information
//   }

//   template<>
//   inline RGBPixel<float> convertPixel<RGBPixel<float>, ColorPixel>(const ColorPixel& value) noexcept {
//     RGBPixel<float> result;
//     result.red = value.red;
//     result.green = value.green;
//     result.blue = value.blue;
//     return result;
//   }

//   template<>
//   inline RGBPixel<float> convertPixel<RGBPixel<float>, ColorAlphaPixel>(const ColorAlphaPixel& value) noexcept {
//     RGBPixel<float> result;
//     result.red = value.red;
//     result.green = value.green;
//     result.blue = value.blue;
//     return result;
//   }

//   template<>
//   inline RGBPixel<double> convertPixel<RGBPixel<double>, ColorPixel>(const ColorPixel& value) noexcept {
//     RGBPixel<double> result;
//     result.red = value.red;
//     result.green = value.green;
//     result.blue = value.blue;
//     return result;
//   }

//   template<>
//   inline RGBPixel<double> convertPixel<RGBPixel<double>, ColorAlphaPixel>(const ColorAlphaPixel& value) noexcept {
//     RGBPixel<double> result;
//     result.red = value.red;
//     result.green = value.green;
//     result.blue = value.blue;
//     return result;
//   }

//   template<>
//   inline RGBPixel<double> convertPixel<RGBPixel<double>, RGBPixel<float> >(const RGBPixel<float>& value) noexcept {
//     RGBPixel<double> result;
//     result.red = value.red;
//     result.green = value.green;
//     result.blue = value.blue;
//     return result;
//   }

//   template<>
//   inline GrayPixel convertPixel<GrayPixel, float>(const float& value) noexcept {
//     return GrayPixel(static_cast<unsigned char>(value)); // looses information
//   }

   template<>
   inline ColorPixel convertPixel<ColorPixel, GrayPixel>(const GrayPixel& value) noexcept {
     ColorPixel result;
     result.red = value;
     result.green = value;
     result.blue = value;
     return result;
   }

   template<>
   inline ColorAlphaPixel convertPixel<ColorAlphaPixel, GrayPixel>(const GrayPixel& value) noexcept {
     ColorAlphaPixel result;
     result.red = value;
     result.green = value;
     result.blue = value;
     result.alpha = 0xff; // ALPHA_OPAQUE_INTENSITY;
     return result;
   }

   template<>
   inline ColorAlphaPixel convertPixel<ColorAlphaPixel, ColorPixel>(const ColorPixel& value) noexcept {
     ColorAlphaPixel result;
     result.rgba = value.rgb;
     result.alpha = 0xff; // ALPHA_OPAQUE_INTENSITY;
     return result;
   }
  
   template<class DEST, class SRC>
   class ConvertPixel {
   };

   template<class SRC>
   class ConvertPixel<ColorPixel, SRC> {
   public:

     inline ColorPixel operator()(const SRC& value) const noexcept {
       return makeColorPixel(value, value, value);
     }
   };

//   template<>
//   class ConvertPixel<ColorPixel, GrayPixel> {
//   public:

//     inline ColorPixel operator()(const GrayPixel& value) const noexcept {
//       return makeColorPixel(value, value, value);
//     }
//   };
  
   template<>
   class ConvertPixel<ColorAlphaPixel, GrayPixel> {
   private:

     const unsigned char alpha;
   public:

     inline ConvertPixel(unsigned char _alpha = 0xff /*ALPHA_OPAQUE_INTENSITY*/) noexcept : alpha(_alpha) {
     }
     
     inline ColorAlphaPixel operator()(const GrayPixel& value) const noexcept {
       return makeColorAlphaPixel(value, value, value, alpha); // value * 0x010101 + alpha * 0x01000000
     }
   };

  template<>
  class ConvertPixel<ColorAlphaPixel, ColorPixel> {
  private:

    const unsigned char alpha;
  public:

    inline ConvertPixel(unsigned char _alpha = 0xff /*ALPHA_OPAQUE_INTENSITY*/) noexcept : alpha(_alpha) {
    }
    
    inline ColorAlphaPixel operator()(const ColorPixel& value) const noexcept {
      ColorAlphaPixel result;
      result.rgba = value.rgb;
      result.alpha = alpha;
      return result;
    }
  };


  
//   template<class DEST, class SRC>
//   class ForcedConvertPixel {
//   };

//   template<>
//   class ForcedConvertPixel<GrayPixel, long double> {
//   public:

//     /** Hint. */
//     enum Policy {CLAMP, DIRECT};
//   private:

//     Policy policy;
//   public:

//     inline ForcedConvertPixel(Policy _policy = DIRECT) noexcept : policy(_policy) {
//     }

//     inline GrayPixel operator()(long double value) const noexcept {
//       switch (policy) {
//       case CLAMP:
//         return (GrayPixel){static_cast<unsigned char>(255 * value)}; // [0.0; 1.0] => [0; 255] // TAG: round to nearest or trunc
//       case DIRECT:
//         return (GrayPixel){static_cast<unsigned char>(value)}; // [0.0; 255.0] => [0; 255] // TAG: round to nearest or trunc
//       }
//     }
//   };

//   template<>
//   class ForcedConvertPixel<GrayPixel, ColorPixel> {
//   public:

//     /** Hint. */
//     enum Policy {FASTEST, NEAREST};
//   private:

//     /** Hint. */
//     Policy policy;
//   public:

//     inline ForcedConvertPixel(Policy _policy = NEAREST) noexcept : policy(_policy) {
//     }
    
//     inline GrayPixel operator()(const ColorPixel& value) const noexcept {
//       switch (policy) {
//       case FASTEST:
//         return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue)/3};
//       case NEAREST:
//         return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3};
//       }
//     }
//   };

//   template<>
//   class ForcedConvertPixel<GrayPixel, ColorAlphaPixel> {
//   public:

//     /** Hint. */
//     enum Policy {FASTEST, NEAREST};
//   private:

//     Policy policy;
//   public:

//     inline ForcedConvertPixel(Policy _policy = NEAREST) noexcept : policy(_policy) {
//     }
    
//     inline GrayPixel operator()(const ColorAlphaPixel& value) const noexcept {
//       switch (policy) {
//       case FASTEST:
//         return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue)/3};
//       case NEAREST:
//         return (GrayPixel){(static_cast<unsigned int>(value.red) + value.green + value.blue + 1)/3};
//       }
//     }
//   };
  
//   template<>
//   class ForcedConvertPixel<long double, ColorPixel> {
//   public:

//     /** Hint. */
//     enum Policy {FLOATING_INTENSITY, HONOUR_INTENSITY};
//   private:

//     Policy policy;
//   public:

//     inline ForcedConvertPixel(Policy _policy = HONOUR_INTENSITY) noexcept : policy(_policy) {
//     }
    
//     inline long double operator()(const ColorPixel& value) const noexcept {
//       switch (policy) {
//       case FLOATING_INTENSITY:
//         return static_cast<unsigned int>(value.red) + value.green + value.blue;
//       case HONOUR_INTENSITY:
//         return (static_cast<unsigned int>(value.red) + value.green + value.blue)/3.0;
//       }
//     }
//   };
  
//   template<>
//   class ForcedConvertPixel<long double, ColorAlphaPixel> {
//   public:

//     /** Hint. */
//     enum Policy {FLOATING_INTENSITY, HONOUR_INTENSITY};
//   private:

//     Policy policy;
//   public:

//     inline ForcedConvertPixel(Policy _policy = HONOUR_INTENSITY) noexcept : policy(_policy) {
//     }
    
//     inline long double operator()(const ColorAlphaPixel& value) const noexcept {
//       switch (policy) {
//       case FLOATING_INTENSITY:
//         return static_cast<unsigned int>(value.red) + value.green + value.blue;
//       case HONOUR_INTENSITY:
//         return (static_cast<unsigned int>(value.red) + value.green + value.blue)/3.0;
//       }
//     }
//   };

}; // end of gip namespace
