/***************************************************************************
    begin                : Sun May 6 2001
    copyright            : (C) 2001 by René Møller Fonseca
    email                : fonseca@mip.sdu.dk
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__IMAGE_EXCEPTION_H
#define _DK_SDU_MIP__GIP__IMAGE_EXCEPTION_H

#include <base/Exception.h>

using namespace base;

namespace gip {

/**
  Exception thrown by GIP classes.

  @short Image exception.
  @author René Møller Fonseca
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
};

}; // end of namespace

#endif
