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

libtransformation_la_SOURCES = Convert.cpp ContrastStretch.cpp Convolution3x3.cpp DiscreteCosineTransformation.cpp Dilate.cpp Duplicate.cpp Erode.cpp EqualizeHistogram.cpp Flip.cpp FourierTransformation.cpp Gradient.cpp HaarTransformation.cpp IntensityMapper.cpp LinearScale.cpp MedianFilter3x3.cpp MinMax.cpp Mirror.cpp Noise.cpp Scale.cpp StraightLineHoughTransformation.cpp Test.cpp Transformation.cpp TSRTransformation.cpp UnaryTransformation.cpp WalshTransformation.cpp

sectiondir = $(includedir)/gip/transformation
section_HEADERS = Crop.h DiscreteCosineTransformation.h Duplicate.h FourierExchange.h HaarTransformation.h Tile.h Transformation.h Noise.h UnaryTransformation.h LinearScale.h TSRTransformation.h Test.h FourierTransformation.h Gradient.h Convert.h MinMax.h Flip.h Mirror.h IntensityMapper.h WalshTransformation.h StraightLineHoughTransformation.h MedianFilter3x3.h ContrastStretch.h Convolution3x3.h Dilate.h Erode.h EqualizeHistogram.h Scale.h

EXTRA_DIST =
