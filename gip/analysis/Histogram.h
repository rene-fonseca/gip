/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#ifndef _DK_SDU_MIP__GIP__HISTOGRAM_H
#define _DK_SDU_MIP__GIP__HISTOGRAM_H

#include <gip/gip.h>
#include <base/collection/Array.h>

namespace gip {

typedef Array<unsigned int> Histogram;

/**
  Gray level histogram operation.

  @short Gray level histogram operation.
  @author René Møller Fonseca
*/

class GrayHistogram : public UnaryOperation<GrayPixel, void> {
private:

  /** The gray histogram. */
  Histogram gray;
public:

  GrayHistogram() throw(MemoryException);

  void operator()(const Argument& value) throw();

  Histogram getHistogram() const throw();
};



/**
  Color histogram operation.

  @short Color histogram operation.
  @author René Møller Fonseca
*/

class ColorHistogram : public UnaryOperation<ColorPixel, void> {
private:

  /** The blue histogram. */
  Histogram blue;
  /** The green histogram. */
  Histogram green;
  /** The red histogram. */
  Histogram red;
public:

  ColorHistogram() throw(MemoryException);

  void operator()(const Argument& value) throw();

  Histogram getBlueHistogram() const throw();

  Histogram getGreenHistogram() const throw();

  Histogram getRedHistogram() const throw();
};

}; // end of namespace

#endif
