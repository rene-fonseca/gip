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
#include <base/Exception.h>

namespace gip {
  
  /**
    @defgroup exceptions Exceptions
  */

  /**
    Exception raised by GIP classes.

    @short Image exception.
    @ingroup exceptions
    @version 1.0
  */

  class _COM_AZURE_DEV__GIP__API ImageException : public Exception {
  public:

    /**
      Initializes the exception object without an associated message.
    */
    inline ImageException() throw() {
    }
    
    /**
      Initializes the exception object with the specified message.

      @param message The message.
    */
    inline ImageException(const char* message) throw() : Exception(message) {
    }
    
    /**
      Initializes the exception object without an associated message.
      
      @param type The identity of the type.
    */
    inline ImageException(Type type) throw() : Exception(type) {
    }
    
    /**
      Initializes the exception object.
      
      @param message An NULL-terminated string (ASCII).
      @param type The identity of the type.
    */
    inline ImageException(const char* message, Type type) throw()
      : Exception(message, type) {
    }
  };

}; // end of gip namespace
