/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#pragma once

#include <gip/features.h>

namespace gip {

/**
  Clamp value to boundary.

  @short Clamp value to boundary
  @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
  @version 1.0
*/
template<class TYPE>
class Clamp : UnaryOperation<TYPE, TYPE> {
private:

  TYPE minimum;
  TYPE maximum;
public:

  inline Clamp(const TYPE& _minimum, const TYPE& _maximum) throw() : minimum(_minimum), maximum(_maximum) {
  }

  inline TYPE operator()(const TYPE& value) const throw() {
    if (value > minimum) {
      if (value < maximum) {
        return value;
      } else {
        return maximum;
      }
    } else {
      return minimum;
    }
  }

};

/**
  Clamps the value to the boundary specified by the minimum and maximum values.

  @param minimum The minimum value.
  @param value The value to be clamped.
  @param maximum The maximum value.
*/
template<class TYPE>
inline TYPE clamp(const TYPE& minimum, const TYPE& value, const TYPE& maximum) throw() {
  if (value > minimum) {
    if (value < maximum) {
      return value;
    } else {
      return maximum;
    }
  } else {
    return minimum;
  }
}

  template<class DESTITERATOR, class SRCITERATOR>
  inline SRCITERATOR copy(DESTITERATOR dest, DESTITERATOR end, SRCITERATOR src) {
    while (dest != end) {
      *dest = *src;
      ++dest;
      ++src;
    }
    return src;
  }

  template<class DESTITERATOR, class SRCITERATOR>
  inline DESTITERATOR copy(DESTITERATOR dest, SRCITERATOR src, SRCITERATOR end) {
    while (src != end) {
      *dest = *src;
      ++dest;
      ++src;
    }
    return dest;
  }

}; // end of gip namespace
