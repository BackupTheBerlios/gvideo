## sub directory

h_sources = GVThreads.h
cpp_sources = GVThreads.cpp

#Install the headers in a versioned directory - gvideo-x.x/libgvaudio:
library_includedir=$(includedir)/$(PACKAGE)-$(PACKAGE_RELEASE)/$(GVTHREADS_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/common

lib_LTLIBRARIES= libgvthreads.la
libgvthreads_la_SOURCES= $(h_sources) $(cpp_sources)
libgvthreads_la_CFLAGS = $(GVTHREADSLIB_CFLAGS)
libgvthreads_la_LIBADD= $(GVTHREADSLIB_LIBS)
libgvthreads_la_LDFLAGS= -version-info $(GVTHREADS_LIBRARY_VERSION) -release $(GVTHREADS_API_VERSION) -lpthread

