/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Ren� M�ller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__SEGMENTOR_H
#define _DK_SDU_MIP__GIP__SEGMENTOR_H

#include <base/Object.h>
#include <base/OutOfRange.h>
#include <gip/ArrayImage.h>
#include <gip/Region.h>

using namespace base;

namespace gip {

/**
  Segmentor used to divide a region onto smaller regions (complies with the strategy pattern).

  @see Segment
  @short Segmentor
  @author Ren� M�ller Fonseca
*/

class Segmentor : public Object {
protected:

  /** The dimension to be segmented. */
  const Dimension dimension;
public:

  /**
    Initializes the image segmentor.

    @param dimension The dimension to be segmented.
  */
  Segmentor(const Dimension& dimension) throw();

  /**
    Returns the dimension being segmented.
  */
  const Dimension& getDimension() const throw();

  /**
    Returns the region specified by row and column.

    @param row The row of the desired region.
    @param columns The column of the desired region.
  */
  virtual Region getRegion(unsigned int row, unsigned int column) const throw(OutOfRange) = 0;
};

inline const Dimension& Segmentor::getDimension() const throw() {
  return dimension;
}

}; // end of namespace

#endif
