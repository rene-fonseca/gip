/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Ren� M�ller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_TRANSFORMATION__DUPLICATE_H
#define _DK_SDU_MIP__GIP_TRANSFORMATION__DUPLICATE_H

#include <gip/transformation/Transformation.h>
#include <gip/ArrayImage.h>
#include <gip/analysis/traverse.h>

namespace gip {

// temporary - should be enable in base
template<class TYPE>
class Same : UnaryOperation<TYPE, TYPE> {
public:
  inline Result operator()(const Argument& value) const throw() {return value;}
};

/**
  Duplicates the contents of an image.

  @author Ren� M�ller Fonseca
*/

class Duplicate : public Transformation<ColorImage, ColorImage> {
public:

  /**
    Initializes duplication object.

    @param destination The destination image.
    @param source The source image.
  */
  Duplicate(DestinationImage* destination, const SourceImage* source) throw();

  /**
    Duplicates the contents of the source image to the destination image.
  */
  void operator()() throw();
};

Duplicate::Duplicate(DestinationImage* destination, const SourceImage* source) throw() :
  Transformation<DestinationImage, SourceImage>(destination, source) {
}

void Duplicate::operator()() throw() {
  Same<ColorPixel> operation;
  fillWithUnary(*destination, *source, operation);
}

}; // end of namespace

#endif