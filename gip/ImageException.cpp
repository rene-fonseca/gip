/***************************************************************************
    begin                : Sun May 6 2001
    copyright            : (C) 2001 by Ren� M�ller Fonseca
    email                : fonseca@mip.sdu.dk
 ***************************************************************************/

#include <gip/ImageException.h>

namespace gip {

ImageException::ImageException() throw() {
}

ImageException::ImageException(const char* message) throw() : Exception(message) {
}

}; // end of namespace
