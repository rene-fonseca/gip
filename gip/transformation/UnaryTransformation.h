/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__UNARY_TRANSFORMATION_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__UNARY_TRANSFORMATION_H

#include <base/Object.h>
#include <gip/gip.h>

namespace gip {

/**
  Unary transformation.

  @short Unary transformation.
  @author René Møller Fonseca
*/

template<class DEST>
class UnaryTransformation : public Object {
public:

  typedef DEST DestinationImage;
protected:

  /** The destination image of the transformation. */
  DestinationImage* destination;
public:

  UnaryTransformation(DestinationImage* destination) throw();
};

template<class DEST>
inline UnaryTransformation<DEST>::UnaryTransformation(DestinationImage* dest) throw() :
  destination(dest) {
}

}; // end of namespace

#endif
