/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__PIXEL_TRAITS_H
#define _DK_SDU_MIP__GIP__PIXEL_TRAITS_H

#include <gip/features.h>
#include <base/Functor.h>

namespace gip {

  /**
    Specifies the default type which is used when calculations are performed on
    a component of a pixel. The type MUST be signed.
  */
  template<class COMPONENT>
  class PixelComponent {
  public:
    typedef COMPONENT Arithmetic;
  };
  
  template<>
  class PixelComponent<char> {
  public:
    typedef int Arithmetic;
  };
  
  template<>
  class PixelComponent<unsigned char> {
  public:
    typedef int Arithmetic;
  };
  
  template<>
  class PixelComponent<short> {
  public:
    typedef int Arithmetic;
  };
  
  template<>
  class PixelComponent<unsigned short> {
  public:
    typedef int Arithmetic;
  };
  
  template<>
  class PixelComponent<unsigned int> {
  public:
    typedef int Arithmetic;
  };

  template<>
  class PixelComponent<unsigned long> {
  public:
    typedef long Arithmetic;
  };

  template<>
  class PixelComponent<unsigned long long> {
  public:
    typedef long long Arithmetic;
  };



  template<class SRC, class DEST>
  inline DEST mapToOneDimension(const SRC& value) throw() {
    return value;
  }

  
  
  template<class PIXEL>
  class PixelTraits {
  public:
    
    typedef PIXEL Pixel;
    typedef Pixel Component;
    typedef PixelComponent<Component>::Arithmetic Arithmetic;

    enum {
      MINIMUM = 0x00,
      MAXIMUM = 0xff // TAG: 1 if floating-point type
    };
    
    class GetOrder : public UnaryOperation<Pixel, Arithmetic> {
    public:
      
      inline Arithmetic operator()(const Pixel& pixel) const throw() {
        return mapToOneDimension(pixel);
      }
    };
  };



  /** Specifies if true that the pixel is a value type which doesn't have any compoenents. */
  template<class PIXEL>
  class IsGrayPixel {
  public:
    enum {IS_GRAY_PIXEL = false};
    // TAG: true if doesn't have any known component: gray, red, green, blue, or alpha
    // TAG: or remove 'cause all pixels must be compatible with a 'gray' pixel (intensity)
  };

  /** Specifies when true that the pixel has a gray and alpha component. */
  template<class PIXEL>
  class IsGrayAlphaPixel {
  public:
    enum {IS_GRAY_ALPHA_PIXEL = false};
  };

  /** Specifies when true that the pixel has red, green, and blue components. */
  template<class PIXEL>
  class IsRGBPixel {
  public:
    enum {IS_RGB_PIXEL = false};
  };

  /** Specifies when true that the pixel has red, green, blue, and alpha components. */
  template<class PIXEL>
  class IsRGBAPixel {
  public:
    enum {IS_RGBA_PIXEL = false};
  };



  /** Specifies the the pixel has a gray component. */
  template<class PIXEL>
  class HasGrayComponent {
  public:
    enum {HAS_GRAY_COMPONENT = false};
  };

  /** Specifies that the pixel has a red component. */
  template<class PIXEL>
  class HasRedComponent {
  public:
    enum {HAS_RED_COMPONENT = false};
  };

  /** Specifies that the pixel has a green component. */
  template<class PIXEL>
  class HasGreenComponent {
  public:
    enum {HAS_GREEN_COMPONENT = false};
  };
  
  /** Specifies that the pixel has a blue component. */
  template<class PIXEL>
  class HasBlueComponent {
  public:
    enum {HAS_BLUE_COMPONENT = false};
  };
  
  /** Specifies that the pixel has an alpha component. */
  template<class PIXEL>
  class HasAlphaComponent {
  public:
    enum {HAS_ALPHA_COMPONENT = false};
  };
  
}; // end of namespace

#endif
