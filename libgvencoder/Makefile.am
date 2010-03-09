## sub directory

h_sources = GVMatroska.h
cpp_sources = GVMatroska.cpp

#Install the headers in a versioned directory - gvideo-x.x/libgvencoder:
library_includedir=$(includedir)/$(PACKAGE)-$(PACKAGE_RELEASE)/$(GVENCODER_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common

lib_LTLIBRARIES= libgvencoder.la
libgvencoder_la_SOURCES= $(h_sources) $(cpp_sources)
libgvencoder_la_CFLAGS = $(GVENCODERLIB_CFLAGS)
libgvencoder_la_LIBADD= $(GVENCODERLIB_LIBS)
libgvencoder_la_LDFLAGS= -version-info $(GVENCODER_LIBRARY_VERSION) -release $(GVENCODER_API_VERSION)

