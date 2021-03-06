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
    @version 1.0
  */

  class _COM_AZURE_DEV__GIP__API Camera : public Object {
  public:

    /**
      Returns the vendor of the camera.
    */
    virtual String getVendorName() const noexcept = 0;

    /**
      Returns the model of the camera.
    */
    virtual String getModelName() const noexcept = 0;
  };

}; // end of gip namespace
