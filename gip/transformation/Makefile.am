# ***************************************************************************
#    Generic Image Processing (GIP) Framework
#    A framework for developing image processing applications
#
#    Copyright (C) 2001-2002 by Rene Moeller Fonseca <fonseca@mip.sdu.dk>
#
#    This framework is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
#
#    For the licensing terms refer to the file 'LICENSE'.
# ***************************************************************************

include $(top_srcdir)/common.am

noinst_LTLIBRARIES = libtransformation.la

libtransformation_la_SOURCES = WalshTransformation.cpp IntensityMapper.cpp Mirror.cpp Flip.cpp MinMax.cpp Convert.cpp Gradient.cpp FourierTransformation.cpp Test.cpp TSRTransformation.cpp LinearScale.cpp DrawRectangle.cpp UnaryTransformation.cpp Noise.cpp Transformation.cpp Duplicate.cpp Scale.cpp StraightLineHoughTransformation.cpp Convolution3x3.cpp MedianFilter3x3.cpp ContrastStretch.cpp EqualizeHistogram.cpp Erode.cpp Dilate.cpp

sectiondir = $(includedir)/gip
section_HEADERS = Scale.h Duplicate.h Transformation.h Noise.h UnaryTransformation.h DrawRectangle.h LinearScale.h TSRTransformation.h Test.h FourierTransformation.h Gradient.h Convert.h MinMax.h Flip.h Mirror.h IntensityMapper.h WalshTransformation.h StraightLineHoughTransformation.h Convolution3x3.h MedianFilter3x3.h ContrastStretch.h EqualizeHistogram.h Erode.h Dilate.h

EXTRA_DIST =
