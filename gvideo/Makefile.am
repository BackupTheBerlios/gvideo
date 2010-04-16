## Process this file with automake to produce Makefile.in

INCLUDES = \
           -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
           -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
           -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
           $(EXTRA_CFLAGS) -I$(top_srcdir) -I$(top_srcdir)/common

AM_CFLAGS = \
            -D_REENTRANT\
            -Wall

bin_PROGRAMS = gvideo

h_sources =

cpp_sources = GVideo.cpp

gvideo_SOURCES = $(h_sources) $(cpp_sources)

gvideo_CFLAGS = $(GVIDEOCMD_CFLAGS)

gvideo_LDFLAGS = $(LIBINTL)

gvideo_LDADD = $(GVIDEOCMD_LIBS) \
                   ../$(GVIDEO_LIBRARY_NAME)/$(GVIDEO_LIBRARY_NAME).la \
                   ../$(GVRENDER_LIBRARY_NAME)/$(GVRENDER_LIBRARY_NAME).la \
                   ../$(GVAUDIO_LIBRARY_NAME)/$(GVAUDIO_LIBRARY_NAME).la \
                   ../$(GVENCODER_LIBRARY_NAME)/$(GVENCODER_LIBRARY_NAME).la

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@



