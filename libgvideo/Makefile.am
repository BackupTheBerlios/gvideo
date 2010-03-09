## Process this file with automake to produce Makefile.in

h_sources = gvideo.h \
            GVid_v4l2.h \
            GVid_v4l2_formats.h \
            GVid_options.h \
            GVid_color_convert.h \
            GVid_mjpg_dec.h \
            GVid_buff.h \
            GVid_image.h
            
cpp_sources = GVid_v4l2.cpp \
              GVid_v4l2_formats.cpp \
              GVid_options.cpp \
              GVid_color_convert.cpp \
              GVid_mjpg_dec.cpp \
              GVid_buff.cpp \
              GVid_image.cpp

#Install the headers in a versioned directory - gvideo-x.x/libgvrender:
library_includedir=$(includedir)/$(PACKAGE)-$(PACKAGE_RELEASE)/$(GVIDEO_LIBRARY_NAME)
library_include_HEADERS = $(h_sources) 

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common

lib_LTLIBRARIES= libgvideo.la
libgvideo_la_SOURCES= $(h_sources) $(cpp_sources)
libgvideo_la_CFLAGS = $(GVIDEOLIB_CFLAGS)
libgvideo_la_LIBADD= $(GVIDEOLIB_LIBS)
libgvideo_la_LDFLAGS= -version-info $(GVIDEO_LIBRARY_VERSION) -release $(GVIDEO_API_VERSION)


