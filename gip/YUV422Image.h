/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/ArrayImage.h>

namespace gip {

  /**
    YUV 422 image.
    
    @short YUV 422 image.
    @ingroup images
    @version 1.0
  */
  class YUV422Image : public Image<uint8> {
  private:

    /** Y component. */
    ArrayImage<uint8> y;
    /** U component. */
    ArrayImage<uint8> u;
    /** V component. */
    ArrayImage<uint8> v;
  public:

    /**
      Initializes dummy image.
    */
    YUV422Image() noexcept;
    
    /**
      Initializes YUV 422 image. Raises MemoryException if dimension is invalid.

      @param dimension The dimension of the image.
    */
    YUV422Image(const Dimension& dimension) throw(MemoryException);

    /**
      Initialization of image by image.
    */
    YUV422Image(const YUV422Image& copy) noexcept;

    /**
      Default assigment of image.
    */
    YUV422Image& operator=(const YUV422Image& eq) noexcept;

    /**
      Returns the Y component frame.
    */
    ArrayImage<uint8> getY() noexcept;

    /**
      Returns the Y component frame.
    */
    const ArrayImage<uint8> getY() const noexcept;

    /**
      Returns the U component frame.
    */
    ArrayImage<uint8> getU() noexcept;
    
    /**
      Returns the U component frame.
    */
    const ArrayImage<uint8> getU() const noexcept;

    /**
      Returns the V component frame.
    */
    ArrayImage<uint8> getV() noexcept;
    
    /**
      Returns the V component frame.
    */
    const ArrayImage<uint8> getV() const noexcept;
  };
  
}; // end of namespace gip
