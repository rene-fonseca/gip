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

#include <gip/features.h>
#include <base/string/String.h>

/**
  @defgroup video Video
*/

namespace gip {

  /**
    This class is the base class of all camera classes.

    @short Camera base class
    @ingroup video
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  class Camera : public Object {
  public:

    /**
      Returns the vendor of the camera.
    */
    virtual String getVendorName() const throw() = 0;

    /**
      Returns the model of the camera.
    */
    virtual String getModelName() const throw() = 0;
  };

}; // end of gip namespace
