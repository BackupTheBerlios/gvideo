## sub directory

h_sources = GVMatroska.h GVJpeg_encoder.h GVCodec.h
cpp_sources = GVMatroska.cpp GVJpeg_encoder.cpp GVCodec.cpp

#Install the headers in a versioned directory - gvideo-x.x/libgvencoder:
library_includedir=$(includedir)/$(PACKAGE)-$(PACKAGE_RELEASE)/$(GVENCODER_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common

lib_LTLIBRARIES= libgvencoder.la
libgvencoder_la_SOURCES= $(h_sources) $(cpp_sources)
libgvencoder_la_CXXFLAGS = $(GVENCODERLIB_CFLAGS) -D__STDC_CONSTANT_MACROS
libgvencoder_la_LIBADD= $(GVENCODERLIB_LIBS)
libgvencoder_la_LDFLAGS= -version-info $(GVENCODER_LIBRARY_VERSION) -release $(GVENCODER_API_VERSION)

