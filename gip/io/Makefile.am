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

noinst_LTLIBRARIES = libio.la

libio_la_SOURCES = InvalidFormat.cpp GIFEncoder.cpp PCXEncoder.cpp JPEGEncoder.cpp BMPEncoder.cpp TIFFEncoder.cpp PNGEncoder.cpp ImageEncoder.cpp

sectiondir = $(includedir)/gip
section_HEADERS = ImageEncoder.h PNGEncoder.h TIFFEncoder.h BMPEncoder.h JPEGEncoder.h PCXEncoder.h GIFEncoder.h InvalidFormat.h

EXTRA_DIST =
