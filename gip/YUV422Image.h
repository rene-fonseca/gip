/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

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
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
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
    YUV422Image() throw();
    
    /**
      Initializes YUV 422 image. Raises MemoryException if dimension is invalid.

      @param dimension The dimension of the image.
    */
    YUV422Image(const Dimension& dimension) throw(MemoryException);

    /**
      Initialization of image by image.
    */
    YUV422Image(const YUV422Image& copy) throw();

    /**
      Default assigment of image.
    */
    YUV422Image& operator=(const YUV422Image& eq) throw();

    /**
      Returns the Y component frame.
    */
    ArrayImage<uint8> getY() throw();

    /**
      Returns the Y component frame.
    */
    const ArrayImage<uint8> getY() const throw();

    /**
      Returns the U component frame.
    */
    ArrayImage<uint8> getU() throw();
    
    /**
      Returns the U component frame.
    */
    const ArrayImage<uint8> getU() const throw();

    /**
      Returns the V component frame.
    */
    ArrayImage<uint8> getV() throw();
    
    /**
      Returns the V component frame.
    */
    const ArrayImage<uint8> getV() const throw();
  };
  
}; // end of namespace gip
