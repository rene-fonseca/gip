/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__IMAGE_EXCEPTION_H
#define _DK_SDU_MIP__GIP__IMAGE_EXCEPTION_H

#include <gip/features.h>
#include <base/Exception.h>

namespace gip {
  
  /**
    @defgroup exceptions Exceptions
  */

  /**
    Exception thrown by GIP classes.

    @short Image exception
    @ingroup exceptions
    @author Rene Moeller Fonseca <fonseca@mip.sdu.dk>
    @version 1.0
  */

  class ImageException : public Exception {
  public:

    /**
      Initializes the exception object without an associated message.
    */
    ImageException() throw();

    /**
      Initializes the exception object with the specified message.

      @param message The message.
    */
    ImageException(const char* message) throw();

    /**
      Initializes the exception object without an associated message.
      
      @param type The identity of the type.
    */
    ImageException(Type type) throw() : Exception(type) {}

    /**
      Initializes the exception object.
      
      @param message An NULL-terminated string (ASCII).
      @param type The identity of the type.
    */
    ImageException(const char* message, Type type) throw() : Exception(message, type) {}
  };

}; // end of namespace

#endif
