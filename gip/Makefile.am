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

lib_LTLIBRARIES = libgip.la

libgip_la_SOURCES = Point2D.cpp Point.cpp ImageException.cpp ArrayImage.cpp CoordinateMapper.cpp Region.cpp Segmentor.cpp Pixel.cpp ShadowImage.cpp Subimage.cpp Image.cpp Version.cpp

libgip_la_LIBADD = transformation/libtransformation.la analysis/libanalysis.la canvas/libcanvas.la io/libio.la media/libmedia.la -lbase @GIP_LIBADD_ADD@ -lpng -ljpeg

libgip_la_LDFLAGS = -avoid-version -module -no-undefined -version-info 0:1:0 @GIP_LDFLAGS_ADD@ @BASE_LDFLAGS_ADD@

sectiondir = $(includedir)/gip
section_HEADERS = features.h gip.h ArrayImage.h CoordinateMapper.h Image.h ImageException.h PixelTraits.h GrayAlphaPixel.h HSLPixel.h RGBPixel.h RGBAPixel.h Pixel.h Point2D.h Point.h Region.h Segmentor.h  ShadowImage.h Subimage.h Version.h YCbCrPixel.h

libgip_la_DEPENDENCIES = transformation/libtransformation.la analysis/libanalysis.la canvas/libcanvas.la io/libio.la media/libmedia.la @GIP_DEPENDENCIES_ADD@

# Windows version resource
# TAG: move to platform specific directory
winrcver.o: gip.rc
	cp $< temp.h
	windres --preprocessor "${CXXCPP} ${INCLUDES} ${CPPFLAGS}" --output-format coff temp.h $@
	-rm -f temp.h

EXTRA_DIST =

SUBDIRS = docs analysis io media canvas transformation
