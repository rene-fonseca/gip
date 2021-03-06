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

#include <base/Object.h>
#include <base/OutOfRange.h>
#include <base/Dimension.h>
#include <gip/ArrayImage.h>
#include <gip/Region.h>

namespace gip {
  
  /**
    Segmentor used to divide a region onto smaller regions (complies with the strategy pattern).

    @see Segment
    @short Segmentor
    @version 1.0
  */

  class _COM_AZURE_DEV__GIP__API Segmentor : public Object {
  protected:

    /** The dimension to be segmented. */
    const Dimension dimension;
  public:

    /**
      Initializes the image segmentor.

      @param dimension The dimension to be segmented.
    */
    Segmentor(const Dimension& dimension) noexcept;

    /**
      Returns the dimension being segmented.
    */
    const Dimension& getDimension() const noexcept;

    /**
      Returns the region specified by row and column.

      @param row The row of the desired region.
      @param columns The column of the desired region.
    */
    virtual Region getRegion(unsigned int row, unsigned int column) const = 0;
  };

  inline const Dimension& Segmentor::getDimension() const noexcept {
    return dimension;
  }

}; // end of gip namespace
