/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__INVALID_FORMAT_H
#define _DK_SDU_MIP__GIP_IO__INVALID_FORMAT_H

#include <gip/ImageException.h>

namespace gip {

  /**
    Exception thrown by GIP classes.

    @short Image exception.
    @ingroup imageEncoders
    @author Rene Moeller Fonseca
    @version 1.0
  */

  class InvalidFormat : public ImageException {
  public:

    /**
      Initializes the exception object without an associated message.
    */
    InvalidFormat() throw();

    /**
      Initializes the exception object with the specified message.

      @param message The message.
    */
    InvalidFormat(const char* message) throw();

    InvalidFormat(Type type) throw() : ImageException(type) {}

    InvalidFormat(const char* message, Type type) throw() : ImageException(message, type) {}
  };

}; // end of namespace

#endif
