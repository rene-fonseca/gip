/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by Ren� M�ller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP_IO__PNG_ENCODER_H
#define _DK_SDU_MIP__GIP_IO__PNG_ENCODER_H

#include <gip/io/ImageEncoder.h>

namespace gip {

/**
  Portable Network Graphics (PNG) format encoder/decoder.

  @short PNG format encoder/decoder.
  @author Ren� M�ller Fonseca
*/

class PNGEncoder : public ImageEncoder {
public:
	
	PNGEncoder();
	
	ColorImage read(InputStream& stream) throw(IOException);

	void write(OutputStream& stream, const ColorImage& image) throw(IOException);
};

}; // end of namespace

#endif