## Process this file with automake to produce Makefile.in

INCLUDES = \
           -DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
           -DPACKAGE_SRC_DIR=\""$(srcdir)"\" \
           -DPACKAGE_DATA_DIR=\""$(datadir)"\" \
           $(EXTRA_CFLAGS) -I$(top_srcdir) -I$(top_srcdir)/common \
           $(GVIDEOQT_CFLAGS)

AM_CFLAGS = \
            -D_REENTRANT\
            -Wall
            
            
#moc_gv_qt_window.cpp: gv_qt_window.h
#	moc $(DEFINES) $(INCPATH) $< -o $@
        
bin_PROGRAMS = gvideo-qt

h_sources = gv_video_threads.h

cpp_sources = GVideo.cpp gv_qt_window.cpp gv_video_threads.cpp gv_qt_pan-tilt_widget.cpp

# We have one .hpp file, it's called moc_gv_qt_window.hpp. Therefore, here we list
# its mocced name, moc_gv_qt_window.cpp.
nodist_gvideo_qt_SOURCES = \
    moc_gv_qt_window.cpp moc_gv_qt_pan-tilt_widget.cpp

# This is to ensure the *.hpp file is distributed in the source tarball
EXTRA_DIST = \
    $(nodist_gvideo_qt_SOURCES:moc_%.cpp=%.hpp)

gvideo_qt_SOURCES = $(h_sources) $(cpp_sources)

gvideo_qt_CFLAGS = $(GVIDEOQT_CFLAGS)

gvideo_qt_LDFLAGS = $(LIBINTL)

gvideo_qt_LDADD = $(GVIDEOQT_LIBS) \
                    $(QT_GUILINK) $(QT_LDADD) \
                   ../$(GVIDEO_LIBRARY_NAME)/$(GVIDEO_LIBRARY_NAME).la \
                   ../$(GVRENDER_LIBRARY_NAME)/$(GVRENDER_LIBRARY_NAME).la \
                   ../$(GVAUDIO_LIBRARY_NAME)/$(GVAUDIO_LIBRARY_NAME).la \
                   ../$(GVENCODER_LIBRARY_NAME)/$(GVENCODER_LIBRARY_NAME).la \
                   ../$(GVTHREADS_LIBRARY_NAME)/$(GVTHREADS_LIBRARY_NAME).la

localedir = $(datadir)/locale
DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@

# This rule lets GNU make create any moc_*.cpp from the equivalent *.hpp
moc_%.cpp: %.hpp
	moc $< -o $@

# Adjust this line according to the resources.qrc
#resources.cpp : resources.qrc octave-gui.png
#   rcc resources.qrc -o resources.cpp
CLEANFILES = moc_*.cpp
