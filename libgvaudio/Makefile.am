## sub directory

h_sources = GVAudio.h
cpp_sources = GVAudio.cpp

#Install the headers in a versioned directory - gvideo-x.x/libgvaudio:
library_includedir=$(includedir)/$(PACKAGE)-$(PACKAGE_RELEASE)/$(GVAUDIO_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common

lib_LTLIBRARIES= libgvaudio.la
libgvaudio_la_SOURCES= $(h_sources) $(cpp_sources)
libgvaudio_la_CFLAGS = $(GVAUDIOLIB_CFLAGS)
libgvaudio_la_LIBADD= $(GVAUDIOLIB_LIBS) -lpthread
libgvaudio_la_LDFLAGS= -version-info $(GVAUDIO_LIBRARY_VERSION) -release $(GVAUDIO_API_VERSION)

