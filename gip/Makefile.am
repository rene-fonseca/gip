####### kdevelop will overwrite this part!!! (begin)##########
bin_PROGRAMS = gip
gip_SOURCES = Point2D.cpp ImageException.cpp ArrayImage.cpp CoordinateMapper.cpp Region.cpp Segmentor.cpp Pixel.cpp ShadowImage.cpp Subimage.cpp Image.cpp main.cpp 
gip_LDADD   = ./transformation/libtransformation.a ./io/libio.a ./analysis/libanalysis.a ./draw/libdraw.a ./media/libmedia.a -lbase

SUBDIRS = docs transformation io analysis draw media 

EXTRA_DIST = main.cpp Image.cpp Image.h Subimage.cpp Subimage.h ShadowImage.cpp ShadowImage.h Pixel.cpp Pixel.h Segmentor.cpp Segmentor.h Region.cpp Region.h CoordinateMapper.cpp CoordinateMapper.h ArrayImage.cpp ArrayImage.h gip.h features.h ImageException.cpp ImageException.h Point2D.h Point2D.cpp 

####### kdevelop will overwrite this part!!! (end)############
DEFS =
bin_PROGRAMS = gip

# set the include path found by configure
INCLUDES = $(all_includes)
 
# the library search path.
gip_LDFLAGS = $(all_libraries) 
