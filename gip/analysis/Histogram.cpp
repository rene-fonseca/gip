/***************************************************************************
    Generic Image Processing (GIP) Framework
    A framework for developing image processing applications

    Copyright (C) 2001 by René Møller Fonseca <fonseca@mip.sdu.dk>

    This framework is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    For the licensing terms refer to the file 'LICENSE'.
 ***************************************************************************/

#include <gip/analysis/Histogram.h>

namespace gip {

ColorHistogram::ColorHistogram() throw(MemoryException) :
  blue(MAXIMUM_INTENSITY + 1, 0), green(MAXIMUM_INTENSITY + 1, 0), red(MAXIMUM_INTENSITY + 1, 0) {
}

void ColorHistogram::operator()(const Argument& value) throw() {
  blue[value.blue] = blue[value.blue] + 1; // TAG: not as fast as it could be
  green[value.green] = green[value.green] + 1;
  red[value.red] = red[value.red] + 1;
}

Histogram ColorHistogram::getBlueHistogram() const throw() {
  return blue;
}

Histogram ColorHistogram::getGreenHistogram() const throw() {
  return green;
}

Histogram ColorHistogram::getRedHistogram() const throw() {
  return red;
}



GrayHistogram::GrayHistogram() throw(MemoryException) :
  gray(MAXIMUM_INTENSITY + 1, 0) {
}

void GrayHistogram::operator()(const Argument& value) throw() {
  gray[value] = gray[value] + 1; // TAG: not as fast as it could be
}

Histogram GrayHistogram::getHistogram() const throw() {
  return gray;
}

}; // end of namespace
