DEFS =
DEFAULT_INCLUDES =

bin_PROGRAMS = gip
lib_LTLIBRARIES = libgip.la

libgip_la_SOURCES = Point2D.cpp ImageException.cpp ArrayImage.cpp CoordinateMapper.cpp Region.cpp Segmentor.cpp Pixel.cpp ShadowImage.cpp Subimage.cpp Image.cpp Version.cpp

libgip_la_LIBADD = transformation/libtransformation.la analysis/libanalysis.la draw/libdraw.la io/libio.la media/libmedia.la @BASE_LIBADD@ @GIP_ADDITIONAL_LIBADD@

libgip_la_LDFLAGS = -avoid-version -module -no-undefined -version-info 0:1:0 @GIP_ADDITIONAL_LDFLAGS@

sectiondir = $(includedir)/gip
section_HEADERS = features.h gip.h ArrayImage.h CoordinateMapper.h Image.h ImageException.h Pixel.h Point2D.h Region.h Segmentor.h  ShadowImage.h Subimage.h Version.h

libgip_la_DEPENDENCIES = transformation/libtransformation.la analysis/libanalysis.la draw/libdraw.la io/libio.la media/libmedia.la @GIP_ADDITIONAL_DEPENDENCIES@

winrcver.o: gip.rc
	windres --preprocessor "${CXXCPP} -xc-header ${CPPFLAGS}" --output-format coff $< $@

gip_SOURCES = main.cpp
gip_LDADD = libgip.la
gip_LDFLAGS =

EXTRA_DIST =

SUBDIRS = docs transformation analysis draw io media
