/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__TEST_H
#define _DK_SDU_MIP__GIP__TEST_H

#include <gip/transformation/UnaryTransformation.h>

namespace gip {

/**
  Test transformation.

  @author René Møller Fonseca
*/

class Test2 : public UnaryTransformation<ColorImage> {
public:
	
	Test2(DestinationImage* destination) throw();
	
	void operator()() throw();
};

}; // end of namespace

#endif
