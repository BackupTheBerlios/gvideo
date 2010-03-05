## sub directory

h_sources = GVid_sdl_render.h render.h
cpp_sources = GVid_sdl_render.cpp

#Install the headers in a versioned directory - gvideo-x.x/libgvrender:
library_includedir=$(includedir)/$(PACKAGE)-$(PACKAGE_RELEASE)/$(GVRENDER_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common

lib_LTLIBRARIES= libgvrender.la
libgvrender_la_SOURCES= $(h_sources) $(cpp_sources)
libgvrender_la_CFLAGS= $(GVRENDERLIB_CFLAGS)
libgvrender_la_LIBADD= $(GVRENDERLIB_LIBS)
libgvrender_la_LDFLAGS= -version-info $(GVRENDER_LIBRARY_VERSION) -release $(GVRENDER_API_VERSION)

