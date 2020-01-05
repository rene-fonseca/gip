/***************************************************************************
    Generic Image Processing (GIP) Framework (Test Suite)
    A framework for developing image processing applications

    See COPYRIGHT.txt for details.

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/YUV422Image.h>

namespace gip {

  YUV422Image::YUV422Image() noexcept {
  }
    
  YUV422Image::YUV422Image(const Dimension& dimension) throw(MemoryException)
          : Image<uint8>(dimension), y(dimension) {
    Dimension d = Dimension(dimension.getWidth()/2, dimension.getHeight()/2);
    u = ArrayImage<uint8>(d);
    v = ArrayImage<uint8>(d);
  }

  YUV422Image::YUV422Image(const YUV422Image& copy) noexcept
          : y(copy.y), u(copy.u), v(copy.v) {
  }

  YUV422Image& YUV422Image::operator=(const YUV422Image& eq) noexcept {
    y = eq.y;
    u = eq.u;
    v = eq.v;
    return *this;
  }

  ArrayImage<uint8> YUV422Image::getY() noexcept {
    return y;
  }

  const ArrayImage<uint8> YUV422Image::getY() const noexcept {
    return y;
  }

  ArrayImage<uint8> YUV422Image::getU() noexcept {
    return u;
  }

  const ArrayImage<uint8> YUV422Image::getU() const noexcept {
    return u;
  }

  ArrayImage<uint8> YUV422Image::getV() noexcept {
    return v;
  }
  
  const ArrayImage<uint8> YUV422Image::getV() const noexcept {
    return v;
  }
  
}; // end of namespace gip
